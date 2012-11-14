
#pragma once

#include <stdint.h>

class HashSet {
	unsigned int size;
	unsigned int mask;
	uint64_t * table;

public:
	HashSet() : size(0), mask(0), table(NULL) { }
	HashSet(unsigned int s){ init(s); }
	~HashSet(){
		if(table)
			delete[] table;
		table = NULL;
	}

	void init(unsigned int s){
		size = roundup(s)*4;
		mask = size-1;
		table = new uint64_t[size];
		for(unsigned int i = 0; i < size; i++)
			table[i] = 0;
	}

	//returns whether it was set successfully, fails if it is already in the set
	bool add(uint64_t h){
		unsigned int i = h & mask;
		while(true){
			uint64_t t = table[i];
			if (t == 0) {
				table[i] = h;
				return true;
			} else if (t == h) {
				return false;
			}
			i = (i+1) & mask;
		}
	}

	//slightly faster than add, but may double-add an entry
	void set(uint64_t h){
		unsigned int i = h & mask;
		while(table[i] != 0)
			i = (i+1) & mask;
		table[i] = h;
	}

	bool exists(uint64_t h){
		for(unsigned int i = h & mask; table[i]; i = (i+1) & mask)
			if(table[i] == h)
				return true;
		return false;
	}

	//round a number up to the nearest power of 2
	static unsigned int roundup(unsigned int v) {
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}
};

