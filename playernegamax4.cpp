
#include "player.h"
#include <pthread.h>
#include "tqueue.h"

#define MAX_THREADS 64

class PlayerNegamax4 : public Player {
	int maxdepth;

	pthread_t thread[MAX_THREADS];
	int numthreads;

	int shared_alpha;
	tqueue<Board> request;
	tqueue<int> response;
	
public:

	PlayerNegamax4(int depth, int nnumthreads = MAX_THREADS, int (*nscorefunc)(const Board & board) = &ScoreSimple::getscore){
		maxdepth = depth;
		numthreads = (nnumthreads < MAX_THREADS ? nnumthreads : MAX_THREADS);
		scorefunc = nscorefunc;

		for(int i = 0; i < numthreads; i++)
			pthread_create(
				&thread[i], 
				NULL, 
				(void* (*)(void*)) &PlayerNegamax4::searchRunner, 
				this);

	}

	~PlayerNegamax4(){
		for(int i = 0; i < numthreads; i++)
			pthread_cancel(thread[i]);
	}

	void describe(){
		printf("class PlayerNegamax4 - a threaded moderately aggressive pruning negamax player searching to depth %i\n", maxdepth);
	}


	//must be static so that pthread will call it. This means it must take a pointer to this, and do all calls through it
	static void * searchRunner(void * blah){
		PlayerNegamax4 * player = (PlayerNegamax4 *) blah;
	
		Board * board;
		int ret;

		while(1){
			board = player->request.pop();

			//using alpha+1 distinguishes between a move equivalent to a previous one, and one cut off by the pruning
			//without losing much of the speed advantage of passing in an alpha value
			board->score = ret = player->negamax(*board, player->maxdepth, -2000000000, player->shared_alpha+1);

			if(player->shared_alpha > ret)
				player->shared_alpha = ret;

			player->response.push(NULL);
		}

		return NULL;
	}

	Board search_move(Board board){
		board.scorefunc = scorefunc;

		Board children[288];
		int numchildren = board.getchildren(children, true);
		depths[maxdepth] = numchildren;

		shared_alpha = 2000000000;

	//push all the children into the queue
		for(int i = 0; i < numchildren; i++)
			request.push(&children[i]);

	//wait for them to all respond
		for(int i = 0; i < numchildren; i++)
			response.pop();

	//find the best
		Board::sortchildren(children, children + numchildren); //insertion sort

		int num;
		for(num = 1; num < numchildren && children[num].score == children[0].score; num++) ;

printf("%i best move(s)\n", num);

		num = rand() % num;

		return children[num];
	}
};

