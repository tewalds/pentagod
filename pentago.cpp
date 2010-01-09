
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "tournament.cpp"

#include "board.cpp"
#include "scoresimple.h"
#include "scoresecond.h"

#include "player.h"
#include "playerhuman.cpp"
#include "playernegamax.h"
#include "playernegamax1.h"
#include "playernegamax2.h"
#include "playernegamax3.h"
#include "playernegamax4.h"
#include "playernegascout.h"
#include "playermontecarlo.cpp"
#include "playeruct.cpp"


int main(int argc, char **argv){
	srand(time(NULL));

	int numrounds = 3;

	Player *players[100];
	int num = 0;




	players[num++] = new PlayerNegamax2(2);
//	players[num++] = new PlayerNegamax2(3);
//	players[num++] = new PlayerUCT(5000000, 5000000, 0.5);
//	players[num++] = new PlayerUCT(50000, 50000000, 1.0);
	players[num++] = new PlayerUCT(100000, 50000000, 1.0);
//	players[num++] = new PlayerUCT(5000000, 5000000, 1.5);
//	players[num++] = new PlayerMonteCarlo(10000);
//	players[num++] = new PlayerHuman();

	Tournament tourney(num, players, numrounds, 1);

	tourney.run();

	return 0;
}


