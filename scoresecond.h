#ifndef __SCORESECOND_H_
#define __SCORESECOND_H_

#include "board.h"
#include "scoresimple.h"

class ScoreSecond {
public:
	static int getscore(const Board & board){
		Board children[9];
		uint64_t hashes[8];
		uint64_t * hashend = hashes;
		uint64_t hash;
		bool dohash = (board.nummoves < 10);

		Board * newboard = children;

		*newboard = board;

	//need to include the score for the current board
	// negate the score so it's comparable to the scores calculated below
		newboard->scorefunc = &ScoreSimple::getscore;
		newboard->getscore();
		newboard->score = -newboard->score;

		++newboard;

		for(int spin = 0; spin < 8; spin++){
			*newboard = board; //copy the board

			newboard->move(-1, spin);

			if(dohash){
				hash = newboard->simplehash();

				if(Board::find(hashes, hashend, hash) != hashend){ //found, reset and go to next
					continue;
				}else{ //not found, meaning this is a new move
					*hashend = hash;
					++hashend;
				}
			}

			newboard->scorefunc = &ScoreSimple::getscore;
			newboard->getscore();

			++newboard;
		}

		Board * best = Board::findbest(children, newboard);

		return -best->score; //return the negative of the highest score
	}
};

#endif

