#ifndef __SCOREBITBOARD_H_
#define __SCOREBITBOARD_H_

#include "bitboard.h"
#include "bitcount.h"

//#define bitcount(x) __builtin_popcountl(x)
//#define bitcount(n) parallel_bitcount(n)
//#define bitcount(n) sparse_ones_bitcount(n)
#define bitcount(n) sparse_ones_bitcount_6bit(n)
//#define bitcount(x) precomputed_bitcount(x)
//#define bitcount(x) precomputed_bitcount2(x)

class ScoreBitBoard {
public:
	//Gives the score for a single line of a varied length
	//actual definition at the bottom of the file because of C++ weirdness
	static const int32_t scoremap[7];

	static int32_t getscore(const BitBoard & board){
		return getscore1(board, scoremap);
	}

	static int32_t getscore1(const BitBoard & board, const int32_t (& scoremap)[7]){
		int32_t scr = 0;

		for(int i = 0; i < 32; i++){
			if(board.sides[0] & BitBoard::winmaps[i]){
				if(!(board.sides[1] & BitBoard::winmaps[i]))
					scr += scoremap[bitcount(board.sides[0] & BitBoard::winmaps[i])];
			}else{
				if(board.sides[1] & BitBoard::winmaps[i])
					scr -= scoremap[bitcount(board.sides[1] & BitBoard::winmaps[i])];
			}
		}

		return (board.turn() == 1 ? scr : -scr);
	}

};

const int32_t ScoreBitBoard::scoremap[7] = {0, 1, 4, 16, 256, 100000000, 100000000};

#endif

