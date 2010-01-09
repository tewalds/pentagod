
#ifndef _PLAYERNEGAMAX_H_
#define _PLAYERNEGAMAX_H_

#include "player.h"

class PlayerNegamax : public Player {
protected:
	int maxdepth;

	int negamax(Board & board, const int depth, int alpha, int beta){
		if(depth > maxdepth || board.won() >= 0) //end of depth search, or won
			return board.score;

		int ret;
		Board children[(36-board.nummoves)*8];
		int numchildren = board.getchildren(children, (depth < maxdepth  && board.nummoves < 20));

		depths[depth] += numchildren;

		for(int i = 0; i < numchildren; i++){
			ret = - negamax(children[i], depth + 1, -beta, -alpha);

			if(ret > alpha)
				alpha = ret;

			//the following if statement constitutes alpha-beta pruning
			if(alpha >= beta)
				return beta;
		}
		return alpha;
	}

public:

	PlayerNegamax(){ }

	PlayerNegamax(int depth, int (*nscorefunc)(const Board & board) = &ScoreSimple::getscore){
		maxdepth = depth;
		scorefunc = nscorefunc;
	}

	~PlayerNegamax(){
	}

	void describe(){
		printf("class PlayerNegamax - a simple negamax player searching to depth %i\n", maxdepth);
	}

	Board search_move(Board board, bool output){
		board.scorefunc = scorefunc;

		Board children[288];
		int numchildren = board.getchildren(children, true);
		depths[0] = numchildren;

		for(int i = 0; i < numchildren; i++)
			children[i].score = negamax(children[i], 1, -2000000000, 2000000000);

		Board::sortchildren(children, children + numchildren); //insertion sort

		int num;
		for(num = 1; num < numchildren && children[num].score == children[0].score; num++) ;

		if(output)
			printf("%i best move(s)\n", num);

		num = rand() % num;

//num = 0; // make it consistent so different run times are comparable

		return children[num];
	}
};

#endif

