
#pragma once

//Interface for the various solvers

#include "types.h"
#include "board.h"
#include "moveiterator.h"

class Agent {
public:
	Agent() { }
	virtual ~Agent() { }

	virtual void search(double time, uint64_t maxruns, int verbose) = 0;
	virtual Move return_move(int verbose) const = 0;
	virtual void set_board(const Board & board, bool clear = true) = 0;
	virtual void move(const Move & m) = 0;
	virtual void set_memlimit(uint64_t lim) = 0; // in bytes
	virtual void clear_mem() = 0;

	virtual vector<Move> get_pv() const = 0;
	string move_stats() const { return move_stats(vector<Move>()); }
	virtual string move_stats(const vector<Move> moves) const = 0;
	virtual double gamelen() const = 0;

	virtual void timedout(){ timeout = true; }

protected:
	volatile bool timeout;
	Board rootboard;

	static int solve1ply(const Board & board, unsigned int & nodes) {
		int outcome = -4;
		int turn = board.toplay();
		for(MoveIterator move(board); !move.done(); ++move){
			++nodes;
			int won = move.board().won();

			if(won == turn)
				return won;
			else if(won == 0)
				outcome = 0;
			else if(outcome == 3 - turn || outcome == -4)
				outcome = won;
		}
		return outcome;
	}
};

