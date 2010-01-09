/*
 * A MonteCarlo player
 *
 */

#include "player.h"

class PlayerMonteCarlo : public Player {
	int maxruns;
	int nummoves;
public:

	PlayerMonteCarlo(int numruns){
		maxruns = numruns;
	}

	~PlayerMonteCarlo(){
	}

	void describe(){
		printf("class PlayerMonteCarlo - a very simple monte carlo player playing to %i runs per depth 0 position\n", maxruns);
	}

	void print_stats(unsigned int runtime){
		printf("Total moves: %u, moves/s: %u\n", nummoves, (unsigned int)(1000.0*nummoves/runtime));
		totalmoves += nummoves;
	}

	Board search_move(Board board, bool output){
		Board children[288];
		int numchildren = board.getchildren(children, true);

		nummoves = maxruns * numchildren;

		depths[0] += nummoves;

	//reset all the scores, they're going to be calculated by montecarlo below
		for(int i = 0; i < numchildren; i++)
			children[i].score = 0;

	//run maxruns random games, evenly distributed between all the children
		for(int i = 0; i < maxruns; i++){
			for(int j = 0; j < numchildren; j++){
				int result = rand_game(children[j]);
				children[j].score += (result == 0 ? 1 : result == board.turn() ? 0 : 2);
			}
		}

		if(output){
			for(int i = 0; i < numchildren; i++)
				printf("%i ", children[i].score);
			printf("\n");
		}


	//return the best one
		Board::sortchildren(children, children + numchildren); //insertion sort

		int num;
		for(num = 1; num < numchildren && children[num].score == children[0].score; num++) ;

		num = rand() % num;

		return children[num];
	}
};

