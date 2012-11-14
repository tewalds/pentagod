
#include "agentmcts.h"
#include "board.h"
#include <cmath>
#include <string>
#include "string.h"
#include "alarm.h"
#include "time.h"

const float AgentMCTS::min_rave = 0.1;

void AgentMCTS::MCTSThread::run(){
	while(true){
		switch(player->threadstate){
		case Thread_Cancelled:  //threads should exit
			return;

		case Thread_Wait_Start: //threads are waiting to start
		case Thread_Wait_Start_Cancelled:
			player->runbarrier.wait();
			CAS(player->threadstate, Thread_Wait_Start, Thread_Running);
			CAS(player->threadstate, Thread_Wait_Start_Cancelled, Thread_Cancelled);
			break;

		case Thread_Wait_End:   //threads are waiting to end
			player->runbarrier.wait();
			CAS(player->threadstate, Thread_Wait_End, Thread_Wait_Start);
			break;

		case Thread_Running:    //threads are running
			if(player->rootboard.won() >= 0 || player->root.outcome >= 0 || (player->maxruns > 0 && player->runs >= player->maxruns)){ //solved or finished runs
				if(CAS(player->threadstate, Thread_Running, Thread_Wait_End) && player->root.outcome >= 0)
					logerr("Solved as " + to_str(player->root.outcome) + "\n");
				break;
			}
			if(player->ctmem.memalloced() >= player->maxmem){ //out of memory, start garbage collection
				CAS(player->threadstate, Thread_Running, Thread_GC);
				break;
			}

			INCR(player->runs);
			iterate();
			break;

		case Thread_GC:         //one thread is running garbage collection, the rest are waiting
		case Thread_GC_End:     //once done garbage collecting, go to wait_end instead of back to running
			if(player->gcbarrier.wait()){
				Time starttime;
				logerr("Starting player GC with limit " + to_str(player->gclimit) + " ... ");
				uint64_t nodesbefore = player->nodes;
				Board copy = player->rootboard;
				player->garbage_collect(copy, & player->root);
				Time gctime;
				player->ctmem.compact(1.0, 0.75);
				Time compacttime;
				logerr(to_str(100.0*player->nodes/nodesbefore, 1) + " % of tree remains - " +
					to_str((gctime - starttime)*1000, 0)  + " msec gc, " + to_str((compacttime - gctime)*1000, 0) + " msec compact\n");

				if(player->ctmem.meminuse() >= player->maxmem/2)
					player->gclimit = (int)(player->gclimit*1.3);
				else if(player->gclimit > 5)
					player->gclimit = (int)(player->gclimit*0.9); //slowly decay to a minimum of 5

				CAS(player->threadstate, Thread_GC,     Thread_Running);
				CAS(player->threadstate, Thread_GC_End, Thread_Wait_End);
			}
			player->gcbarrier.wait();
			break;
		}
	}
}

void AgentMCTS::genmove(double time, uint64_t max_runs, int verbose){
	time_used = 0;
	int toplay = rootboard.toplay();

	if(rootboard.won() >= 0 || (time <= 0 && max_runs == 0))
		return;

	Time starttime;

	stop_threads();

	if(runs)
		logerr("Pondered " + to_str(runs) + " runs\n");

	runs = 0;
	maxruns = max_runs;
	for(unsigned int i = 0; i < threads.size(); i++)
		threads[i]->reset();


	//let them run!
	start_threads();

	Alarm timer;
	if(time > 0)
		timer(time - (Time() - starttime), std::tr1::bind(&AgentMCTS::timedout, this));

	//wait for the timer to stop them
	runbarrier.wait();
	CAS(threadstate, Thread_Wait_End, Thread_Wait_Start);
	assert(threadstate == Thread_Wait_Start);

	time_used = Time() - starttime;


	if(verbose){
		DepthStats gamelen, treelen;
		double times[4] = {0,0,0,0};
		for(unsigned int i = 0; i < threads.size(); i++){
			gamelen += threads[i]->gamelen;
			treelen += threads[i]->treelen;
			for(int a = 0; a < 4; a++)
				times[a] += threads[i]->times[a];
		}

		logerr("Finished:    " + to_str(runs) + " runs in " + to_str(time_used*1000, 0) + " msec: " + to_str(runs/time_used, 0) + " Games/s\n");
		if(runs > 0){
			logerr("Game length: " + gamelen.to_s() + "\n");
			logerr("Tree depth:  " + treelen.to_s() + "\n");
			if(profile)
				logerr("Times:       " + to_str(times[0], 3) + ", " + to_str(times[1], 3) + ", " + to_str(times[2], 3) + ", " + to_str(times[3], 3) + "\n");
		}

		if(root.outcome != -3){
			logerr("Solved as a ");
			if(     root.outcome == 0)        logerr("draw\n");
			else if(root.outcome == 3)        logerr("draw by simultaneous win\n");
			else if(root.outcome == toplay)   logerr("win\n");
			else if(root.outcome == 3-toplay) logerr("loss\n");
			else if(root.outcome == -toplay)  logerr("win or draw\n");
			else if(root.outcome == toplay-3) logerr("loss or draw\n");
		}

		vector<Move> pv = get_pv();
		string pvstr;
		for(vector<Move>::iterator m = pv.begin(); m != pv.end(); ++m)
			pvstr += " " + m->to_s();
		logerr("PV:         " + pvstr + "\n");

		if(verbose >= 3 && !root.children.empty())
			logerr("Move stats:\n" + move_stats(vector<Move>()));
	}

	for(unsigned int i = 0; i < threads.size(); i++)
		threads[i]->reset();
	runs = 0;


	if(ponder && root.outcome < 0)
		start_threads();
}

AgentMCTS::AgentMCTS() {
	nodes = 0;
	runs = 0;
	gclimit = 5;
	time_used = 0;

	profile     = false;
	ponder      = false;
//#ifdef SINGLE_THREAD ... make sure only 1 thread
	numthreads  = 1;
	maxmem      = 1000*1024*1024;

	explore     = 1;
	parentexplore = true;
	knowledge   = true;
	useexplore  = 1;
	fpurgency   = 1;
	rollouts    = 10;

	keeptree    = true;
	minimax     = 1;
	visitexpand = 1;
	prunesymmetry = true;
	gcsolved    = 100000;

	instantwin  = 0;

	//no threads started until a board is set
	threadstate = Thread_Wait_Start;
}
AgentMCTS::~AgentMCTS(){
	stop_threads();

	numthreads = 0;
	reset_threads(); //shut down the theads properly

	root.dealloc(ctmem);
	ctmem.compact();
}
void AgentMCTS::timedout() {
	CAS(threadstate, Thread_Running, Thread_Wait_End);
	CAS(threadstate, Thread_GC, Thread_GC_End);
}

string AgentMCTS::statestring(){
	switch(threadstate){
	case Thread_Cancelled:  return "Thread_Wait_Cancelled";
	case Thread_Wait_Start: return "Thread_Wait_Start";
	case Thread_Wait_Start_Cancelled: return "Thread_Wait_Start_Cancelled";
	case Thread_Running:    return "Thread_Running";
	case Thread_GC:         return "Thread_GC";
	case Thread_GC_End:     return "Thread_GC_End";
	case Thread_Wait_End:   return "Thread_Wait_End";
	}
	return "Thread_State_Unknown!!!";
}

void AgentMCTS::stop_threads(){
	if(threadstate != Thread_Wait_Start){
		timedout();
		runbarrier.wait();
		CAS(threadstate, Thread_Wait_End, Thread_Wait_Start);
		assert(threadstate == Thread_Wait_Start);
	}
}

void AgentMCTS::start_threads(){
	assert(threadstate == Thread_Wait_Start);
	runbarrier.wait();
	CAS(threadstate, Thread_Wait_Start, Thread_Running);
}

void AgentMCTS::reset_threads(){ //start and end with threadstate = Thread_Wait_Start
	assert(threadstate == Thread_Wait_Start);

//wait for them to all get to the barrier
	assert(CAS(threadstate, Thread_Wait_Start, Thread_Wait_Start_Cancelled));
	runbarrier.wait();

//make sure they exited cleanly
	for(unsigned int i = 0; i < threads.size(); i++)
		threads[i]->join();

	threads.clear();

	threadstate = Thread_Wait_Start;

	runbarrier.reset(numthreads + 1);
	gcbarrier.reset(numthreads);

//start new threads
	for(int i = 0; i < numthreads; i++)
		threads.push_back(new MCTSThread(this));
}

void AgentMCTS::set_ponder(bool p){
	if(ponder != p){
		ponder = p;
		stop_threads();

		if(ponder)
			start_threads();
	}
}

void AgentMCTS::set_board(const Board & board, bool clear){
	stop_threads();

	nodes -= root.dealloc(ctmem);
	root = Node();
	root.exp.addwins(visitexpand+1);

	rootboard = board;

	reset_threads(); //needed since the threads aren't started before a board it set

	if(ponder)
		start_threads();
}
void AgentMCTS::move(const Move & m){
	stop_threads();

	uword nodesbefore = nodes;

	if(keeptree && root.children.num() > 0){
		Node child;

		for(Node * i = root.children.begin(); i != root.children.end(); i++){
			if(i->move == m){
				child = *i;          //copy the child experience to temp
				child.swap_tree(*i); //move the child tree to temp
				break;
			}
		}

		nodes -= root.dealloc(ctmem);
		root = child;
		root.swap_tree(child);

		if(nodesbefore > 0)
			logerr("Nodes:       before: " + to_str(nodesbefore) + ", after: " + to_str(nodes) + ", saved " +  to_str(100.0*nodes/nodesbefore, 1) + "% of the tree\n");
	}else{
		nodes -= root.dealloc(ctmem);
		root = Node();
		root.move = m;
	}
	assert(nodes == root.size());

	rootboard.move(m);

	root.exp.addwins(visitexpand+1); //+1 to compensate for the virtual loss
	if(rootboard.won() < 0)
		root.outcome = -3;

	if(ponder)
		start_threads();
}

double AgentMCTS::gamelen() const {
	DepthStats len;
	for(unsigned int i = 0; i < threads.size(); i++)
		len += threads[i]->gamelen;
	return len.avg();
}

vector<Move> AgentMCTS::get_pv() const {
	vector<Move> pv;

	const Node * n = & root;
	char turn = rootboard.toplay();
	while(n && !n->children.empty()){
		Move m = return_move(n, turn);
		pv.push_back(m);
		n = find_child(n, m);
		turn = 3 - turn;
	}

	if(pv.size() == 0)
		pv.push_back(Move(M_RESIGN));

	return pv;
}

string AgentMCTS::move_stats(vector<Move> moves) const {
	string s = "";
	const Node * node = & root;

	for(vector<Move>::iterator m = moves.begin(); node && m != moves.end(); ++m)
		node = find_child(node, *m);

	if(node){
		Node * child = node->children.begin(),
		     * childend = node->children.end();
		for( ; child != childend; child++)
			if(child->move != M_NONE)
				s += child->to_s() + "\n";
	}
	return s;
}

Move AgentMCTS::return_move(const Node * node, int toplay, int verbose) const {
	double val, maxval = -1000000000000.0; //1 trillion

	Node * ret = NULL,
		 * child = node->children.begin(),
		 * end = node->children.end();

	for( ; child != end; child++){
		if(child->outcome >= 0){
			if(child->outcome == toplay) val =  800000000000.0 - child->exp.num(); //shortest win
			else if(child->outcome == 0) val = -400000000000.0 + child->exp.num(); //longest tie
			else                         val = -800000000000.0 + child->exp.num(); //longest loss
		}else{ //not proven
//			val = child->exp.num(); //num simulations
			val = child->exp.sum(); //num wins
		}

		if(maxval < val){
			maxval = val;
			ret = child;
		}
	}

	assert(ret);

	if(verbose)
		logerr("Score:       " + to_str(ret->exp.avg()*100., 2) + "% / " + to_str(ret->exp.num()) + "\n");

	return ret->move;
}

void AgentMCTS::garbage_collect(Board & board, Node * node){
	Node * child = node->children.begin(),
		 * end = node->children.end();

	int toplay = board.toplay();
	for( ; child != end; child++){
		if(child->children.num() == 0)
			continue;

		if(	(node->outcome >= 0 && child->exp.num() > gcsolved && (node->outcome != toplay || child->outcome == toplay || child->outcome == 0)) || //parent is solved, only keep the proof tree, plus heavy draws
			(node->outcome <  0 && child->exp.num() > (child->outcome >= 0 ? gcsolved : gclimit)) ){ // only keep heavy nodes, with different cutoffs for solved and unsolved
			board.move(child->move);
			garbage_collect(board, child);
			board.undo(child->move);
		}else{
			nodes -= child->dealloc(ctmem);
		}
	}
}

AgentMCTS::Node * AgentMCTS::find_child(const Node * node, const Move & move) const {
	for(Node * i = node->children.begin(); i != node->children.end(); i++)
		if(i->move == move)
			return i;

	return NULL;
}


