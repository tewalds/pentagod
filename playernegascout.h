
#ifndef _PLAYERNEGASCOUT_H_
#define _PLAYERNEGASCOUT_H_

#include "player.h"

class PlayerNegascout : public Player {
	int maxdepth;
	
public:

	PlayerNegascout(int depth, int (*nscorefunc)(const Board & board) = &ScoreSimple::getscore){
		maxdepth = depth;
		scorefunc = nscorefunc;
	}
	
	~PlayerNegascout(){
	}

	void describe(){
		printf("class PlayerNegascout - a simple negascout player searching to depth %i\n", maxdepth);
	}

	int negascout(Board & board, int depth, int alpha, int beta){
		if(depth >= maxdepth || board.won() >= 0) //end of depth search, or won
			return board.score;

		Board children[(36-board.nummoves)*8];
		int numchildren = board.getchildren(children, depth);

		depths[depth] += numchildren;

		int b = alpha + 1;
		for(int i = 0; i < numchildren; i++){
			alpha = -negascout(children[i], depth + 1, -b, -alpha);

			if(alpha >= beta) // Beta cut-off
				return alpha;

			if(alpha >= b){ // check if null-window failed high
				alpha = -negascout(children[i], depth + 1, -beta, -alpha); // full re-search

				if(alpha >= beta) // Beta cut-off
					return alpha;
			}

			b = alpha + 1; // set new null window
		}
		return alpha;
	}

	Board search_move(Board board, bool output){
		board.scorefunc = scorefunc;
		
		Board children[288];
		int numchildren = board.getchildren(children, true);
		depths[maxdepth] = numchildren;

		for(int i = 0; i < numchildren; i++)
			children[i].score = negascout(children[i], maxdepth, -2000000000, 2000000000);

		Board::sortchildren(children, children + numchildren); //insertion sort

		int num;
		for(num = 1; num < numchildren && children[num].score == children[0].score; num++) ;

		if(output)
			printf("%i best move(s)\n", num);

		num = rand() % num;

		return children[num];
	}
};

#endif

