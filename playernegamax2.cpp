
#include "player.h"

class PlayerNegamax2 : public Player {
	int maxdepth;
	
public:

	PlayerNegamax2(int depth, int (*nscorefunc)(const Board & board) = &ScoreSimple::getscore){
		maxdepth = depth;
		scorefunc = nscorefunc;
	}

	~PlayerNegamax2(){
	}

	void describe(){
		printf("class PlayerNegamax2 - a simple negamax player searching to depth %i\n", maxdepth);
	}

	Board search_move(Board board){
		board.scorefunc = scorefunc;

		Board children[288];
		int numchildren = board.getchildren(children, true);
		depths[maxdepth] = numchildren;

		for(int i = 0; i < numchildren; i++)
			children[i].score = negamax(children[i], maxdepth, -2000000000, 2000000000);

		Board::sortchildren(children, children + numchildren); //insertion sort

		int num;
		for(num = 1; num < numchildren && children[num].score == children[0].score; num++) ;

printf("%i best move(s)\n", num);

		num = rand() % num;

		return children[num];
	}
};

