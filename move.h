
#pragma once

#include <stdint.h>
#include <cstdlib>
#include <cassert>
#include "string.h"
#include "log.h"

enum MoveSpecial {
	M_SWAP    = -1, //-1 so that adding 1 makes it into a valid move
	M_RESIGN  = -2,
	M_NONE    = -3,
	M_UNKNOWN = -4,
};

struct Move {
	int      l : 8; //location = MoveSpecial | y*6+x
	unsigned r : 4; //rotation = 0-7
	unsigned o : 4; //orientation = 0-7 | 8=unoriented
//	int8_t l, r, o; //location, rotation, orientation

/* location =
 0  1  2  3  4  5
 6  7  8  9 10 11
12 13 14 15 16 17
18 19 20 21 22 23
24 25 26 27 28 29
30 31 32 33 34 35

rotation:
  1 2
0     3
7     4
  6 5

quadrants:
01
32

quadrant  = rotation >> 1
direction = rotation & 1

orientation = orientation of the board when this move was created
              alternatively, this is the location once the board has been rotated to this orientation
              This is NOT where the piece would be on a 0-orientation board (unless it is a 0-orientation move).
              a->b != b->a

*/


	Move(MoveSpecial a = M_UNKNOWN, unsigned int O = 8) : l(a), r(14), o(O) { } //big r so it will always wrap to l=0 with swap
	Move(int L, unsigned int R, unsigned int O = 8) : l(L), r(R), o(O) { }
	Move(unsigned int X, unsigned int Y, unsigned int R, unsigned int O) : l(Y*6 + X), r(R), o(O) { }

	Move(const std::string & str, unsigned int O = 8){
		if(     str == "swap"    ){ l = M_SWAP;    r = 14; o = O; }
		else if(str == "resign"  ){ l = M_RESIGN;  r = 14; o = O; }
		else if(str == "none"    ){ l = M_NONE;    r = 14; o = O; }
		else if(str == "unknown" ){ l = M_UNKNOWN; r = 14; o = O; }
		else if(str.length() <= 2){ l = M_NONE;    r = 14; o = O; }
		else{
			unsigned int y = tolower(str[0]) - 'a'; //[abcdef]
			unsigned int x = str[1] - '1';          //[123456]
			l = y*6+x;

			char c = tolower(str[2]);
			if(     c >= '1' && c <= '8') r = c - '1'; //[12345678]
			else if(c >= 's' && c <= 'z') r = c - 's'; //[stuvwxyz]
			else if(c >= 'a' && c <= 'h') r = c - 'a'; //[abcdefgh]
			else r = 0; //unknown, but do something

			if(str.length() == 5) {
				assert(O == 8 && str[3] == ':');
				o = str[4] - '0';
			} else {
				o = O;
			}
		}
	}

	int loc()                  const { return l; }
	unsigned int x()           const { return l % 6; }
	unsigned int y()           const { return l / 6; }
	unsigned int quadrant()    const { return r >> 1; }
	unsigned int direction()   const { return r & 1; }
	unsigned int orientation() const { return o; }

	Move rotate(unsigned int other) const {
		if(l < 0) //special
			return *this;

		assert(o <= 8 && other <= 8);

		if(other == o) //already the correct orientation
			return *this;

		if(o == 8 && other < 8) //unoriented move, default to board
			return Move(l, r, other);

		if(o == 8 || other == 8){ // oriented moves are incompatible with unoriented boards
			printf("rotate %s to %i\n", to_s(true).c_str(), other);
			assert(o <= 7 && other <= 7);
		}

/*
flip move:  Move(y, x, (9-r)&7). Works because (9-1)&7=8&7=0, (9-1)&7=8&7=0
rotate cw:  Move(5-y,   x, (r+2)&7)
rotate 180: Move(5-x, 5-y, (r+4)&7)
rotate ccw: Move(  y, 5-x, (r+6)&7)
*/

		unsigned int c = o*8 + other;
		switch(c){
		case  0: case 011: case 022: case 033: case 044: case 055: case 066: case 077: return *this;
		                                                                                    //Move(  x(),   y(),       r, other);
		case 01: case 012: case 023: case 030: case 047: case 054: case 065: case 076: return Move(5-y(),   x(), (r+2)&7, other);
		case 02: case 013: case 020: case 031: case 046: case 057: case 064: case 075: return Move(5-x(), 5-y(), (r+4)&7, other);
		case 03: case 010: case 021: case 032: case 045: case 056: case 067: case 074: return Move(  y(), 5-x(), (r+6)&7, other);

		case 04: case 015: case 026: case 037: case 040: case 051: case 062: case 073: return Move(  y(),   x(), ( 9-r)&7, other);
		case 07: case 014: case 025: case 036: case 041: case 052: case 063: case 070: return Move(5-x(),   y(), (11-r)&7, other);
		case 06: case 017: case 024: case 035: case 042: case 053: case 060: case 071: return Move(5-y(), 5-x(), (13-r)&7, other);
		case 05: case 016: case 027: case 034: case 043: case 050: case 061: case 072: return Move(  x(), 5-y(), (15-r)&7, other);
		default:
			printf("o: %i, other: %i, c: %#4x", o, other, c);
			assert(false && "Bad orientation?!?");
		}
	}

	static void test();

	std::string to_s(bool orient = false) const {
		if(l == M_UNKNOWN) return "unknown";
		if(l == M_NONE)    return "none";
		if(l == M_SWAP)    return "swap";
		if(l == M_RESIGN)  return "resign";

		std::string s = std::string() + char(y() + 'a') + to_str(x() + 1) + char(r + 's');
		if(orient)
			s += ":" + to_str(o);
		return s;
	}

	//TODO: handle orientation?
	bool operator< (const Move & b) const { return (l == b.l ? r <  b.r : l <  b.l); }
	bool operator<=(const Move & b) const { return (l == b.l ? r <= b.r : l <= b.l); }
	bool operator> (const Move & b) const { return (l == b.l ? r >  b.r : l >  b.l); }
	bool operator>=(const Move & b) const { return (l == b.l ? r >= b.r : l >= b.l); }
	bool operator==(const MoveSpecial & b) const { return (l == b); }
	bool operator==(const Move & b) const { return (l == b.l && r == b.r && o == b.o); }
	bool operator!=(const Move & b) const { return (l != b.l || r != b.r || o != b.o); }
	bool operator!=(const MoveSpecial & b) const { return (l != b); }
};


struct PairMove {
	Move a, b;
	PairMove(Move A = M_UNKNOWN, Move B = M_UNKNOWN) : a(A), b(B) { }
	PairMove(MoveSpecial A) : a(Move(A)), b(M_UNKNOWN) { }
};
