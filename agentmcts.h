
#pragma once

//A Monte-Carlo Tree Search based player

#include <cmath>
#include <cassert>

#include "time.h"
#include "types.h"
#include "move.h"
#include "board.h"
#include "depthstats.h"
#include "thread.h"
#include "xorshift.h"
#include "compacttree.h"
#include "log.h"
#include "agent.h"

class AgentMCTS : public Agent{
public:
	class ExpPair {
		uword s, n;
		ExpPair(uword S, uword N) : s(S), n(N) { }
	public:
		ExpPair() : s(0), n(0) { }
		float avg() const { return 0.5f*s/n; }
		uword num() const { return n; }
		uword sum() const { return s/2; }

		void clear() { s = 0; n = 0; }

		void addvloss(){ INCR(n); }
		void addvtie() { INCR(s); }
		void addvwin() { PLUS(s, 2); }
		void addv(const ExpPair & a){
			if(a.s) PLUS(s, a.s);
			if(a.n) PLUS(n, a.n);
		}

		void addloss(){ n++; }
		void addtie() { s++; }
		void addwin() { s += 2; }
		void add(const ExpPair & a){
			s += a.s;
			n += a.n;
		}

		void addwins(uword num)  { n += num; s += 2*num; }
		void addlosses(uword num){ n += num; }
		ExpPair & operator+=(const ExpPair & a){
			s += a.s;
			n += a.n;
			return *this;
		}
		ExpPair operator + (const ExpPair & a){
			return ExpPair(s + a.s, n + a.n);
		}
		ExpPair & operator*=(uword m){
			s *= m;
			n *= m;
			return *this;
		}
		ExpPair invert(){ //return it from the other player's perspective
			return ExpPair(n*2 - s, n);
		}
	};

	struct Node {
	public:
		ExpPair exp;
		int16_t know;
		int8_t  outcome;
		uint8_t proofdepth;
		Move    move;
		Move    bestmove; //if outcome is set, then bestmove is the way to get there
		CompactTree<Node>::Children children;
//		int padding;
		//seems to need padding to multiples of 8 bytes or it segfaults?
		//don't forget to update the copy constructor/operator

		Node()                            : know(0), outcome(-3), proofdepth(0)          { }
		Node(const Move & m, char o = -3) : know(0), outcome( o), proofdepth(0), move(m) { }
		Node(const Node & n) { *this = n; }
		Node & operator = (const Node & n){
			if(this != & n){ //don't copy to self
				//don't copy to a node that already has children
				assert(children.empty());

				exp  = n.exp;
				know = n.know;
				move = n.move;
				bestmove = n.bestmove;
				outcome = n.outcome;
				proofdepth = n.proofdepth;
				//children = n.children; ignore the children, they need to be swap_tree'd in
			}
			return *this;
		}

		void swap_tree(Node & n){
			children.swap(n.children);
		}

		void print() const {
			printf("%s\n", to_s().c_str());
		}
		string to_s() const {
			return "Node: move " + move.to_s() +
					", exp " + to_str(exp.avg(), 2) + "/" + to_str(exp.num()) +
					", know " + to_str(know) +
					", outcome " + to_str((int)outcome) + "/" + to_str((int)proofdepth) +
					", best " + bestmove.to_s() +
					", children " + to_str(children.num());
		}

		unsigned int size() const {
			unsigned int num = children.num();

			if(children.num())
				for(Node * i = children.begin(); i != children.end(); i++)
					num += i->size();

			return num;
		}

		~Node(){
			assert(children.empty());
		}

		unsigned int alloc(unsigned int num, CompactTree<Node> & ct){
			return children.alloc(num, ct);
		}
		unsigned int dealloc(CompactTree<Node> & ct){
			unsigned int num = 0;

			if(children.num())
				for(Node * i = children.begin(); i != children.end(); i++)
					num += i->dealloc(ct);
			num += children.dealloc(ct);

			return num;
		}

		float value(bool knowledge, float fpurgency){
			float val = fpurgency;
			float expnum = exp.num();

			if(expnum > 0)
				val = exp.avg();

			if(knowledge && know > 0){
				if(expnum <= 1)
					val += 0.01f * know;
				else if(expnum < 1000) //knowledge is only useful with little experience
					val += 0.01f * know / sqrt(expnum);
			}

			return val;
		}
	};

	struct MoveList { //intended to be used to track moves for use in rave or similar

		ExpPair  exp[2];       //aggregated outcomes overall

		MoveList() { }

		void addtree(const Move & move, char turn){
		}
		void addrollout(const Move & move, char turn){
		}
		void reset(Board * b){
			exp[0].clear();
			exp[1].clear();
		}
		void finishrollout(int won){
			exp[0].addloss();
			exp[1].addloss();
			if(won == 0){
				exp[0].addtie();
				exp[1].addtie();
			}else{
				exp[won-1].addwin();
			}
		}
		void subvlosses(int n){
			exp[0].addlosses(-n);
			exp[1].addlosses(-n);
		}
		const ExpPair & getexp(int turn) const {
			return exp[turn-1];
		}
	};

	class MCTSThread {
		mutable XORShift_uint64 rand64;
		mutable XORShift_float unitrand;
		Thread thread;
		AgentMCTS * player;
		bool use_explore; //whether to use exploration for this simulation
		MoveList movelist;
		int stage; //which of the four MCTS stages is it on

	public:
		DepthStats treelen, gamelen;
		double times[4]; //time spent in each of the stages
		Time timestamps[4]; //timestamps for the beginning, before child creation, before rollout, after rollout

		MCTSThread(AgentMCTS * p) : rand64(std::rand()), unitrand(std::rand()), player(p) {
			reset();
			thread(bind(&MCTSThread::run, this));
		}
		~MCTSThread() { }

		void reset(){
			treelen.reset();
			gamelen.reset();
			use_explore = false;
			for(int a = 0; a < 4; a++)
				times[a] = 0;
		}

		int join(){ return thread.join(); }

	private:
		void run(); //thread runner, calls iterate on each iteration
		void iterate(); //handles each iteration
		void walk_tree(Board & board, Node * node, int depth);
		bool create_children(const Board & board, Node * node, int toplay);
		void add_knowledge(const Board & board, Node * node, Node * child);
		Node * choose_move(const Node * node, int toplay) const;

		int rollout(Board & board, Move move, int depth);
//		PairMove rollout_choose_move(Board & board, const Move & prev, int & doinstwin, bool checkrings);
	};


public:

	static const float min_rave;

	bool  ponder;     //think during opponents time?
	int   numthreads; //number of player threads to run
	u64   maxmem;     //maximum memory for the tree in bytes
	bool  profile;    //count how long is spent in each stage of MCTS

//tree traversal
	bool  parentexplore; // whether to multiple exploration by the parents winrate
	float explore;    //greater than one favours exploration, smaller than one favours exploitation
	bool  knowledge;  //whether to include knowledge
	float useexplore; //what probability to use UCT exploration
	float fpurgency;  //what value to return for a move that hasn't been played yet
	int   rollouts;   //number of rollouts to run after the tree traversal

//tree building
	bool  keeptree;   //reuse the tree from the previous move
	int   minimax;    //solve the minimax tree within the uct tree
	uint  visitexpand;//number of visits before expanding a node
	bool  prunesymmetry; //prune symmetric children from the move list, useful for proving but likely not for playing
	uint  gcsolved;   //garbage collect solved nodes or keep them in the tree, assuming they meet the required amount of work

//rollout
	int   instantwin;     //look for instant wins in rollouts

	Board rootboard;
	Node  root;
	uword nodes;
	int   gclimit; //the minimum experience needed to not be garbage collected

	uint64_t runs, maxruns;

	CompactTree<Node> ctmem;

	enum ThreadState {
		Thread_Cancelled,  //threads should exit
		Thread_Wait_Start, //threads are waiting to start
		Thread_Wait_Start_Cancelled, //once done waiting, go to cancelled instead of running
		Thread_Running,    //threads are running
		Thread_GC,         //one thread is running garbage collection, the rest are waiting
		Thread_GC_End,     //once done garbage collecting, go to wait_end instead of back to running
		Thread_Wait_End,   //threads are waiting to end
	};
	volatile ThreadState threadstate;
	vector<MCTSThread *> threads;
	Barrier runbarrier, gcbarrier;

	double time_used;

	AgentMCTS();
	~AgentMCTS();

	string statestring();

	void stop_threads();
	void start_threads();
	void reset_threads();

	void set_memlimit(uint64_t lim) { }; // in bytes
	void clear_mem() { };

	void set_ponder(bool p);
	void set_board(const Board & board, bool clear = true);

	void move(const Move & m);

	void search(double time, uint64_t maxruns, int verbose);
	Move return_move(int verbose) const { return return_move(& root, rootboard.toplay(), verbose); }

	double gamelen() const;
	vector<Move> get_pv() const;
	string move_stats(const vector<Move> moves) const;

	void timedout();
protected:

	void garbage_collect(Board & board, Node * node); //destroys the board, so pass in a copy
	bool do_backup(Node * node, Node * backup, int toplay);
	Move return_move(const Node * node, int toplay, int verbose = 0) const;
	Node * find_child(const Node * node, const Move & move) const ;
};

