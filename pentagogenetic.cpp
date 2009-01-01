
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "board.cpp"
#include "scoresimple.h"

#include "player.h"
#include "playernegamax3.cpp"

struct GenPlayer {
	int32_t map[7];
	Player *player;
	int result;
	int games;
	int generation;
};


const int numgenerations = 20;
const int numrounds = 10;
const int numtested = 8;
const int numrandom = 20;
const int numpromote = 2; //number to move from the random to the tested group

GenPlayer players[numtested + numrandom];


int cmp_players(const void * va, const void * vb){
	GenPlayer * a = (GenPlayer *) va;
	GenPlayer * b = (GenPlayer *) vb;
	return b->result - a->result;
}

void cpvec(int32_t (& a)[7], int32_t (& b)[7]){
	for(int i = 0; i < 7; i++)
		a[i] = b[i];
}

template <int OFFSET>
int32_t getscore(const Board & board){
	return ScoreSimple::getscore2(board, players[OFFSET].map);
}

int run_game(Player *player1, Player *player2){
	Player *players[2];
	players[0] = player1;
	players[1] = player2;

	Board board(1);

	int turn = 0;

	while(board.won() < 0){
		board = players[turn]->move(board, false);

		turn = !turn;
	}

	return board.won();
}

int main(int argc, char **argv){
	srand(time(NULL));
	struct timeval start, finish;

	int32_t (* scorefuncs[numtested + numrandom])(const Board & board);
/*
//generate the scoring functions
//manually loop unroll since the c++ compilers don't do this early enough, and template magic is hard/unreadable
	for(int i = 0; i < numtested + numrandom; i++)
		scorefuncs[i] = &(getscore<i>);
*/
	scorefuncs[0] = &(getscore<0>);
	scorefuncs[1] = &(getscore<1>);
	scorefuncs[2] = &(getscore<2>);
	scorefuncs[3] = &(getscore<3>);
	scorefuncs[4] = &(getscore<4>);
	scorefuncs[5] = &(getscore<5>);
	scorefuncs[6] = &(getscore<6>);
	scorefuncs[7] = &(getscore<7>);
	scorefuncs[8] = &(getscore<8>);
	scorefuncs[9] = &(getscore<9>);
	scorefuncs[10] = &(getscore<10>);
	scorefuncs[11] = &(getscore<11>);
	scorefuncs[12] = &(getscore<12>);
	scorefuncs[13] = &(getscore<13>);
	scorefuncs[14] = &(getscore<14>);
	scorefuncs[15] = &(getscore<15>);
	scorefuncs[16] = &(getscore<16>);
	scorefuncs[17] = &(getscore<17>);
	scorefuncs[18] = &(getscore<18>);
	scorefuncs[19] = &(getscore<19>);
	scorefuncs[20] = &(getscore<20>);
	scorefuncs[21] = &(getscore<21>);
	scorefuncs[22] = &(getscore<22>);
	scorefuncs[23] = &(getscore<23>);
	scorefuncs[24] = &(getscore<24>);
	scorefuncs[25] = &(getscore<25>);
	scorefuncs[26] = &(getscore<26>);
	scorefuncs[27] = &(getscore<27>);

	int32_t init_map[numtested][7] = {
		{0, 3, 8, 23, 61, 100000000, 100000000},
		{0, 3, 6, 12, 24, 100000000, 100000000},
		{0, 3, 9, 27, 81, 100000000, 100000000},
		{0, 3, 10, 31, 95, 100000000, 100000000},
		{0, 3, 11, 37, 129, 100000000, 100000000},
		{0, 3, 12, 48, 192, 100000000, 100000000},
		{0, 3, 13, 59, 265, 100000000, 100000000},
		{0, 3, 15, 75, 375, 100000000, 100000000},
	};

//copy over the initial set	
	for(int i = 0; i < numtested; i++){
		cpvec(players[i].map, init_map[i]);
		players[i].result = 1; //initial weighting
		players[i].generation = 0;
	}

	for(int i = numtested; i < numtested + numrandom; i++)
		cpvec(players[i].map, init_map[0]);

//main loop
	for(int a = 0; a < numgenerations; a++){

	//generate random players based on the previous best tested player
		for(int i = numtested; i < numtested + numrandom; i++){
			cpvec(players[i].map, players[0].map);
			for(int j = 2; j < 5; j++){
				do{
					players[i].map[j] += rand()%(2*j + 1) - j;
				}while(players[i].map[j] <= players[i].map[j-1]);
			}
			players[i].generation = a+1;
		}


	//initialize the players
		for(int i = 0; i < numtested + numrandom; i++){
			players[i].player = new PlayerNegamax3(2, scorefuncs[i]);
			players[i].result = 0;
			players[i].games = 0;
		}

		int num_games = numtested*numrandom*2*numrounds;

		printf("Generation %i of %i:\n", a+1, numgenerations);
		printf("Playing a tournament of %i rounds (%i games) between:\n", numrounds, num_games);
		for(int i = 0; i < numtested + numrandom; i++){
			printf("%2i:", i+1);
			for(int j = 1; j < 5; j++)
				printf("%4i", players[i].map[j]);
			printf(" (%2i)\n", players[i].generation);
		}

		gettimeofday(&start, NULL);

		int result;
		int b = 0;
		for(int a = 0; a < numrounds; a++){
			for(int i = 0; i < numtested; i++){
				for(int j = numtested; j < numrandom+numtested; j++){
					printf("Playing game %i: round %i, player %i vs player %i   \r", ++b, a+1, i+1, j+1); fflush(0);
					result = run_game(players[i].player, players[j].player);

					switch(result){
						case 1: players[i].result++; players[j].result--; break;
						case 2: players[i].result--; players[j].result++; break;
					}

					printf("Playing game %i: round %i, player %i vs player %i   \r", ++b, a+1, j+1, i+1); fflush(0);
					result = run_game(players[j].player, players[i].player);

					switch(result){
						case 1: players[j].result--; players[i].result++; break;
						case 2: players[j].result++; players[i].result--; break;
					}
					players[i].games += 2;
					players[j].games += 2;
				}
			}
		}

		gettimeofday(&finish, NULL);
		int runtime = ((finish.tv_sec*1000+finish.tv_usec/1000)-(start.tv_sec*1000+start.tv_usec/1000));

	//sort the players according to their results
		qsort(players, numtested, sizeof(GenPlayer), cmp_players);             //sort the tested ones
		qsort(players + numtested, numrandom, sizeof(GenPlayer), cmp_players); //sort the random ones

	//output the results
		printf("Results:                                                                       \n");
		for(int i = 0; i < numtested + numrandom; i++){
			printf("%2i:", i+1);
			for(int j = 1; j < 5; j++)
				printf("%4i", players[i].map[j]);
			printf(" (%2i) -> %4i /%4i  : ", players[i].generation, players[i].result, players[i].games);
			players[i].player->print_total_stats();
		}
		printf("Played %i games, Total Time: %i s, Average Time: %.2f s\n", num_games, runtime/1000, 1.0*runtime/(1000*num_games));
		printf("-------------------------------------------\n");

	//replace the worst tested with the best random
		for(int i = 0; i < numpromote; i++)
			players[numtested - 1 - i] = players[numtested + i];


	//cleanup
//segfaults for some reason, leak a bit of memory instead of segfaulting
//		for(int i = 0; i < numtested + numrandom; i++)
//			delete players[i].player;
	}

	return 0;
}


