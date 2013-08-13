
#pragma once

//An Alpha-beta solver, single threaded with an optional transposition table.

#include "agent.h"
#include "xorshift.h"

class AgentAB : public Agent {
	static const int16_t SCORE_WIN  = 32767;
	static const int16_t SCORE_LOSS = -32767;
	static const int16_t SCORE_DRAW = 0;

	static const uint8_t VALID  = 1;
	static const uint8_t LBOUND = 2;
	static const uint8_t UBOUND = 3;

	struct Node {
		uint64_t hash;     // hash value of this node according to board.hash()
		int16_t  score;    // score by depth, or an upper/lower bound
		Move     bestmove; // best move relative to this node
		uint8_t  depth;    // moves searched below, or to the deepest terminal node if it's proven
		uint8_t  flag;     // whether this is an valid/upper/lower bound
		//int8_t   outcome;  // proven outcome from this node
		int16_t  padding;

//		Node() : hash(~0ull), score(0), bestmove(M_UNKNOWN), depth(0), flag(0), padding(0xDEAD) //, outcome(-3)
//			{ }
		Node(uint64_t h = ~0ull, int16_t s = 0, Move b = M_UNKNOWN, int8_t d = 0, int8_t f = 0) : //. int8_t o = -3
			hash(h), score(s), bestmove(b), depth(d), flag(f), padding(0xDEAD) { } //, outcome(o)

		string to_s(int orientation=8) const {
			return  "score " + to_str(score) +
					", depth " + to_str((int)depth) +
					", flag " + to_str((int)flag) +
					", best " + bestmove.to_s(true) +
					(orientation == 8 ? string() : "/" + bestmove.rotate(orientation).to_s(true));
		}
	};

public:

	Node * TT;
	uint64_t maxnodes, memlimit;

	int maxdepth;
	uint64_t nodes_seen;
	double time_used;
	XORShift_uint32 rand;
	int randomness;

	AgentAB() : rand(std::rand()) {
		maxdepth = 0;
		nodes_seen = 0;
		time_used = 0;
		TT = NULL;
		set_memlimit(100*1024*1024);

		randomness = 2;
	}
	~AgentAB() { }

	void set_board(const Board & board, bool clear = true){
		rootboard = board;
		reset();
	}
	void move(const Move & m){
		rootboard.move(m);
		reset();
	}
	void set_memlimit(uint64_t lim){
		memlimit = lim;
		maxnodes = memlimit/sizeof(Node);
		clear_mem();
	}

	void clear_mem(){
		reset();
		if(TT){
			delete[] TT;
			TT = NULL;
		}
	}
	void reset(){
		timeout = false;
		maxdepth = 0;
		nodes_seen = 0;
		time_used = 0;
	}

	void timedout() { timeout = true; }

	void search(double time, uint64_t maxiters, int verbose);
	Move return_move(int verbose) const { return return_move(rootboard, verbose); }
	double gamelen() const { return rootboard.moves_remain(); }
	vector<Move> get_pv() const;
	string move_stats(vector<Move> moves) const;

private:
	int16_t negamax(const Board & board, int16_t alpha, int16_t beta, int depth);
	Move return_move(const Board & board, int verbose = 0) const;

	Node * tt(uint64_t hash) const ;
	Node * tt_get(uint64_t hash) const ;
	Node * tt_get(const Board & b) const ;
	void tt_set(const Node & n) ;
};
