
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
	int      l : 8;
	unsigned r : 4;
	unsigned o : 4;
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

*/


	Move(MoveSpecial a = M_UNKNOWN, unsigned int O = 0) : l(a), r(14), o(O) { } //big r so it will always wrap to l=0 with swap
	Move(int L, int R, unsigned int O = 0) : l(L), r(R), o(O) { }
	Move(int X, int Y, int R, unsigned int O) : l(Y*6 + X), r(R), o(O) { }

	Move(const std::string & str, unsigned int O = 0){
		if(     str == "swap"    ){ l = M_SWAP;    r = 14; }
		else if(str == "resign"  ){ l = M_RESIGN;  r = 14; }
		else if(str == "none"    ){ l = M_NONE;    r = 14; }
		else if(str == "unknown" ){ l = M_UNKNOWN; r = 14; }
		else if(str.length() != 3){ l = M_NONE;    r = 14; }
		else{
			int y = tolower(str[0]) - 'a'; //[abcdef]
			int x = str[1] - '1';          //[123456]
			l = y*6+x;

			char c = str[2];
			if(     c >= '1' && c <= '8') r = c - '1'; //[12345678]
			else if(c >= 's' && c <= 'z') r = c - 's'; //[stuvwxyz]
			else if(c >= 'S' && c <= 'Z') r = c - 'S'; //[STUVWXYZ]
			else if(c >= 'a' && c <= 'h') r = c - 'a'; //[abcdefgh]
			else if(c >= 'A' && c <= 'H') r = c - 'A'; //[abcdefgh]
			else r = 0; //unknown, but do something

			o = O;
		}
	}

	int x()           const { return l % 6; }
	int y()           const { return l / 6; }
	int loc()         const { return l; }
	int quadrant()    const { return r >> 1; }
	int direction()   const { return r & 1; }
	int orientation() const { return o; }

	Move rotate(int other) const {
		if(l < 0) //special
			return *this;

/*
flip move:  Move(y, x, (9-r)&7). Works because (9-1)&7=8&7=0, (9-1)&7=8&7=0
rotate cw:  Move(5-y,   x, (r+2)&7)
rotate 180: Move(5-x, 5-y, (r+4)&7)
rotate ccw: Move(  y, 5-x, (r+6)&7)

Given the two board rotations, what move rotation is needed?
0,0 => 0, 0,1 => 3, 0,2 => 2, 0,3 => 1
1,0 => 1, 1,1 => 0, 1,2 => 3, 1,3 => 2
2,0 => 2, 2,1 => 1, 2,2 => 0, 2,3 => 3
3,0 => 3, 3,1 => 2, 3,2 => 1, 3,3 => 0

(4+o-other)&3

Given the two board flips, do we need to flip the move or change the direction of rotation?
0,0 => 0, 0,1 => 1
1,0 => 2, 1,1 => 3

(o<<1) | other
*/

		int c = ((o&4)<<1) | (other&4) | ((4 + (o&3) - (other&3))&3);
//		printf(" :  %2i -> %2i = %#4x ", o, other, c);

		switch(c){
		case 0x0: case 0xC: return *this;
		                          //Move(  x(),   y(),       r, other);
		case 0x1: case 0xF: return Move(5-y(),   x(), (r+2)&7, other);
		case 0x2: case 0xE: return Move(5-x(), 5-y(), (r+4)&7, other);
		case 0x3: case 0xD: return Move(  y(), 5-x(), (r+6)&7, other);

		case 0x8: case 0x4: return Move(  y(),   x(), ( 9-r)&7, other);
		case 0x9: case 0x7: return Move(5-x(),   y(), (11-r)&7, other);
		case 0xA: case 0x6: return Move(5-y(), 5-x(), (13-r)&7, other);
		case 0xB: case 0x5: return Move(  x(), 5-y(), (15-r)&7, other);
		default:
			printf("o: %i, other: %i, c: %#4x", o, other, c);
			assert(false && "Bad orientation?!?");
		}
	}

	static void test();

	std::string to_s() const {
		if(l == M_UNKNOWN) return "unknown";
		if(l == M_NONE)    return "none";
		if(l == M_SWAP)    return "swap";
		if(l == M_RESIGN)  return "resign";

		return std::string() + char(y() + 'a') + to_str(x() + 1) + char(r + 's');
	}

	//TODO: handle orientation?
	bool operator< (const Move & b) const { return (l == b.l ? r <  b.r : l <  b.l); }
	bool operator<=(const Move & b) const { return (l == b.l ? r <= b.r : l <= b.l); }
	bool operator> (const Move & b) const { return (l == b.l ? r >  b.r : l >  b.l); }
	bool operator>=(const Move & b) const { return (l == b.l ? r >= b.r : l >= b.l); }
	bool operator==(const MoveSpecial & b) const { return (l == b); }
	bool operator==(const Move & b) const { return (l == b.l && r == b.r); }
	bool operator!=(const Move & b) const { return (l != b.l || r != b.r); }
	bool operator!=(const MoveSpecial & b) const { return (l != b); }
};


struct PairMove {
	Move a, b;
	PairMove(Move A = M_UNKNOWN, Move B = M_UNKNOWN) : a(A), b(B) { }
	PairMove(MoveSpecial A) : a(Move(A)), b(M_UNKNOWN) { }
};
