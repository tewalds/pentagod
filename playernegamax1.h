
#include "playernegamax.h"

class PlayerNegamax1 : public PlayerNegamax {
public:

	PlayerNegamax1(){ }

	PlayerNegamax1(int depth, int (*nscorefunc)(const Board & board) = &ScoreSimple::getscore){
		maxdepth = depth;
		scorefunc = nscorefunc;
	}

	~PlayerNegamax1(){
	}

	void describe(){
		printf("class PlayerNegamax1 - an aggressively pruning negamax player searching to depth %i\n", maxdepth);
	}

	Board search_move(Board board, bool output){
		board.scorefunc = scorefunc;

		Board children[288];
		int numchildren = board.getchildren(children, true);
		depths[0] = numchildren;

		int best = 0; //index to element in children with the highest score
		int ret;
		int alpha = 2000000000;

	//can only choose one best move, because all after it also claim to be best, even if they're not
		for(int i = 0; i < numchildren; i++){
			ret = negamax(children[i], 1, -2000000000, alpha);

			if(alpha > ret){
				alpha = ret;
				best = i;
			}
		}

		return children[best];
	}
};

