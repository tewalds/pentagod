#ifndef __BOARD_H_
#define __BOARD_H_

class Board {
public:
	char squares[36];
	char turn;
	char nummoves;
	char outcome; // -1 unknown, 0 tie, 1,2 player win
	int score;
	int (*scorefunc)(const Board & board);

	Board();
	Board(int newgame);
	Board(const char * str, int side); //take a string of "0001020102...."
	void print() const;
	char won_calc() const;

	int getchildren(Board * children, bool dohash, bool doscore = true);
	void move(int pos, int spin);
	void spinquadrant(int spin);
	void spinpartcw(int x, int y);
	void spinpartccw(int x, int y);
	uint64_t fullhash();
	uint64_t simplehash();

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


	static Board * findbest(Board * start, Board * end){
		Board * i, *best = start;
		
		for(i = start; i < end; ++i)
			if(i->score > best->score)
				best = i;
		
		return best;
	}

	//sort them by score (insertion sort)
	static void sortchildren(Board * start, Board * end){
		Board * i, * j;
		Board value;

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
	static void sortchildren2(Board * start, Board * end){
		Board * i, * j;
		Board value;
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
		return ((Board *)a)->score - ((Board *)b)->score;
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

#endif
