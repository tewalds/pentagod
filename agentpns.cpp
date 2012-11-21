
#include "agentpns.h"
#include "moveiterator.h"
#include "time.h"
#include "alarm.h"
#include "log.h"

void AgentPNS::search(double time, uint64_t maxiters, int verbose){
	if(rootboard.won() >= 0){
		outcome = rootboard.won();
		return;
	}

	start_threads();

	timeout = false;
	Alarm timer(time, std::tr1::bind(&AgentPNS::timedout, this));
	Time start;

//	logerr("max memory: " + to_str(memlimit/(1024*1024)) + " Mb\n");

	//wait for the timer to stop them
	runbarrier.wait();
	CAS(threadstate, Thread_Wait_End, Thread_Wait_Start);
	assert(threadstate == Thread_Wait_Start);

	if(root.phi == 0 && root.delta == LOSS){ //look for the winning move
		for(Node * i = root.children.begin() ; i != root.children.end(); i++){
			if(i->delta == 0){
				bestmove = i->move;
				break;
			}
		}
		outcome = rootboard.toplay();
	}else if(root.phi == 0 && root.delta == DRAW){ //look for the move to tie
		for(Node * i = root.children.begin() ; i != root.children.end(); i++){
			if(i->delta == DRAW){
				bestmove = i->move;
				break;
			}
		}
		outcome = 0;
	}else if(root.delta == 0){ //loss
		bestmove = M_NONE;
		outcome = 3 - rootboard.toplay();
	}else{ //unknown
		bestmove = M_UNKNOWN;
		outcome = -3;
	}

	time_used = Time() - start;
}

void AgentPNS::PNSThread::run(){
	while(true){
		switch(agent->threadstate){
		case Thread_Cancelled:  //threads should exit
			return;

		case Thread_Wait_Start: //threads are waiting to start
		case Thread_Wait_Start_Cancelled:
			agent->runbarrier.wait();
			CAS(agent->threadstate, Thread_Wait_Start, Thread_Running);
			CAS(agent->threadstate, Thread_Wait_Start_Cancelled, Thread_Cancelled);
			break;

		case Thread_Wait_End:   //threads are waiting to end
			agent->runbarrier.wait();
			CAS(agent->threadstate, Thread_Wait_End, Thread_Wait_Start);
			break;

		case Thread_Running:    //threads are running
			if(agent->root.terminal()){ //solved
				CAS(agent->threadstate, Thread_Running, Thread_Wait_End);
				break;
			}
			if(agent->ctmem.memalloced() >= agent->memlimit){ //out of memory, start garbage collection
				CAS(agent->threadstate, Thread_Running, Thread_GC);
				break;
			}

			pns(agent->rootboard, &agent->root, 0, INF32/2, INF32/2);
			break;

		case Thread_GC:         //one thread is running garbage collection, the rest are waiting
		case Thread_GC_End:     //once done garbage collecting, go to wait_end instead of back to running
			if(agent->gcbarrier.wait()){
				logerr("Starting solver GC with limit " + to_str(agent->gclimit) + " ... ");

				Time starttime;
				agent->garbage_collect(& agent->root);

				Time gctime;
				agent->ctmem.compact(1.0, 0.75);

				Time compacttime;
				logerr(to_str(100.0*agent->ctmem.meminuse()/agent->memlimit, 1) + " % of tree remains - " +
					to_str((gctime - starttime)*1000, 0)  + " msec gc, " + to_str((compacttime - gctime)*1000, 0) + " msec compact\n");

				if(agent->ctmem.meminuse() >= agent->memlimit/2)
					agent->gclimit = (unsigned int)(agent->gclimit*1.3);
				else if(agent->gclimit > 5)
					agent->gclimit = (unsigned int)(agent->gclimit*0.9); //slowly decay to a minimum of 5

				CAS(agent->threadstate, Thread_GC,     Thread_Running);
				CAS(agent->threadstate, Thread_GC_End, Thread_Wait_End);
			}
			agent->gcbarrier.wait();
			break;
		}
	}
}

void AgentPNS::timedout() {
	CAS(threadstate, Thread_Running, Thread_Wait_End);
	CAS(threadstate, Thread_GC, Thread_GC_End);
	timeout = true;
}

string AgentPNS::statestring(){
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

void AgentPNS::stop_threads(){
	if(threadstate != Thread_Wait_Start){
		timedout();
		runbarrier.wait();
		CAS(threadstate, Thread_Wait_End, Thread_Wait_Start);
		assert(threadstate == Thread_Wait_Start);
	}
}

void AgentPNS::start_threads(){
	assert(threadstate == Thread_Wait_Start);
	runbarrier.wait();
	CAS(threadstate, Thread_Wait_Start, Thread_Running);
}

void AgentPNS::reset_threads(){ //start and end with threadstate = Thread_Wait_Start
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
		threads.push_back(new PNSThread(this));
}


bool AgentPNS::PNSThread::pns(const Board & board, Node * node, int depth, uint32_t tp, uint32_t td){
	iters++;
	if(agent->maxdepth < depth)
		agent->maxdepth = depth;

	if(node->children.empty()){
		if(node->terminal())
			return true;

		if(agent->ctmem.memalloced() >= agent->memlimit)
			return false;

		if(!node->children.lock())
			return false;

		int numnodes = board.moves_avail();
		CompactTree<Node>::Children temp;
		temp.alloc(numnodes, agent->ctmem);

		unsigned int i = 0;
		for(MoveIterator move(board, true); !move.done(); ++move){
			int outcome = move.board().won();
			int pd = 1;
			temp[i] = Node(*move).outcome(outcome, board.toplay(), agent->ties, pd);
			i++;
		}
		PLUS(agent->nodes, i);
		temp.shrink(i); //if symmetry, there may be extra moves to ignore
		node->children.swap(temp);
		assert(temp.unlock());

		PLUS(agent->nodes_seen, i);

		updatePDnum(node);

		return true;
	}

	bool mem;
	do{
		Node * child = node->children.begin(),
		        * child2 = node->children.begin(),
		        * childend = node->children.end();

		uint32_t tpc, tdc;

		if(agent->df){
			for(Node * i = node->children.begin(); i != childend; i++){
				if(i->refdelta() <= child->refdelta()){
					child2 = child;
					child = i;
				}else if(i->refdelta() < child2->refdelta()){
					child2 = i;
				}
			}

			tpc = min(INF32/2, (td + child->phi - node->delta));
			tdc = min(tp, (uint32_t)(child2->delta*(1.0 + agent->epsilon) + 1));
		}else{
			tpc = tdc = 0;
			for(Node * i = node->children.begin(); i != childend; i++)
				if(child->refdelta() > i->refdelta())
					child = i;
		}

		Board next = board;
		next.move(child->move);

		child->ref();
		uint64_t itersbefore = iters;
		mem = pns(next, child, depth + 1, tpc, tdc);
		child->deref();
		PLUS(child->work, iters - itersbefore);

		if(updatePDnum(node) && !agent->df)
			break;

	}while(!agent->timeout && mem && (!agent->df || (node->phi < tp && node->delta < td)));

	return mem;
}

bool AgentPNS::PNSThread::updatePDnum(Node * node){
	Node * i = node->children.begin();
	Node * end = node->children.end();

	uint32_t min = i->delta;
	uint64_t sum = 0;

	bool win = false;
	for( ; i != end; i++){
		win |= (i->phi == LOSS);
		sum += i->phi;
		if( min > i->delta)
			min = i->delta;
	}

	if(win)
		sum = LOSS;
	else if(sum >= INF32)
		sum = INF32;

	if(min == node->phi && sum == node->delta){
		return false;
	}else{
		if(sum == 0 && min == DRAW){
			node->phi = 0;
			node->delta = DRAW;
		}else{
			node->phi = min;
			node->delta = sum;
		}
		return true;
	}
}


double AgentPNS::gamelen() const {
	//TODO: how to calculate this?
	return rootboard.moves_remain();
}

vector<Move> AgentPNS::get_pv() const {
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

string AgentPNS::move_stats(vector<Move> moves) const {
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

Move AgentPNS::return_move(const Node * node, int toplay, int verbose) const {
	double val, maxval = -1000000000000.0; //1 trillion

	Node * ret = NULL,
		 * child = node->children.begin(),
		 * end = node->children.end();

	for( ; child != end; child++){
		int outcome = child->to_outcome(toplay);
		if(outcome >= 0){
			if(outcome == toplay) val =  800000000000.0 - (double)child->work; //shortest win
			else if(outcome == 0) val = -400000000000.0 + (double)child->work; //longest tie
			else                  val = -800000000000.0 + (double)child->work; //longest loss
		}else{ //not proven
			val = child->work;
		}

		if(maxval < val){
			maxval = val;
			ret = child;
		}
	}

	assert(ret);

	if(verbose)
		logerr(ret->to_s() + "\n");

	return ret->move;
}

AgentPNS::Node * AgentPNS::find_child(const Node * node, const Move & move) const {
	for(Node * i = node->children.begin(); i != node->children.end(); i++)
		if(i->move == move)
			return i;

	return NULL;
}

//removes the children of any node with less than limit work
void AgentPNS::garbage_collect(Node * node){
	Node * child = node->children.begin();
	Node * end = node->children.end();

	for( ; child != end; child++){
		if(child->terminal() || child->work < gclimit){ //solved or low work, ignore solvedness since it's trivial to re-solve
			nodes -= child->dealloc(ctmem);
		}else if(child->children.num() > 0){
			garbage_collect(child);
		}
	}
}

