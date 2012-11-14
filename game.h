
#pragma once

#include "board.h"
#include "move.h"
#include "string.h"

class Game {
	vector<Move> hist;

public:

	Game(){
	}

	const vector<Move> & get_hist() const {
		return hist;
	}

	Move get_last() const {
		if(hist.size() == 0)
			return M_NONE;

		return hist[hist.size()-1];
	}

	Board getboard(int offset = 0) const {
		Board board;
		if(offset <= 0)
			offset += hist.size();
		for(int i = 0; i < offset; i++)
			board.move(hist[i]);
		return board;
	}

	int len() const {
		return hist.size();
	}

	void clear(){
		hist.clear();
	}

	bool undo(){
		if(hist.size() <= 0)
			return false;

		hist.pop_back();
		return true;
	}

	int moves_remain() const {
		return getboard().moves_remain();
	}

	int toplay() const {
		return getboard().toplay();
	}

	bool valid(const Move & m) const {
		return getboard().valid_move(m);
	}

	bool move(const Move & m){
		if(getboard().move(m)){
			hist.push_back(m);
			return true;
		}
		return false;
	}
};

