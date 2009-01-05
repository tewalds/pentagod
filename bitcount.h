
#if 0
//taken out due to compile time warnings
#define TWO(c) (0x1u << (c))
#define MASK(c) (((unsigned int)(-1)) / (TWO(TWO(c)) + 1u))
#define COUNT(x,c) ((x) & MASK(c)) + (((x) >> (TWO(c))) & MASK(c))

inline int parallel_bitcount(uint64_t n){
	n = COUNT(n, 0);
	n = COUNT(n, 1);
	n = COUNT(n, 2);
	n = COUNT(n, 3);
	n = COUNT(n, 4);
	n = COUNT(n, 5);
	return n;
}
#endif

//probably the best general purpose bitcount function
int sparse_ones_bitcount(uint64_t n){
	int count = 0;
	while(n){
		++count;
		n &= (n - 1);
	}
	return count;
}

//loop unrolled version of above with a limitation of counting a max of 6 bits, which is all that is needed here.
//the time savings seem to be about 30%+ in many situations
int sparse_ones_bitcount_6bit(uint64_t n){
	int count = (n > 0);
	if((n &= (n - 1))){ ++count;
	if((n &= (n - 1))){ ++count;
	if((n &= (n - 1))){ ++count;
	if((n &= (n - 1))){ ++count;
	if((n &= (n - 1))){ ++count;
	}}}}}

	return count;
}


static const unsigned char BitsSetTable256[] = 
{
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

// Both of these are limited to count the bits in the first 5 bytes, which is all that is needed here
// That limitation is easy to fix should it be needed
int precomputed_bitcount(uint64_t n){
	return 
		BitsSetTable256[(n >> 0)  & 0xff] + 
		BitsSetTable256[(n >> 8)  & 0xff] + 
		BitsSetTable256[(n >> 16) & 0xff] + 
		BitsSetTable256[(n >> 24) & 0xff] +
		BitsSetTable256[(n >> 32) & 0xff];
}
int precomputed_bitcount2(uint64_t n){
	unsigned char * p = (unsigned char *) & n;
	return 
		BitsSetTable256[p[0]] + 
		BitsSetTable256[p[1]] + 
		BitsSetTable256[p[2]] + 
		BitsSetTable256[p[3]] + 
		BitsSetTable256[p[4]];
}




