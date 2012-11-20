
#pragma once

#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>
#include <cassert>
using namespace std;

#include "move.h"
#include "string.h"
#include "hashset.h"
#include "xorshift.h"
#include "log.h"

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
	static const uint64_t lookup3to2[4096]; // convert base 3 for one line of 6 to base 2, used in hashing
	static const int      scoremap[6];      // how many points a given line with how many pieces is worth

public:
	class MoveIterator { //only returns valid moves...
		const Board & board;
		Move move;
		bool unique;
		HashSet hashes;
	public:
		MoveIterator(const Board & b, bool Unique) : board(b), move(Move(M_SWAP)), unique(Unique) {
			if(board.outcome >= 0){
				move = Move(36, 8); //already done
			} else {
				if(unique)
					hashes.init(board.moves_avail());
				++(*this); //find the first valid move
			}
		}

		const Move & operator * ()  const { return move; }
		const Move * operator -> () const { return & move; }
		bool done() const { return (move.l >= 36); }
		bool operator == (const Board::MoveIterator & rhs) const { return (move == rhs.move); }
		bool operator != (const Board::MoveIterator & rhs) const { return (move != rhs.move); }
		MoveIterator & operator ++ (){ //prefix form
			while(true){
				move.r++;
				if(move.r >= 8){
					move.r = 0;
					do{
						move.l++;
						if(move.l >= 36) //done
							return *this;
					}while(!board.valid_move_fast(move));
				}
				if(unique){
					uint64_t h = board.test_hash(move);
					if(!hashes.add(h))
						continue;
				}
				break;
			}
			return *this;
		}
		MoveIterator operator ++ (int){ //postfix form, discouraged from being used
			MoveIterator newit(*this);
			++(*this);
			return newit;
		}
	};

private:
	uint64_t sides[3]; // sides[0] = sides[1] | sides[2];
	uint8_t nummoves;
	uint8_t to_play;
	mutable int8_t outcome; //-3 = unknown, 0 = tie, 1,2 = player win
	mutable int cached_score;
	static const int default_score = 0xDEADBEEF;

	static const short unique_depth = 5; //update and test rotations/symmetry with less than this many pieces on the board

public:
	Board(){
		sides[0] = 0;
		sides[1] = 0;
		sides[2] = 0;
		nummoves = 0;
		to_play = 1;
		outcome = -4;
		cached_score = default_score;
	}

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

	int score() const {
		if(cached_score == default_score)
			cached_score = score_calc();
		return cached_score;
	}
	int score_calc() const {
		int s = 0;
		uint64_t ws = sides[1];
		uint64_t bs = sides[2];

		for(int i = 0; i < 32; i++){
			uint64_t wm = winmaps[i];
			uint64_t w = (ws & wm);
			uint64_t b = (bs & wm);

			if     (w && !b) s += scoremap[bitcount(w)];
			else if(!w && b) s -= scoremap[bitcount(b)];
		}
		return (to_play == 1 ? s : -s);
	}


	uint64_t hash() const {
		return (nummoves < unique_depth ? full_hash() : simple_hash());
	}

	uint64_t test_hash(const Move & m) const {
		Board b = *this;
		b.move(m);
		return b.hash();
	}

	int test_win(const Move & m) const {
		Board b = *this;
		b.move(m);
		return b.won_calc();
	}

	MoveIterator moveit(bool unique = false) const {
		return MoveIterator(*this, (unique ? nummoves <= unique_depth : false));
	}

	bool move(const Move & m){
		assert(outcome < 0);

		if(!valid_move(m))
			return false;

		if(m == M_SWAP){
			swap(sides[1], sides[2]);
			to_play = 1;
			return true;
		}

		sides[to_play] |=xybits[m.l];

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

		return true;
	}

	bool undo(const Move & m) {
		if(valid_move(m))
			return false;

		if(m == M_SWAP){
			swap(sides[1], sides[2]);
			to_play = 0;
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

		return true;
	}

private:

	uint64_t simple_hash() const {
		//Take 6 bits at a time from each player, merge them, convert to base 2
		//results in a 60 bit hash when only 48 bits are needed, but much more efficient to compute
		uint64_t h = 0;
		uint64_t w = sides[1] << 6;
		uint64_t b = sides[2];
		h |= lookup3to2[((w & (0x3Full <<  6)) | (b & (0x3Full      )))      ];
		h |= lookup3to2[((w & (0x3Full << 12)) | (b & (0x3Full <<  6))) >>  6] << 10;
		h |= lookup3to2[((w & (0x3Full << 18)) | (b & (0x3Full << 12))) >> 12] << 20;
		h |= lookup3to2[((w & (0x3Full << 24)) | (b & (0x3Full << 18))) >> 18] << 30;
		h |= lookup3to2[((w & (0x3Full << 30)) | (b & (0x3Full << 24))) >> 24] << 40;
		h |= lookup3to2[((w & (0x3Full << 36)) | (b & (0x3Full << 30))) >> 30] << 50;
		return h;
	}

	uint64_t full_hash() const {
		uint64_t h = ~0ull;
		Board b(*this);
	/*
		int i = 0;
		while(true){
			uint64_t t = b.simple_hash();
			if(h > t)
				h = t;
			if     (i == 3) b.flip_board();
			else if(i != 7) b.rotate_board();
			else            break;
			i++;
		}
/*/
		h =        b.simple_hash();  b.rotate_board();
		h = min(h, b.simple_hash()); b.rotate_board();
		h = min(h, b.simple_hash()); b.rotate_board();
		h = min(h, b.simple_hash()); b.flip_board();
		h = min(h, b.simple_hash()); b.rotate_board();
		h = min(h, b.simple_hash()); b.rotate_board();
		h = min(h, b.simple_hash()); b.rotate_board();
		h = min(h, b.simple_hash());
//*/
		return h;
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

		//flip along the vertical axix
//		return (flipquad[(b & (0x1FFull <<  0)) >>  0] <<  9) |
//		       (flipquad[(b & (0x1FFull <<  9)) >>  9] <<  0) |
//		       (flipquad[(b & (0x1FFull << 18)) >> 18] << 27) |
//		       (flipquad[(b & (0x1FFull << 27)) >> 27] << 18);
		//flip along a diagonal axis
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

