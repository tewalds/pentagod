
#pragma once

#include <stdint.h>
#include <cstdlib>
#include "string.h"

enum MoveSpecial {
	M_SWAP    = -1, //-1 so that adding 1 makes it into a valid move
	M_RESIGN  = -2,
	M_NONE    = -3,
	M_UNKNOWN = -4,
};

struct Move {
	int8_t l, r; //location, rotation

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


	Move(MoveSpecial a = M_UNKNOWN) : l(a), r(120) { } //big r so it will always wrap to l=0 with swap
	Move(int L, int R) : l(L), r(R) { }

	Move(const std::string & str){
		if(     str == "swap"    ){ l = M_SWAP;    r = 120; }
		else if(str == "resign"  ){ l = M_RESIGN;  r = 120; }
		else if(str == "none"    ){ l = M_NONE;    r = 120; }
		else if(str == "unknown" ){ l = M_UNKNOWN; r = 120; }
		else if(str.length() != 3){ l = M_NONE;    r = 120; }
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
		}
	}

//abcdefghijklmnopqrstuvwxyz

	int x() const { return l % 6; }
	int y() const { return l / 6; }
	int quadrant()  const { return r >> 1; }
	int direction() const { return r & 1; }

	std::string to_s() const {
		if(l == M_UNKNOWN) return "unknown";
		if(l == M_NONE)    return "none";
		if(l == M_SWAP)    return "swap";
		if(l == M_RESIGN)  return "resign";

		return std::string() + char(y() + 'a') + to_str(x() + 1) + char(r + 's');
	}

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

