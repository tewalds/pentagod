/*
 * User controlled player
 *
 */

#include "player.h"

class PlayerHuman : public Player {
public:

	void describe(){
		printf("class PlayerHuman - a user controlled player\n");
	}

	void print_stats(unsigned int runtime){
	}

	Board search_move(Board board){
		int x, y, r;
		char buf[10];

		printf("Input your move as x,y,r eg: e,2,7\n"
		       "    a b c d e f\n"
		       "  1     2 3    \n"
		       "  2               Move the nearest corner towards\n"
		       "  3 1         4   the number you want it at.\n"
		       "  4 8         5\n"
		       "  5            \n"
		       "  6     7 6    \n");
		do{
			x = -1;
			y = -1;
			r = -1;

			printf("Move: ");

			if(!fgets(buf, 10, stdin)){
				printf("Input failed\n");
				exit(1);
			}

			for(int i = 0; i < 10 && buf[i]; i++){
				if(x < 0 && buf[i] >= 'a' && buf[i] <= 'f')
					x = buf[i] - 'a'; // returns 0 based
				else if(y < 0 && buf[i] >= '1' && buf[i] <= '6')
					y = buf[i] - '1'; // returns 0 based
				else if(r < 0 && x >= 0 && y >= 0 && buf[i] >= '1' && buf[i] <= '8')
					r = buf[i] - '0'; // returns 1 based
				//else must be something we don't care about
			}
		}while(x < 0 || y < 0 || r < 1 || board.squares[x + y*6]);

		switch(r){
			case 1: r = 0*4 + 0*2 + 0*1; break;
			case 2: r = 0*4 + 0*2 + 1*1; break;
			case 3: r = 1*4 + 0*2 + 0*1; break;
			case 4: r = 1*4 + 0*2 + 1*1; break;
			case 5: r = 1*4 + 1*2 + 0*1; break;
			case 6: r = 1*4 + 1*2 + 1*1; break;
			case 7: r = 0*4 + 1*2 + 0*1; break;
			case 8: r = 0*4 + 1*2 + 1*1; break;
		}

		board.move(x + y*6, r);

		return board;
	}
};


