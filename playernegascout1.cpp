
#include "player.h"

class PlayerNegascout1 : public Player {
	int maxdepth;
	
public:

	PlayerNegascout1(int depth, int (*nscorefunc)(const Board & board) = &ScoreSimple::getscore){
		maxdepth = depth;
		scorefunc = nscorefunc;
	}
	
	~PlayerNegascout1(){
	}

	void describe(){
		printf("class PlayerNegascout1 - a simple negascout player searching to depth %i\n", maxdepth);
	}

	//Take a board position, and using the negamax algorythm, return the best new board position.
	Board search_move(Board board){
		board.scorefunc = scorefunc;
		
		Board children[288];
		int numchildren = board.getchildren(children, true);
		depths[maxdepth] = numchildren;

		for(int i = 0; i < numchildren; i++)
			children[i].score = negascout(children[i], maxdepth, -2000000000, 2000000000);

		Board::sortchildren(children, children + numchildren); //insertion sort

		int num;
		for(num = 1; num < numchildren && children[num].score == children[0].score; num++) ;

printf("%i best move(s)\n", num);

		num = rand() % num;

		return children[num];
	}
};

