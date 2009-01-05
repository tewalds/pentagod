// run with: % g++ -Wall -fno-strict-aliasing -O3 -funroll-loops -lrt bitboardscoretest.cpp -o bitboardscoretest && ./bitboardscoretest

#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include <sys/types.h>

#include "board.cpp"
#include "bitboard.cpp"
#include "scoresimple.h"
#include "scorebitboard.h"

int main(int argc, char **argv){
	srand(time(NULL));

	
	const int num = 100000000;
	
	struct timeval start, finish;
	unsigned int runtime;

//	uint64_t hash = 1694896204500;
//	uint64_t hash = 4696942636;
	uint64_t hash = 573387235767;


	printf("Testing %lu\n", hash);

	Board board(hash);
	board.print();
	
	BitBoard bitboard(hash);
	bitboard.print();

	gettimeofday(&start, NULL);

	for(int i = 0; i < num; i++)
		board.score = ScoreSimple::getscore(board);


	gettimeofday(&finish, NULL);

	runtime = ((finish.tv_sec*1000+finish.tv_usec/1000)-(start.tv_sec*1000+start.tv_usec/1000));
	printf("   Board Time: %u ms, rate: %u/s, Score: %i\n", runtime, (unsigned int)(1000.0*num/runtime), ScoreSimple::getscore(board));


	gettimeofday(&start, NULL);

	for(int i = 0; i < num; i++)
		bitboard.score = ScoreBitBoard::getscore(bitboard);

	gettimeofday(&finish, NULL);

	runtime = ((finish.tv_sec*1000+finish.tv_usec/1000)-(start.tv_sec*1000+start.tv_usec/1000));
	printf("BitBoard Time: %u ms, rate: %u/s, score: %i\n", runtime, (unsigned int)(1000.0*num/runtime), ScoreBitBoard::getscore(bitboard));

	return 0;
}

