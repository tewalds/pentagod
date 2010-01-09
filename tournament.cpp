
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "board.h"
#include "scoresimple.h"
#include "scoresecond.h"

#include "player.h"
#include "tqueue.h"

#define MAX_THREADS 64

struct Pair {
	int nums[2];
	
	Pair(int i, int j){
		nums[0] = i;
		nums[1] = j;
	}
};

class Tournament {
	tqueue<Pair> games;
	tqueue<int> response;
	
	pthread_t thread[MAX_THREADS];
	int num_threads;
	
	int num_players;
	Player **players;
	int num_rounds;
	
public:
	int * results;
	unsigned int runtime;


	Tournament(int nnum_players, Player **nplayers, int nnum_rounds = 1, int nnum_threads = MAX_THREADS){
		num_players = nnum_players;
		players = nplayers;
		num_rounds = nnum_rounds;

		results = new int[num_players*num_players];
		for(int i = 0; i < num_players*num_players; i++)
			results[i] = 0;

		num_threads = (nnum_threads < MAX_THREADS ? nnum_threads : MAX_THREADS);

	//set the upper bound at half the number of players, otherwise there will be threads spinning on the player->playing lock
		if(num_threads > num_players/2)
			num_threads = num_players/2;

		num_threads = 1;

		if(num_threads > 1){
			for(int i = 0; i < num_threads; i++)
				pthread_create(
					&thread[i], 
					NULL, 
					(void* (*)(void*)) &Tournament::gameRunner, 
					this);
		}
	}
	
	~Tournament(){
		if(num_threads > 1){
			for(int i = 0; i < num_threads; i++)
				pthread_join(thread[i], NULL);
		}
	
		delete[] results;
	}


protected:

	//must be static so that pthread will call it. This means it must take a pointer to this, and do all calls through it
	static void * gameRunner(void * blah){
		Tournament * tourney = (Tournament *) blah;

		int result;
		int i, j;
		int num = tourney->num_players;

		while(1){
			Pair * pair = tourney->games.pop();
			i = pair->nums[0];
			j = pair->nums[1];

			pthread_mutex_lock(&(tourney->players[i]->lock));
			pthread_mutex_lock(&(tourney->players[j]->lock));

			if(tourney->players[i]->playing || tourney->players[j]->playing){
				pthread_mutex_unlock(&(tourney->players[i]->lock));
				pthread_mutex_unlock(&(tourney->players[j]->lock));

				tourney->games.push(pair);
			}else{
				tourney->players[i]->playing = true;
				tourney->players[j]->playing = true;

				pthread_mutex_unlock(&(tourney->players[i]->lock));
				pthread_mutex_unlock(&(tourney->players[j]->lock));


				result = tourney->run_game(tourney->players[i], tourney->players[j]);


				pthread_mutex_lock(&(tourney->players[i]->lock));
				pthread_mutex_lock(&(tourney->players[j]->lock));

				tourney->players[i]->playing = false;
				tourney->players[j]->playing = false;

				pthread_mutex_unlock(&(tourney->players[i]->lock));
				pthread_mutex_unlock(&(tourney->players[j]->lock));


				if(result == 1)
					tourney->results[i*num+j]++;
				else if(result == 2)
					tourney->results[j*num+i]++;

				tourney->response.push(NULL);
				delete pair;
			}
		}

		return NULL;
	}

	int run_game(Player *player1, Player *player2, bool output = true){
		Player *players[2];
		players[0] = player1;
		players[1] = player2;

		Board board(true);

		int turn = 0;

		if(output){
			printf("--------------------------------------------------------------\n\n");
			printf("Player 1: "); players[0]->describe();
			printf("Player 2: "); players[1]->describe();
			printf("\n");
		}

	//	gettimeofday(&start, NULL);

		while(board.won() < 0){
			if(output)
				printf("Turn %i, Player: %c\n", board.nummoves+1, (board.turn() == 1 ? 'X' : 'O'));

			board = players[turn]->move(board, output);

			if(output)
				board.print();

			turn = !turn;
		}
	//	gettimeofday(&finish, NULL);

		if(output){
			if(board.won() > 0)
				printf("Player %c won!\n", (board.won() == 1 ? 'X' : 'O'));
			else
				printf("Tie game!\n");

			printf("\n\n");
		}

		return board.won();
	}

public:

	void run(bool output = true){
		struct timeval start, finish;

		int num_games = num_players*(num_players-1)*num_rounds;

		printf("Playing a tournament of %i rounds (%i games) between:\n", num_rounds, num_games);

		for(int i = 0; i < num_players; i++){
			printf("Player %i: ", i+1); players[i]->describe();
		}
		printf("\n\n\n");

		gettimeofday(&start, NULL);

		if(num_threads == 1){
		//just play them directly
			for(int a = 0; a < num_rounds; a++){
				for(int i = 0; i < num_players; i++){
					for(int j = 0; j < num_players; j++){
						if(i != j){
							int result = run_game(players[i], players[j], output);

							if(result == 1)
								results[i*num_players+j]++;
							else if(result == 2)
								results[j*num_players+i]++;
						}
					}
				}
			}
		}else{
		//queue up the games to play
			for(int a = 0; a < num_rounds; a++)
				for(int i = 0; i < num_players; i++)
					for(int j = 0; j < num_players; j++)
						if(i != j)
							games.push(new Pair(i, j));

		//wait for the games to finish playing
			for(int i = 0; i < num_games; i++)
				response.pop();
		}

		gettimeofday(&finish, NULL);
		runtime = ((finish.tv_sec*1000+finish.tv_usec/1000)-(start.tv_sec*1000+start.tv_usec/1000));
//*
		printf("\n\n");


		printf("-----------------------------------------\n\n");
		printf("Results:\n");

		printf("Played a tournament of %i rounds (%i games) between:\n", num_rounds, num_games);

		for(int i = 0; i < num_players; i++){
			printf("Player %i: ", i+1); players[i]->describe();
		}
		printf("\n");
//*/

		for(int i = 0; i < num_players; i++){
			printf("Player %i: ", i+1); players[i]->print_total_stats();
		}
		printf("\n");
	
	//win vs loss matrix
		printf("Win vs Loss Matrix:\n");
		printf("   ");
		for(int i = 0; i < num_players; i++)
			printf(" %2i", i+1);
		printf("\n");

		for(int i = 0; i < num_players; i++){
			int wins = 0;
			int losses = 0;
			printf("%2i:", i+1);
			for(int j = 0; j < num_players; j++){
				if(i == j){
					printf("   ");
				}else{
					printf(" %2i", results[i*num_players+j]);
					wins += results[i*num_players+j];
					losses += results[j*num_players+i];
				}
			}
			printf(" : %i wins, %i losses, %i ties\n", wins, losses, (num_players-1)*2*num_rounds - wins - losses);
		}
		printf("\n");

		printf("Played %i games, Total Time: %i s, Average Time: %i s\n", num_games, runtime/1000, runtime/(1000*num_games));
	}
};


