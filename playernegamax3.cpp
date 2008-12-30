
#include "player.h"

class PlayerNegamax3 : public Player {
	int maxdepth;
	
public:

	PlayerNegamax3(int depth, int (*nscorefunc)(const Board & board) = &ScoreSimple::getscore){
		maxdepth = depth;
		scorefunc = nscorefunc;
	}

	~PlayerNegamax3(){
	}

	void describe(){
		printf("class PlayerNegamax3 - a moderately aggressive pruning negamax player searching to depth %i\n", maxdepth);
	}

	Board search_move(Board board, bool output){
		board.scorefunc = scorefunc;

		Board children[288];
		int numchildren = board.getchildren(children, true);
		depths[maxdepth] = numchildren;

		int ret;
		int alpha = 2000000000;

		//using alpha+1 distinguishes between a move equivalent to a previous one, and one cut off by the pruning
		//without losing much of the speed advantage of passing in an alpha value
		for(int i = 0; i < numchildren; i++){
			children[i].score = ret = negamax(children[i], maxdepth, -2000000000, alpha+1);

			if(alpha > ret)
				alpha = ret;
		}

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

