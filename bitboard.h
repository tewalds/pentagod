#ifndef __BITBOARD_H_
#define __BITBOARD_H_

#define bittest(a, n)  (a &   (1UL << n))
#define bitset(a, n)   (a |=  (1UL << n))
#define bitunset(a, n) (a &= ~(1UL << n))

class BitBoard {
public:
	uint64_t sides[2];
	char nummoves;
	char outcome; // -1 unknown, 0 tie, 1,2 player win
	int score;
	int (*scorefunc)(const BitBoard & board);

	BitBoard();
	BitBoard(bool newgame);
	BitBoard(uint64_t hash);
	BitBoard(const char * str); //take a string of "0001020102...."
	BitBoard(const Board & board);
	void print() const;
	char won_calc() const;

/*
	int getchildren(BitBoard * children, bool dohash, bool doscore = true);
	void move(int pos, int spin);
	void spinquadrant(int spin);
	void spinpartcw(int x, int y);
	void spinpartccw(int x, int y);
	uint64_t fullhash();
	uint64_t simplehash();
*/
	static const uint64_t winmaps[32];

	inline char pos(int loc) const {
		return (bittest(sides[0], loc) > 0) + 2*(bittest(sides[1],loc) > 0);
	}
	inline char pos(int x, int y) const {
		return pos(y*6 + x);
	}
	inline void setpos(int loc, char side){
		switch(side){
			case 0: bitunset(sides[0], loc); 
					bitunset(sides[1], loc); 
					break;
			case 1: bitset(sides[0], loc); break;
			case 2: bitset(sides[1], loc); break;
			default: break;
		}
	}
	inline void setpos(int x, int y, char side){
		setpos(y*6+x, side);
	}

	char turn() const {
		return nummoves % 2 + 1;
	}
	int getscore(){
		if(!score)
			score = scorefunc(*this);
	//		score = ScoreSimple::getscore2(*this);
		return score;
	}

	char won(){
		if(outcome < 0)
			outcome = won_calc();
		return outcome;
	}


	static BitBoard * findbest(BitBoard * start, BitBoard * end){
		BitBoard * i, *best = start;
		
		for(i = start; i < end; ++i)
			if(i->score > best->score)
				best = i;
		
		return best;
	}

	//sort them by score (insertion sort)
	static void sortchildren(BitBoard * start, BitBoard * end){
		BitBoard * i, * j;
		BitBoard value;

		for(i = start+1; i < end; ++i){
			value = *i;
			j = i-1;

			while(j >= start && j->score > value.score){
				*(j+1) = *j;
				--j;
			}

			*(j+1) = value;
		}
	}

	//sort them by score (shell sort)
	static void sortchildren2(BitBoard * start, BitBoard * end){
		BitBoard * i, * j;
		BitBoard value;
		int incr = (end - start)/2;

		while(incr > 0){
			for(i = start + incr; i < end; ++i){
				value = *i;
				j = i;

				while(j >= start + incr && (j-incr)->score > value.score){
					*j = *(j-incr);
					j -= incr;
				}

				*j = value;
			}

			if(incr == 2)
				incr = 1;
			else
				incr = (int) (incr / 2.2);
		}
	}

	//used for qsort, but otherwise useless
	static int cmpboards(const void * a, const void * b){
		return ((BitBoard *)a)->score - ((BitBoard *)b)->score;
	}
/*
	static uint64_t * find(uint64_t * pos, uint64_t * end, uint64_t val){
		while(pos != end && *pos != val) ++pos;
		return pos;
	}
/*/
	static uint64_t * find(uint64_t * start, uint64_t * end, uint64_t val){
		uint64_t * pos = end-1;
		while(pos >= start && *pos != val) --pos;
		return (pos >= start ? pos : end);
	}
//*/
};

const uint64_t BitBoard::winmaps[32] = {
	//horizontal
	0x1f, 0x3e, 0x7c0, 0xf80, 0x1f000, 0x3e000, 
	0x7c0000, 0xf80000, 0x1f000000, 0x3e000000, 0x7c0000000, 0xf80000000,
	//vertical
	0x1041041, 0x41041040, 0x2082082, 0x82082080, 0x4104104, 0x104104100, 
	0x8208208, 0x208208200, 0x10410410, 0x410410400, 0x20820820, 0x820820800, 
	//diagonal
	0x10204081, 0x810204080, 0x42108400, 0x2108420,
	0x408102040, 0x20408102, 0x1084210, 0x84210800
};

#endif
