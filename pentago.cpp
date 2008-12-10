
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
#include "playernegamax1.cpp"
#include "playernegamax2.cpp"
#include "playernegamax3.cpp"
#include "playernegamax4.cpp"
#include "playernegascout1.cpp"
#include "playermontecarlo.cpp"
#include "playeruct.cpp"


int main(int argc, char **argv){
	srand(time(NULL));

	int numrounds = 2;

	Player *players[100];
	int num = 0;




	players[num++] = new PlayerNegamax3(2, &ScoreSimple::getscore2);
	players[num++] = new PlayerNegamax3(3, &ScoreSimple::getscore2);
	players[num++] = new PlayerUCT(5000000, 5000000, 0.5);
	players[num++] = new PlayerUCT(5000000, 5000000, 1.0);
	players[num++] = new PlayerUCT(5000000, 5000000, 1.5);


//	players[num++] = new PlayerUCT(300000, 500000);
//	players[num++] = new PlayerUCT(1000000, 500000);

//	players[num++] = new PlayerNegamax3(3);
//	players[num++] = new PlayerNegamax3(3, &ScoreSimple::getscore2);
//	players[num++] = new PlayerHuman();
//	players[num++] = new PlayerUCT(1000000, 100000);
//	players[num++] = new PlayerUCT(1000000, 100000);
//	players[num++] = new PlayerNegamax3(3);
//	players[num++] = new PlayerNegamax3(3, &ScoreSimple::getscore2);
//	players[num++] = new PlayerHuman();
//	players[num++] = new PlayerNegascout1(3);//, &ScoreSecond::getscore);
//	players[num++] = new PlayerNegamax1(3, &ScoreSecond::getscore);

	Tournament tourney(num, players, numrounds, 1);

	tourney.run();

	return 0;
}


