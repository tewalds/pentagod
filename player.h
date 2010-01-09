#ifndef __PLAYER_H_
#define __PLAYER_H_

#include "board.h"
#include "scoresimple.h"
#include "scoresecond.h"

#include <pthread.h>

/*
 * Simple player base class
 *
 */

class Player {
protected:
	uint64_t depths[36];
	int (*scorefunc)(const Board & board);

	void cleardepths(){
		for(int i = 0; i < 36; i++)
			depths[i] = 0;
	}

public:
	uint64_t totaltime;
	uint64_t totalmoves;
	uint64_t totalturns;

	pthread_mutex_t lock; // The queue lock
	bool playing;

	Player(){
		pthread_mutex_init(&lock, NULL);
		playing = false;
		
		resetstats();
	}
	virtual ~Player(){
	}

	virtual void resetstats(){
		totaltime = 0;
		totalmoves = 0;
		totalturns = 0;
		cleardepths();
	}

//move is the entry function for playing a move. It sets up the environment for search_move.
	virtual Board move(const Board board, bool output = true) {
		struct timeval start, finish;
		unsigned int runtime;

		totalturns++;

		cleardepths();

		int (*oldscorefunc)(const Board & board) = board.scorefunc;

		gettimeofday(&start, NULL);

		Board ret = search_move(board, output);

		gettimeofday(&finish, NULL);

		runtime = ((finish.tv_sec*1000+finish.tv_usec/1000)-(start.tv_sec*1000+start.tv_usec/1000));
		totaltime += runtime;

		print_stats(runtime, output);

		ret.scorefunc = oldscorefunc;
		ret.score = 0;

		return ret;
	}

	virtual void describe() = 0;

	virtual void print_total_stats(){
		printf("Total Turns: %llu, Moves: %llu, Time: %llu s, Moves/s: %u\n", totalturns, totalmoves, totaltime/1000, (unsigned int)(1000.0*totalmoves/totaltime));
	}

protected:

	virtual void print_stats(unsigned int runtime, bool output = true) {
		uint64_t moves = 0;

		for(int i = 0; depths[i]; i++)
			moves += depths[i];
		
		if(output){
			printf("Moves: %llu, Time: %u ms, Moves/s: %u\n", moves, runtime, (unsigned int)(1000.0*moves/runtime));

			printf("Depth 0: %10llu\n", depths[0]);
			for(int i = 1; depths[i]; i++)
				printf("Depth %i: %10llu, %6.2f\n", i, depths[i], (double)depths[i]/depths[i-1]);
		}

		totalmoves += moves;
	}

//search_move must be implemented by the subclass. It takes a board state and returns the next board state
	virtual Board search_move(const Board, bool output) = 0;


//non-pruning version of negamax
	int minimax(Board & board, int depth){
		if(depth == 0 || board.won() >= 0) //end of depth search, or won
			return board.score;

		int ret, best = -2000000000;
		Board children[(36-board.nummoves)*8];
		int numchildren = board.getchildren(children, depth);

		depths[depth-1] += numchildren;

		for(int i = 0; i < numchildren; i++){
			ret = -minimax(children[i], depth - 1);

			if(ret > best)
				best = ret;
		}
		return best;
	}

//play a random game starting from a board state, and return the results of who won	
	int rand_game(Board board){
		int move, rot;
		char won;
	
		while((won = board.won()) < 0){
			do{
				move = rand() % 36;
			}while(board.squares[move]); // don't choose a position that is already taken
			rot = rand() % 8;
			
			board.move(move, rot);
		}
		return won;
	}
};

#endif

