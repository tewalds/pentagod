
#include <cstdio>
#include <cstdlib>
#include <stdint.h>

#include <string.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "board.cpp"
#include "scoresimple.h"
#include "scoresecond.h"

#include "player.h"
#include "playernegamax1.cpp"
#include "playernegamax2.cpp"
#include "playernegamax3.cpp"
#include "playernegamax4.cpp"
#include "playernegascout1.cpp"
#include "playermontecarlo.cpp"
#include "playeruct.cpp"

#include "tqueue.h"

#include <sys/queue.h>
#include <event.h>
#include <evhttp.h>

#ifndef MAX_THREADS
#define MAX_THREADS 100
#endif


struct server_stats {
	unsigned int starttime;
	unsigned int games;
	unsigned int requests;
	unsigned int moves;
	unsigned int cputime;
};

struct request_t {
	struct evhttp_request *req;
	Board boardstart;
	Board boardend;
	Player *player;
};

struct global_data {
	int pushfd;
	int popfd;
	server_stats stats;
	char * static_source;
	tqueue<request_t> * request;
	tqueue<request_t> * response;
};

struct thread_data_t {
	unsigned int threadid;
	char state;
	global_data * global;
};


unsigned int get_now(void){
	struct timeval now_tv;
	gettimeofday(&now_tv, NULL);
	return now_tv.tv_sec;
}

//to tell the main thread that there is new stuff in the queue
void signalfd(int fd){
	const char signalbuf[1] = { 0 };
	if(fd)
		if(write(fd, signalbuf, 1)); //ignore the return value, but suppress the warning
}

void * requestRunner(thread_data_t * threaddata){
	request_t * req;
	global_data * global = threaddata->global;

	while(threaddata->state){
		req = global->request->pop();

		if(req){
			req->boardend = req->player->move(req->boardstart);

			global->response->push(req);
			signalfd(global->pushfd);
		}
	}

	return NULL;
}

void build_move_str(request_t * req, char * buf){
	uint64_t hash = req->boardend.simplehash();

	Board start = req->boardstart;
	Board test;
	
	char spin_map[5] = "1324";

	buf[0] = buf[1] = buf[2] = buf[3] = buf[4] = '\0';

	for(int pos = 0; pos < 36; pos++){
		if(!start.squares[pos]){ //is a valid place to put a piece
			for(int spin = 0; spin < 8; spin++){
				test = start; //copy the board

				test.move(pos, spin);

			//if the move is found, return how to get to it, according to the api defined by pentagoo
				if(test.simplehash() == hash){
					buf[0] = (pos % 6) + '0';
					buf[1] = (pos / 6) + '0';
					buf[2] = spin_map[spin/2];
					buf[3] = (spin % 2 ? 'l' : 'r');
					buf[4] = '\0';

					return;
				}
			}
		}
	}
}

void handle_queue_response(int fd, short event, void *arg){
	global_data * global = (global_data *) arg;
	request_t * req;

	struct evbuffer *evb;

	char buf[64];

//read from the response queue, but throw it all away, since it's just put there for notification purposes
	if(read(fd, buf, sizeof(buf))); //ignore the return value, but suppress the warning

	while((req = global->response->pop(0))){ //pop a response in non-block mode
		global->stats.moves += req->player->totalmoves;
		global->stats.cputime += req->player->totaltime;

		req->boardend.print();

		evb = evbuffer_new();

		build_move_str(req, buf);

		evbuffer_add_printf(evb, "%s", buf);

		evhttp_send_reply(req->req, HTTP_OK, "OK", evb);

		evbuffer_free(evb);

		delete req->player;
		delete req;
	}
}

//handles /ai?...
void handle_request_ai(struct evhttp_request *req, void *arg){
	global_data * global = (global_data *) arg;

	global->stats.requests++;

	request_t * aireq = new request_t();
	const char * ptr;
	struct evkeyvalq options;

	evhttp_parse_query(req->uri, &options);

	int depth = 2;
	Board board = Board(true); //start with an empty board, replacing it below if a one is supplied

	if((ptr = evhttp_find_header(&options, "l")))
		depth = atoi(ptr);
	
	if(depth < 0)	depth = 0;
	if(depth > 4)	depth = 4;

	if((ptr = evhttp_find_header(&options, "m")) && strlen(ptr) == 36)
		board = Board(ptr);

	aireq->req = req;
	aireq->boardstart = board;
	aireq->player = new PlayerNegamax3(depth);

	global->request->push(aireq);
	evhttp_clear_headers(&options);
}


void handle_request_stats(struct evhttp_request *req, void *arg){
	global_data * global = (global_data *) arg;

	struct evbuffer *evb;
	evb = evbuffer_new();

	evbuffer_add_printf(evb, "Uptime:   %u<br />\n", (get_now() - global->stats.starttime));
	evbuffer_add_printf(evb, "Requests: %u<br />\n", global->stats.requests);
//	evbuffer_add_printf(evb, "Games:    %u<br />\n", global->stats.games);
	evbuffer_add_printf(evb, "Moves:    %u<br />\n", global->stats.moves);
	evbuffer_add_printf(evb, "Cpu Time: %u s<br />\n", global->stats.cputime/1000);

	evhttp_send_reply(req, HTTP_OK, "OK", evb);

	evbuffer_free(evb);
}

void handle_http_static(struct evhttp_request *req, void *arg){
	global_data * global = (global_data *) arg;

	struct evkeyvalq options;

	evhttp_parse_query(req->uri, &options);

	char filename[1000];
	filename[0] = '\0';

	strcat(filename, global->static_source);

//make sure it ends in / at this point
	if(filename[strlen(filename)-1] != '/')
		strcat(filename, "/");

	char * start = req->uri;
	if(*start == '/')
		++start;
	char * ptr = start;

	//look for the end of the string, end of the uri, or ..
	while(*ptr != '\0' && *ptr != '?' && !(*ptr == '.' && *(ptr+1) == '.')) 
		++ptr;

	if(*ptr == '\0'){
		strncat(filename, start, 1000 - (ptr - start) - strlen(filename));
	}else if(*ptr == '?'){
		strncat(filename, start, ((unsigned int)(ptr - start) < (unsigned int)(1000 - strlen(filename)) ? ptr - start : 1000 - strlen(filename)) );
	}
	//if it included .. , just ignore the url altogether

//add index.html if it ends in / at this point
	if(filename[strlen(filename)-1] == '/')
		strcat(filename, "index.html");

	FILE *fd;
	char buf[4096];

	if((fd = fopen(filename, "r")) == NULL){
		struct evbuffer *evb;
		evb = evbuffer_new();
		evbuffer_add_printf(evb, "File Not found:  %s\n", req->uri);
		
		evhttp_send_reply(req, HTTP_NOTFOUND, "Not Found", evb);
		evbuffer_free(evb);
	}else{
		evhttp_send_reply_start(req, HTTP_OK, "OK");

		int len;
		struct evbuffer *evb;
		while((len = fread(buf, sizeof(char), 4096, fd))){
			evb = evbuffer_new();
			evbuffer_add(evb, buf, len);
		
			evhttp_send_reply_chunk(req, evb);
			evbuffer_free(evb);
		}

		evhttp_send_reply_end(req);
		
		fclose(fd);
	}
	evhttp_clear_headers(&options);
}

int main(int argc, char **argv){
	unsigned int i;
	char * ptr;

	unsigned int numthreads;

	global_data * global = new global_data;

	global->stats.starttime = get_now();

//2 queues
	global->request  = new tqueue<request_t>();
	global->response = new tqueue<request_t>();

//thread stuff
	pthread_t thread[MAX_THREADS];
	thread_data_t threaddata[MAX_THREADS];

//http server stuff
	struct evhttp *http;
	struct event updateEvent;


//notification fds
	int fds[2];

	srand(time(NULL));

	//defaults
	char port_def[] = "80";
	char hostname_def[] = "0.0.0.0";
	char threads_def[] = "3";
	char static_source_def[] = "pentagoo/";

	//Argument Pointers
	char *port_arg = port_def;
	char *hostname_arg = hostname_def;
	char *threads_arg = threads_def;
	char *static_source = static_source_def;


//Parse command line options
	for (i = 1; i < (unsigned int)argc; i++) {
		ptr = argv[i];
		if(strcmp(ptr, "--help") == 0){
			printf("Usage:\n"
				"\t--help        Show this help\n"
				"\t-l            Location for the web frontend [%s]\n"
				"\t-p            Port Number [%s]\n"
				"\t-h            Hostname [%s]\n"
				"\t-t            Number of threads [%s]\n\n",
				static_source_def, port_def, hostname_def, threads_def);
			exit(255);
		}else if (strcmp(ptr, "-p") == 0)
			port_arg = ptr = argv[++i];
		else if (strcmp(ptr, "-h") == 0)
			hostname_arg = ptr = argv[++i];
		else if (strcmp(ptr, "-t") == 0)
			threads_arg = ptr = argv[++i];
		else if (strcmp(ptr, "-l") == 0)
			static_source = ptr = argv[++i];
	}


	global->static_source = static_source;

	numthreads = atoi(threads_arg);
	if(numthreads < 1)
		numthreads = 1;

	if(numthreads > MAX_THREADS){
		printf("Invalid number of threads '%s', setting to max threads %i\n", threads_arg, MAX_THREADS);
		numthreads = MAX_THREADS;
	}

//initialize update notification pipe
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

	global->pushfd = fds[0];
	global->popfd = fds[1];


	printf("Starting %u computation threads\n", numthreads);

	for(i = 0; i < numthreads; i++){
		threaddata[i].threadid = i;
		threaddata[i].state = 1;
		threaddata[i].global = global;
		
		pthread_create(&thread[i], NULL, (void* (*)(void*)) requestRunner, (void*) &threaddata[i]);
	}


//init the event lib
	event_init();


	printf("Listening on %s:%s\n", hostname_arg, port_arg);

//start the http server
	http = evhttp_start(hostname_arg, atoi(port_arg));
	if(http == NULL) {
		printf("Couldn't start server on %s:%s\n", hostname_arg, port_arg);
		return 1;
	}


//Register a callback for requests
	evhttp_set_gencb(http, handle_http_static, global); //the generic catch all callback, used to serve static files
	evhttp_set_cb(http, "/ai",              handle_request_ai,       global); //my nice url
	evhttp_set_cb(http, "/pentagoo_ai.php", handle_request_ai,       global); //hijack the pentagoo url

	evhttp_set_cb(http, "/stats",    handle_request_stats,    global);

	event_set(& updateEvent, global->popfd, EV_READ|EV_PERSIST, handle_queue_response, global);
	event_add(& updateEvent, 0);


	printf("Starting event loop\n");

	event_dispatch();


	printf("Exiting\n");

	global->request->nonblock();
	for(i = 0; i < numthreads; i++)
		pthread_join(thread[i], NULL);

	return 0;
}


