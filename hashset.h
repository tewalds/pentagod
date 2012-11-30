
#pragma once

#include <stdint.h>

class HashSet {
	unsigned int size;  // how many slots there are, must be a power of 2
	unsigned int mask;  // a mask for the size, ie size-1
	unsigned int count; // how many of the slots are filled in
	uint64_t * table;   // pointer to the actual memory of slots

public:
	HashSet() : size(0), mask(0), count(0), table(NULL) { }
	HashSet(unsigned int s){ init(s); }
	~HashSet(){
		if(table)
			delete[] table;
		table = NULL;
	}

	void init(unsigned int s){
		size = roundup(s)*4;
		mask = size-1;
		count = 0;
		table = new uint64_t[size];
		for(unsigned int i = 0; i < size; i++)
			table[i] = 0;
	}

	//returns whether it was set successfully, fails if it is already in the set
	bool add(uint64_t h){
		h = mix_bits(h);
		unsigned int i = h & mask;
		while(true){
			uint64_t t = table[i];
			if (t == 0) {
				table[i] = h;
				count++;
				return true;
			} else if (t == h) {
				return false;
			}
			i = (i+1) & mask;
		}
	}

	//slightly faster than add, but may double-add an entry
	void set(uint64_t h){
		h = mix_bits(h);
		unsigned int i = h & mask;
		while(table[i] != 0)
			i = (i+1) & mask;
		table[i] = h;
		count++;
	}

	bool exists(uint64_t h){
		h = mix_bits(h);
		for(unsigned int i = h & mask; table[i]; i = (i+1) & mask)
			if(table[i] == h)
				return true;
		return false;
	}

	//give a more even distribution over the bitspace
	static uint64_t mix_bits(uint64_t h){
		h ^= (h >> 17);
		h ^= (h << 31);
		h ^= (h >>  8);
		return h;
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

