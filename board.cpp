
#include "board.h"
#include "scoresimple.h"

#define xy(x,y) ((x) + ((y)*6))

Board::Board(){
}

Board::Board(int newgame){
	nummoves = 0;
	score = 0;
	outcome = -1;
	for(int i = 0; i < 36; i++)
		squares[i] = 0;

	scorefunc = & ScoreSimple::getscore;
}

Board::Board(const char * str){
	nummoves = 0;
	score = 0;
	outcome = -1;

	for(int i = 0; i < 36; i++){
		squares[i] = str[i] - '0';

		if(squares[i])
			nummoves++;
	}

	scorefunc = &ScoreSimple::getscore;
}

void Board::print() const {
	char buf[74];
	char * ptr = buf;

	for(int i = 0; i < 36; ++i){
		*ptr++ = (squares[i] ? (squares[i] == 1 ? 'X' : 'O') : '.');
		*ptr++ = (i % 6 == 5 ? '\n' : ' ');
	}

	*ptr++ = '\n';
	*ptr++ = '\0';

	printf("%s", buf);
}

char Board::won_calc() const {
	char side;
	char wonside = 0;

//check horizontal lines
	for(int y = 0; y < 6; y++)
		if((side = squares[xy(1,y)]) && side == squares[xy(2,y)] &&
		    side == squares[xy(3,y)] &&	side == squares[xy(4,y)] &&
		   (side == squares[xy(0,y)] || side == squares[xy(5,y)]) )
			wonside |= side;

//check vertical lines
	for(int x = 0; x < 6; x++)
		if((side = squares[xy(x,1)]) && side == squares[xy(x,2)] &&
		    side == squares[xy(x,3)] &&	side == squares[xy(x,4)] &&
		   (side == squares[xy(x,0)] || side == squares[xy(x,5)]) )
			wonside |= side;


//check center diagonal lines
	if((side = squares[xy(1,1)]) && side == squares[xy(2,2)] &&
	    side == squares[xy(3,3)] &&	side == squares[xy(4,4)] &&
	   (side == squares[xy(0,0)] || side == squares[xy(5,5)]) )
		wonside |= side;

	if((side = squares[xy(4,1)]) && side == squares[xy(3,2)] &&
	    side == squares[xy(2,3)] &&	side == squares[xy(1,4)] &&
	   (side == squares[xy(5,0)] || side == squares[xy(0,5)]) )
		wonside |= side;


//check the off-center diagonal lines
	if((side = squares[xy(0,1)]) && side == squares[xy(1,2)] &&
	    side == squares[xy(2,3)] &&	side == squares[xy(3,4)] &&
	    side == squares[xy(4,5)])
		wonside |= side;

	if((side = squares[xy(1,0)]) && side == squares[xy(2,1)] &&
	    side == squares[xy(3,2)] &&	side == squares[xy(4,3)] &&
	    side == squares[xy(5,4)])
		wonside |= side;

	if((side = squares[xy(0,4)]) && side == squares[xy(1,3)] &&
	    side == squares[xy(2,2)] &&	side == squares[xy(3,1)] &&
	    side == squares[xy(4,0)])
		wonside |= side;

	if((side = squares[xy(1,5)]) && side == squares[xy(2,4)] &&
	    side == squares[xy(3,3)] &&	side == squares[xy(4,2)] &&
	    side == squares[xy(5,1)])
		wonside |= side;

	if(nummoves >= 36 || wonside == 3) //wonside == 3 when both sides win simultaneously
		return 0;
	if(wonside == 0)
		return -1;
	return wonside;
}

int Board::getchildren(Board * children, bool dohash, bool doscore){
	uint64_t hashes[(36 - nummoves)*8];
	uint64_t * hashend = hashes;
	uint64_t hash;

	Board * newboard = children;

//find all the boards, and put them in the children array
	for(int pos = 0; pos < 36; pos++){
		if(!squares[pos]){ //is a valid place to put a piece
			for(int spin = 0; spin < 8; spin++){
				*newboard = *this; //copy the board

				newboard->move(pos, spin);

				if(dohash){
					if(nummoves < 7) //7 is best so far, haven't tested 8+
						hash = newboard->fullhash();
					else
						hash = newboard->simplehash();

					if(find(hashes, hashend, hash) != hashend){ //found, reset and go to next
						continue;
					}else{ //not found, meaning this is a new move
						*hashend = hash;
						++hashend;
					}
				}

				if(doscore)
					newboard->getscore();

				++newboard;
			}
		}
	}

	if(doscore){
//		sortchildren(children, newboard); //insertion sort
		sortchildren2(children, newboard); //shell sort
//		qsort(children, (newboard - children), sizeof(Board), cmpboards); //built-in quick sort
	}

	return (newboard - children);
}



//apply the move characterised by pos and spin.
//pos is a value between 0 and 35 inclusive, where pos = x + y*6;
//if pos is negative, don't place a piece at all
//spin is both the quadrant and the direction.
// - bit 2 is the x quadrant (0 for left, 1 for right)
// - bit 1 is the y quadrant (0 for top, 1 for bottom)
// - bit 0 is the direction  (0 for clockwise, 1 for counter-clockwise)
void Board::move(int pos, int spin){
	score = 0;

	if(pos >= 0)
		squares[pos] = turn();
	spinquadrant(spin);

	nummoves++;
}

void Board::spinquadrant(int spin){
	switch(spin){
	case 0: spinpartcw (0, 0); break;
	case 1: spinpartccw(0, 0); break;
	case 2: spinpartcw (0, 3); break;
	case 3: spinpartccw(0, 3); break;
	case 4: spinpartcw (3, 0); break;
	case 5: spinpartccw(3, 0); break;
	case 6: spinpartcw (3, 3); break;
	case 7: spinpartccw(3, 3); break;
	}
}

inline void Board::spinpartcw(int x, int y){
	int temp;

//corners
	temp                  = squares[xy(x+0, y+0)];
	squares[xy(x+0, y+0)] = squares[xy(x+0, y+2)];
	squares[xy(x+0, y+2)] = squares[xy(x+2, y+2)];
	squares[xy(x+2, y+2)] = squares[xy(x+2, y+0)];
	squares[xy(x+2, y+0)] = temp;

//sides
	temp                  = squares[xy(x+0, y+1)];
	squares[xy(x+0, y+1)] = squares[xy(x+1, y+2)];
	squares[xy(x+1, y+2)] = squares[xy(x+2, y+1)];
	squares[xy(x+2, y+1)] = squares[xy(x+1, y+0)];
	squares[xy(x+1, y+0)] = temp;
}

inline void Board::spinpartccw(int x, int y){
	int temp;

//corners
	temp                  = squares[xy(x+0, y+0)];
	squares[xy(x+0, y+0)] = squares[xy(x+2, y+0)];
	squares[xy(x+2, y+0)] = squares[xy(x+2, y+2)];
	squares[xy(x+2, y+2)] = squares[xy(x+0, y+2)];
	squares[xy(x+0, y+2)] = temp;

//sides
	temp                  = squares[xy(x+1, y+0)];
	squares[xy(x+1, y+0)] = squares[xy(x+2, y+1)];
	squares[xy(x+2, y+1)] = squares[xy(x+1, y+2)];
	squares[xy(x+1, y+2)] = squares[xy(x+0, y+1)];
	squares[xy(x+0, y+1)] = temp;
}

uint64_t Board::fullhash(){
	uint64_t hash = 0;
	uint64_t h = 0;

	int x, y;

// 1 - left -> down
	h = 0;
	for(y = 0; y < 6; y++)
		for(x = 0; x < 6; x++)
			h = (h * 3) + squares[xy(x,y)];

	if(!hash || hash > h)
		hash = h;

// 2 down -> left
	h = 0;
	for(x = 0; x < 6; x++)
		for(y = 0; y < 6; y++)
			h = (h * 3) + squares[xy(x,y)];

	if(!hash || hash > h)
		hash = h;

// 3 - right -> down
	h = 0;
	for(y = 0; y < 6; y++)
		for(x = 5; x >= 0; x--)
			h = (h * 3) + squares[xy(x,y)];

	if(!hash || hash > h)
		hash = h;

// 4 down -> right
	h = 0;
	for(x = 5; x >= 0; x--)
		for(y = 0; y < 6; y++)
			h = (h * 3) + squares[xy(x,y)];

	if(!hash || hash > h)
		hash = h;

// 5 - left -> up
	h = 0;
	for(y = 5; y >= 0; y--)
		for(x = 0; x < 6; x++)
			h = (h * 3) + squares[xy(x,y)];

	if(!hash || hash > h)
		hash = h;

// 6 up -> left
	h = 0;
	for(x = 0; x < 6; x++)
		for(y = 5; y >= 0; y--)
			h = (h * 3) + squares[xy(x,y)];

	if(!hash || hash > h)
		hash = h;

// 7 - right -> up
	h = 0;
	for(y = 5; y >= 0; y--)
		for(x = 5; x >= 0; x--)
			h = (h * 3) + squares[xy(x,y)];

	if(!hash || hash > h)
		hash = h;

// 8 up -> right
	h = 0;
	for(x = 5; x >= 0; x--)
		for(y = 5; y >= 0; y--)
			h = (h * 3) + squares[xy(x,y)];

	if(!hash || hash > h)
		hash = h;

	return hash;
}

//return a hash of this board, as is.
uint64_t Board::simplehash(){
	uint64_t hash = 0;

	for(int i = 0; i < 36; i++)
		hash = (hash * 3) + squares[i];

	return hash;
}

