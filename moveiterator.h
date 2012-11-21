
#pragma once

#include "board.h"
#include "move.h"
#include "hashset.h"

class MoveIterator { //only returns valid moves...
	const Board & base_board; //base board
	Board after; // board after making the move
	Move move;
	bool unique;
	HashSet hashes;
public:
	MoveIterator(const Board & b, int Unique = -1) : base_board(b), move(Move(M_SWAP)) {
		unique = (Unique == -1 ? base_board.num_moves() <= Board::unique_depth : Unique);

		if(base_board.won() >= 0){
			move = Move(36, 8); //already done
		} else {
			if(unique)
				hashes.init(base_board.moves_avail());
			++(*this); //find the first valid move
		}
	}

	const Board & board() const { return after; }
	const Move & operator * ()  const { return move; }
	const Move * operator -> () const { return & move; }
	bool done() const { return (move.l >= 36); }
	bool operator == (const MoveIterator & rhs) const { return (move == rhs.move); }
	bool operator != (const MoveIterator & rhs) const { return (move != rhs.move); }
	MoveIterator & operator ++ (){ //prefix form
		while(true){
			move.r++;
			if(move.r >= 8){
				move.r = 0;
				do{
					move.l++;
					if(move.l >= 36) //done
						return *this;
				}while(!base_board.valid_move_fast(move));
			}
			after = base_board;
			after.move(move);
			if(unique){
				uint64_t h = after.hash();
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

