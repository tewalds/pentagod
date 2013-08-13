
#pragma once

#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>
#include <cassert>
using namespace std;

#include "move.h"
#include "xorshift.h"

//#include <bitset>
//#define bitcount(x) std::bitset<typeof(x)>(x).count()
#define bitcount(x) __builtin_popcountll(x)
//#include "bitcount.h"
//#define bitcount(n) parallel_bitcount(n)
//#define bitcount(n) sparse_ones_bitcount(n)
//#define bitcount(n) sparse_ones_bitcount_6bit(n)
//#define bitcount(x) precomputed_bitcount(x)
//#define bitcount(x) precomputed_bitcount2(x)


/*
bits are laid out in this pattern so that:
- rotating a quadrant means pulling out an 8 bit strip and rotating it by 2
- rotating the entire board means rotating the bits by 9
*/

class Board{
	static const int      xytobit[36];      // indexed by xy coordinate, return bit pattern index
	static const uint64_t xybits[36];       // xybits[i] = (1ull << xytobit[i])
	static const uint64_t winmaps[32];      // the bit patterns for the 32 win conditions
	static const uint64_t flipquad[512];    // lookup table to mirror a quadrant
	static const uint16_t * lookup3to2;     // convert base 3 for one line of 6 to base 2, used in hashing
	static const  int16_t scoremap[6];      // how many points a given line with how many pieces is worth

	uint64_t sides[3]; // sides[0] = sides[1] | sides[2]; bitmap of position for each side
	uint8_t nummoves;  // how many moves have been made so far
	uint8_t to_play;   // who's turn is it next, 1|2
	mutable int8_t outcome; //-3 = unknown, 0 = tie, 1,2 = player win
	mutable uint8_t orientation;
	mutable int16_t cached_score;
	mutable uint64_t cached_hash;
	static const int16_t default_score = 0xDEAD;
public:
	static const short unique_depth = 10;  //look for redundant moves up to this depth
	static const short fullhash_depth = 7; //also consider rotations/mirrors of the board

	Board(){
		sides[0] = 0;
		sides[1] = 0;
		sides[2] = 0;
		nummoves = 0;
		to_play = 1;
		outcome = -4;
		orientation = 8;
		cached_score = default_score;
		cached_hash = 0;
	}

	//take a position as 01012200 ... of length 36, left to right, top to bottom, all [012]
	Board(string str);

	static void test();

	int num_moves() const { return nummoves; }
	int moves_remain() const { return (won() >= 0 ? 0 : 36 - nummoves); }
	int moves_avail() const { return moves_remain()*8; } //upper bound


	bool valid_move_fast(const Move & m) const { return !(sides[0] & xybits[m.l]); }
	bool valid_move(const Move & m) const { return valid_move_fast(m) && m.r >= 0 && m.r < 8; }

	uint8_t get(int x, int y) const {
		uint64_t mask = xybits[x + 6*y];
		if(sides[1] & mask) return 1;
		if(sides[2] & mask) return 2;
		return 0;
	}

	string to_s(bool color = true) const ;
	string state() const ;

	void print(bool color = true) const {
		printf("%s", to_s(color).c_str());
	}

	string won_str() const ;

	uint8_t toplay() const {
		return to_play;
	}

	int8_t won() const {
		if(outcome == -4)
			outcome = won_calc();
		return outcome;
	}
	int8_t won_calc() const {
		int8_t wonside = 0;
		uint64_t ws = sides[1];
		uint64_t bs = sides[2];

		for(int i = 0; i < 32; i++){
			uint64_t wm = winmaps[i];
			if     ((ws & wm) == wm) wonside |= 1;
			else if((bs & wm) == wm) wonside |= 2;
		}

		switch(wonside){
			case 1:
			case 2:  return wonside;
			case 3:  return 0; //wonside == 3 when both sides win simultaneously
			default: return (nummoves >= 36 ? 0 : -3);
		}
	}

	int16_t score() const {
		if(cached_score == default_score)
			cached_score = score_calc();
		return cached_score;
	}
	int16_t score_calc() const {
		int16_t s = 0;
		uint64_t ws = sides[1];
		uint64_t bs = sides[2];

		//calculate score from white's perspective
		for(int i = 0; i < 32; i++){
			uint64_t wm = winmaps[i];
			uint64_t w = (ws & wm);
			uint64_t b = (bs & wm);

			if     (w && !b) s += scoremap[bitcount(w)];
			else if(!w && b) s -= scoremap[bitcount(b)];
		}
		//return the score from the perspective of the player that just played
		//ie not the player whose turn it is now
		return (to_play == 1 ? -s : s);
	}

	unsigned int orient() const {
		if(!cached_hash)
			hash();
		return orientation;
	}

	uint64_t hash() const {
		if(!cached_hash)
			cached_hash = (nummoves < fullhash_depth ? full_hash() : simple_hash());
		return cached_hash;
	}

	bool move(Move mo){
		assert(outcome < 0);

		orient();
		Move m = mo.rotate(orientation);

		//TODO: only call valid_move if the move didn't come from an iterator?
		if(!valid_move(m))
			return false;

		if(m == M_SWAP){
			swap(sides[1], sides[2]);
			to_play = 1;
			return true;
		}

		sides[to_play] |= xybits[m.l];

		if (m.direction() == 0) {
			sides[1] = rotate_quad_ccw(sides[1], m.quadrant());
			sides[2] = rotate_quad_ccw(sides[2], m.quadrant());
		} else {
			sides[1] = rotate_quad_cw(sides[1], m.quadrant());
			sides[2] = rotate_quad_cw(sides[2], m.quadrant());
		}
		sides[0] = sides[1] | sides[2];

		nummoves++;
		to_play = 3 - to_play;
		outcome = -4;
		cached_score = default_score;
		cached_hash = 0;
		orientation = 8; //start with an unoriented board


//		if(m != mo)
//			logerr(mo.to_s(true) + " -> " + m.to_s(true) + " -> " + to_str(orient()) + "\n");

		return true;
	}

	bool move_rand(XORShift_uint64 & rand){
		//find an empty space, this should take log2(bits set) to converge, assuming a good and fast RNG
		uint64_t move = (~sides[0]) & 0xFFFFFFFFFULL; // bits set for all empty positions
		uint64_t mask;
		do {
			mask = rand();
			if((move & mask) > 0) // don't let it drop to no bits set
				move &= mask;
		} while(move & (move-1));
//		} while(bitcount(move) > 1); // if there's only one bit left, that's our move

		sides[to_play] |= move;

		uint64_t rotation = (mask >> 36); //mask is already a random number, so just re-use the unused high bits
		uint64_t direction = rotation & 0x4;
		uint64_t quadrant = rotation & 0x3;

		if (direction) {
			sides[1] = rotate_quad_ccw(sides[1], quadrant);
			sides[2] = rotate_quad_ccw(sides[2], quadrant);
		} else {
			sides[1] = rotate_quad_cw(sides[1], quadrant);
			sides[2] = rotate_quad_cw(sides[2], quadrant);
		}
		sides[0] = sides[1] | sides[2];

		nummoves++;
		to_play = 3 - to_play;
		outcome = -4;
		cached_score = default_score;
		cached_hash = 0;
		orientation = 8;

		return true;
	}

	bool undo(const Move & m) {
		if(valid_move(m))
			return false;

		if(m == M_SWAP){
			swap(sides[1], sides[2]);
			to_play = 1;
			return true;
		}

		to_play = 3 - to_play;
		nummoves--;

		if (m.direction() == 0) {
			sides[1] = rotate_quad_cw(sides[1], m.quadrant());
			sides[2] = rotate_quad_cw(sides[2], m.quadrant());
		} else {
			sides[1] = rotate_quad_ccw(sides[1], m.quadrant());
			sides[2] = rotate_quad_ccw(sides[2], m.quadrant());
		}

		sides[to_play] &= ~xybits[m.l];

		sides[0] = sides[1] | sides[2];

		outcome = -4;
		cached_score = default_score;
		cached_hash = 0;
		orientation = 8;

		return true;
	}

private:

	uint64_t simple_hash() const {
		//Take 9 bits at a time from each player, merge them, convert to base 2
		//results in a 60 bit hash when only 48 bits are needed, but much more efficient to compute
		uint64_t h = 0;
		uint64_t w = sides[1] << 9;
		uint64_t b = sides[2];
		h |= ((uint64_t)(lookup3to2[((w & (0x1FFull <<  9)) | (b & (0x1FFull <<  0))) >>  0])) <<  0;
		h |= ((uint64_t)(lookup3to2[((w & (0x1FFull << 18)) | (b & (0x1FFull <<  9))) >>  9])) << 15;
		h |= ((uint64_t)(lookup3to2[((w & (0x1FFull << 27)) | (b & (0x1FFull << 18))) >> 18])) << 30;
		h |= ((uint64_t)(lookup3to2[((w & (0x1FFull << 36)) | (b & (0x1FFull << 27))) >> 27])) << 45;
		orientation = 8;
		return h;
	}

	static inline void choose(uint64_t & m, uint64_t h, uint8_t & o, uint8_t no){
		if(m > h){
			m = h;
			o = no;
		}
	}

	uint64_t full_hash() const {
		//make sure this matches Move::rotate
		Board b(*this);

		uint64_t h, m = ~0;
		uint8_t o = 0;
		choose(m, (h = b.simple_hash()), o, 0);
		choose(m, (h = rotate_hash(h) ), o, 1);
		choose(m, (h = rotate_hash(h) ), o, 2);
		choose(m, (    rotate_hash(h) ), o, 3);
		b.flip_board();
		choose(m, (h = b.simple_hash()), o, 4);
		choose(m, (h = rotate_hash(h) ), o, 5);
		choose(m, (h = rotate_hash(h) ), o, 6);
		choose(m, (    rotate_hash(h) ), o, 7);

		orientation = o;
		return m;
	}

	static uint64_t rotate_hash(uint64_t h){ // rotate ccw
		return ((h & 0xFFFFFFFFFFF8000ull) >> 15) | ((h & 0x7FFFull) << 45);
	}

	void rotate_board(){
		//ignore sides[0], not used by simple_hash
		sides[1] = rotate_side(sides[1]);
		sides[2] = rotate_side(sides[2]);
	}

	void flip_board(){
		//ignore sides[0], not used by simple_hash
		sides[1] = flip_side(sides[1]);
		sides[2] = flip_side(sides[2]);
	}


	static uint64_t rotate_side(uint64_t b){ // rotate one player
		return ((b & 0xFFFFFFE00ull) >> 9) | ((b & 0x1FFull) << 27);
	}

	static uint64_t flip_side(uint64_t b){ // flip one player
		//starting pattern
		// 0  1  2 15 16  9
		// 7  8  3 14 17 10
		// 6  5  4 13 12 11
		//shifted down by 9
		// 0  1  2  6  7  0
		// 7  8  3  5  8  1
		// 6  5  4  4  3  2
		//linearized pattern from:to
		// 0  1  2  3  4  5  6  7  8
		// 0  7  6  5  4  3  2  1  8

		//flip along a diagonal axis
		//make sure this matches Move::rotate
		return (flipquad[(b & (0x1FFull <<  0)) >>  0] <<  0) |
		       (flipquad[(b & (0x1FFull <<  9)) >>  9] << 27) |
		       (flipquad[(b & (0x1FFull << 18)) >> 18] << 18) |
		       (flipquad[(b & (0x1FFull << 27)) >> 27] <<  9);
	}

	static uint64_t rotate_quad_ccw(uint64_t b, int q){
		uint64_t m = 0xFFull << (q*9);
		return (b & ~m) | (((b & m) >> 2) & m) | (((b & m) << 6) & m);
	}
	static uint64_t rotate_quad_cw(uint64_t b, int q){
		uint64_t m = 0xFFull << (q*9);
		return (b & ~m) | (((b & m) >> 6) & m) | (((b & m) << 2) & m);
	}
};
