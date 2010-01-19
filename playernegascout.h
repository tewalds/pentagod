
#ifndef _PLAYERNEGASCOUT_H_
#define _PLAYERNEGASCOUT_H_

#include "player.h"

class PlayerNegascout : public Player {
protected:
	int maxdepth;

	int negascout(Board & board, const int depth, int alpha, int beta){
		if(depth > maxdepth || board.won() >= 0) //end of depth search, or won
			return board.score;

		Board children[(36-board.nummoves)*8];
		int numchildren = board.getchildren(children, (depth < maxdepth  && board.nummoves < 20));

		depths[depth] += numchildren;

		int b = beta;
		for(int i = 0; i < numchildren; i++){
			int value = -negascout(children[i], depth + 1, -b, -alpha);

			if(value > alpha && value < beta && i > 0) // re-search
				value = -negascout(children[i], depth + 1, -beta, -alpha);

			if(value > alpha)
				alpha = value;

			if(alpha >= beta)
				return beta; //alpha for fail-soft

			b = alpha + 1; // set up null window
		}
		return alpha;
	}
	
public:

	PlayerNegascout() { }

	PlayerNegascout(int depth, int (*nscorefunc)(const Board & board) = &ScoreSimple::getscore){
		maxdepth = depth;
		scorefunc = nscorefunc;
	}
	
	~PlayerNegascout(){
	}

	void describe(){
		printf("class PlayerNegascout - a simple negascout player searching to depth %i\n", maxdepth);
	}

	Board search_move(Board board, bool output){
		board.scorefunc = scorefunc;
		
		Board children[288];
		int numchildren = board.getchildren(children, true);
		depths[0] = numchildren;

		for(int i = 0; i < numchildren; i++)
			children[i].score = negascout(children[i], 1, -2000000000, 2000000000);

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

