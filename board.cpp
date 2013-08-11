
#include "board.h"
#include "string.h"

const int Board::xytobit[36] = {
	 0,  1,  2, 15, 16,  9,
	 7,  8,  3, 14, 17, 10,
	 6,  5,  4, 13, 12, 11,
	29, 30, 31, 22, 23, 24,
	28, 35, 32, 21, 26, 25,
	27, 34, 33, 20, 19, 18,
};

#define p2(x) ( 1ull << Board::xytobit[x])
const uint64_t Board::xybits[36] = {
	p2( 0), p2( 1), p2( 2), p2( 3), p2( 4), p2( 5),
	p2( 6), p2( 7), p2( 8), p2( 9), p2(10), p2(11),
	p2(12), p2(13), p2(14), p2(15), p2(16), p2(17),
	p2(18), p2(19), p2(20), p2(21), p2(22), p2(23),
	p2(24), p2(25), p2(26), p2(27), p2(28), p2(29),
	p2(30), p2(31), p2(32), p2(33), p2(34), p2(35),
};
#undef p2

const int16_t Board::scoremap[6] = { 0, 1, 3, 9, 27, 127 };


Board::Board(string str) {
	sides[1] = 0;
	sides[2] = 0;
	nummoves = 0;
	outcome = -4;
	cached_score = default_score;
	cached_hash = 0;

	assert(str.length() == 36);
	int moved[3] = {0,0,0};
	for(int i = 0; i < 36; i++){
		int8_t side = str[i] - '0';
		assert(side >= 0 && side <= 2);

		if(side > 0){
			nummoves++;
			sides[side] |= xybits[i];
			moved[side]++;
		}
	}
	assert(moved[1] == moved[2] || moved[1] == moved[2] + 1); //even number of moves per player
	sides[0] = sides[1] | sides[2];
	to_play = (nummoves % 2) + 1;
}

string Board::state() const {
	string s;
	for(int y = 0; y < 6; y++)
		for(int x = 0; x < 6; x++)
			s += to_str((int)get(x, y));
	return s;
}

string Board::to_s(bool color) const {
	string white = "O",
	       black = "@",
	       empty = ".",
	       coord = "",
	       reset = "";
	if(color){
		string esc = "\033";
		reset = esc + "[0m";
		coord = esc + "[1;37m";
		empty = reset + ".";
		white = esc + "[1;33m" + "@"; //yellow
		black = esc + "[1;34m" + "@"; //blue
	}

	// clockwise around, starting from left
	// 1 2
	//0   3
	//7   4
	// 6 5
	static string arrows[] = {"↙", "↗", "↖", "↘", "↗", "↙", "↘", "↖"}; // ↙↘↗↖
//	static string arrows[] = {"↓", "→", "←", "↓", "↑", "←", "→", "↑"}; // ←↑→↓
//	static string arrows[] = {"⬐", "↱", "↰", "⬎", "⬏", "↲", "↳", "⬑"}; // ↰ ↱ ↲ ↳ ⬐ ⬎ ⬑ ⬏ ↴ ↵
//	static string arrows[] = {"⤹", "↷", "↶", "⤸", "⤿", "⤾", "⤿", "⤾"}; // ↺ ↻ ⥀ ⥁ ⟲ ⟳ ↶ ↷ ⤾ ⤿ ⤸ ⤹ ⤺ ⤻

	static string top[]    = {arrows[1], " t     u ", arrows[2] };
	static string left[]   = {arrows[0], "s", " ", " ", "z", arrows[7] };
	static string right[]  = {arrows[3], "v", " ", " ", "w", arrows[4] };
	static string bottom[] = {arrows[6], " y     x ", arrows[5] };

	string s;
	s += coord;
	s += "    ";
	for(unsigned int i = 0; i < sizeof(top)/sizeof(string); i++)
		s += top[i];
	s += "\n";
	s += "    1 2 3 4 5 6\n";
	for(int y = 0; y < 6; y++){
		s += left[y] + " " + string(1, 'a' + y) + " ";
		for(int x = 0; x < 6; x++){
			int p = get(x, y);
			if(p == 0) s += empty;
			if(p == 1) s += white;
			if(p == 2) s += black;
			s += " ";
		}
		s += coord + right[y] + "\n";
	}
	s += "    ";
	for(unsigned int i = 0; i < sizeof(bottom)/sizeof(string); i++)
		s += bottom[i];
	s += reset + "\n";
	return s;
}

string Board::won_str() const {
	switch(won()){
		case -3: return "none";
		case -2: return "black_or_draw";
		case -1: return "white_or_draw";
		case 0:
		case 3:  return "draw";
		case 1:  return "white";
		case 2:  return "black";
	}
	return "unknown";
}


#define winpattern(a,b,c,d,e) ((1ULL<<(a)) | (1ULL<<(b)) | (1ULL<<(c)) | (1ULL<<(d)) | (1ULL<<(e)))

const uint64_t Board::winmaps[32] = {
//horizontal
	winpattern( 0,  1,  2, 15, 16), winpattern( 1,  2, 15, 16,  9),
	winpattern( 7,  8,  3, 14, 17), winpattern( 8,  3, 14, 17, 10),
	winpattern( 6,  5,  4, 13, 12), winpattern( 5,  4, 13, 12, 11),
	winpattern(29, 30, 31, 22, 23), winpattern(30, 31, 22, 23, 24),
	winpattern(28, 35, 32, 21, 26), winpattern(35, 32, 21, 26, 25),
	winpattern(27, 34, 33, 20, 19), winpattern(24, 33, 20, 19, 18),
//vertical
	winpattern( 0,  7,  6, 29, 28), winpattern( 7,  6, 29, 28, 27),
	winpattern( 1,  8,  5, 30, 35), winpattern( 8,  5, 30, 35, 34),
	winpattern( 2,  3,  4, 31, 32), winpattern( 3,  4, 31, 32, 33),
	winpattern(15, 14, 13, 22, 21), winpattern(14, 13, 22, 21, 20),
	winpattern(16, 17, 12, 23, 26), winpattern(17, 12, 23, 26, 19),
	winpattern( 9, 10, 11, 24, 25), winpattern(10, 11, 24, 25, 18),
//diagonal
	winpattern( 0,  8,  4, 22, 26), winpattern( 8,  4, 22, 26, 18),
	winpattern( 7,  5, 31, 21, 19), winpattern( 1,  3, 13, 23, 25),
	winpattern( 9, 17, 13, 31, 35), winpattern(17, 13, 31, 35, 27),
	winpattern(16, 14,  4, 30, 28), winpattern(10, 12, 22, 32, 34),
};

/*
generated with ruby:
(0...512).each{|i|
	puts "" if (i % 16) == 0
	bits = ""
	(0...9).each{|j|
		bits << (i & 1).to_s
		i /= 2
	}
	bits = bits.reverse
	bits[1...8] = bits[1...8].reverse
	print ('0x' + (bits.to_i(2)).to_s(16).upcase).rjust(5) + ", "
}
puts
*/
const uint64_t Board::flipquad[512] = {
	  0x0,   0x1,  0x80,  0x81,  0x40,  0x41,  0xC0,  0xC1,  0x20,  0x21,  0xA0,  0xA1,  0x60,  0x61,  0xE0,  0xE1,
	 0x10,  0x11,  0x90,  0x91,  0x50,  0x51,  0xD0,  0xD1,  0x30,  0x31,  0xB0,  0xB1,  0x70,  0x71,  0xF0,  0xF1,
	  0x8,   0x9,  0x88,  0x89,  0x48,  0x49,  0xC8,  0xC9,  0x28,  0x29,  0xA8,  0xA9,  0x68,  0x69,  0xE8,  0xE9,
	 0x18,  0x19,  0x98,  0x99,  0x58,  0x59,  0xD8,  0xD9,  0x38,  0x39,  0xB8,  0xB9,  0x78,  0x79,  0xF8,  0xF9,
	  0x4,   0x5,  0x84,  0x85,  0x44,  0x45,  0xC4,  0xC5,  0x24,  0x25,  0xA4,  0xA5,  0x64,  0x65,  0xE4,  0xE5,
	 0x14,  0x15,  0x94,  0x95,  0x54,  0x55,  0xD4,  0xD5,  0x34,  0x35,  0xB4,  0xB5,  0x74,  0x75,  0xF4,  0xF5,
	  0xC,   0xD,  0x8C,  0x8D,  0x4C,  0x4D,  0xCC,  0xCD,  0x2C,  0x2D,  0xAC,  0xAD,  0x6C,  0x6D,  0xEC,  0xED,
	 0x1C,  0x1D,  0x9C,  0x9D,  0x5C,  0x5D,  0xDC,  0xDD,  0x3C,  0x3D,  0xBC,  0xBD,  0x7C,  0x7D,  0xFC,  0xFD,
	  0x2,   0x3,  0x82,  0x83,  0x42,  0x43,  0xC2,  0xC3,  0x22,  0x23,  0xA2,  0xA3,  0x62,  0x63,  0xE2,  0xE3,
	 0x12,  0x13,  0x92,  0x93,  0x52,  0x53,  0xD2,  0xD3,  0x32,  0x33,  0xB2,  0xB3,  0x72,  0x73,  0xF2,  0xF3,
	  0xA,   0xB,  0x8A,  0x8B,  0x4A,  0x4B,  0xCA,  0xCB,  0x2A,  0x2B,  0xAA,  0xAB,  0x6A,  0x6B,  0xEA,  0xEB,
	 0x1A,  0x1B,  0x9A,  0x9B,  0x5A,  0x5B,  0xDA,  0xDB,  0x3A,  0x3B,  0xBA,  0xBB,  0x7A,  0x7B,  0xFA,  0xFB,
	  0x6,   0x7,  0x86,  0x87,  0x46,  0x47,  0xC6,  0xC7,  0x26,  0x27,  0xA6,  0xA7,  0x66,  0x67,  0xE6,  0xE7,
	 0x16,  0x17,  0x96,  0x97,  0x56,  0x57,  0xD6,  0xD7,  0x36,  0x37,  0xB6,  0xB7,  0x76,  0x77,  0xF6,  0xF7,
	  0xE,   0xF,  0x8E,  0x8F,  0x4E,  0x4F,  0xCE,  0xCF,  0x2E,  0x2F,  0xAE,  0xAF,  0x6E,  0x6F,  0xEE,  0xEF,
	 0x1E,  0x1F,  0x9E,  0x9F,  0x5E,  0x5F,  0xDE,  0xDF,  0x3E,  0x3F,  0xBE,  0xBF,  0x7E,  0x7F,  0xFE,  0xFF,
	0x100, 0x101, 0x180, 0x181, 0x140, 0x141, 0x1C0, 0x1C1, 0x120, 0x121, 0x1A0, 0x1A1, 0x160, 0x161, 0x1E0, 0x1E1,
	0x110, 0x111, 0x190, 0x191, 0x150, 0x151, 0x1D0, 0x1D1, 0x130, 0x131, 0x1B0, 0x1B1, 0x170, 0x171, 0x1F0, 0x1F1,
	0x108, 0x109, 0x188, 0x189, 0x148, 0x149, 0x1C8, 0x1C9, 0x128, 0x129, 0x1A8, 0x1A9, 0x168, 0x169, 0x1E8, 0x1E9,
	0x118, 0x119, 0x198, 0x199, 0x158, 0x159, 0x1D8, 0x1D9, 0x138, 0x139, 0x1B8, 0x1B9, 0x178, 0x179, 0x1F8, 0x1F9,
	0x104, 0x105, 0x184, 0x185, 0x144, 0x145, 0x1C4, 0x1C5, 0x124, 0x125, 0x1A4, 0x1A5, 0x164, 0x165, 0x1E4, 0x1E5,
	0x114, 0x115, 0x194, 0x195, 0x154, 0x155, 0x1D4, 0x1D5, 0x134, 0x135, 0x1B4, 0x1B5, 0x174, 0x175, 0x1F4, 0x1F5,
	0x10C, 0x10D, 0x18C, 0x18D, 0x14C, 0x14D, 0x1CC, 0x1CD, 0x12C, 0x12D, 0x1AC, 0x1AD, 0x16C, 0x16D, 0x1EC, 0x1ED,
	0x11C, 0x11D, 0x19C, 0x19D, 0x15C, 0x15D, 0x1DC, 0x1DD, 0x13C, 0x13D, 0x1BC, 0x1BD, 0x17C, 0x17D, 0x1FC, 0x1FD,
	0x102, 0x103, 0x182, 0x183, 0x142, 0x143, 0x1C2, 0x1C3, 0x122, 0x123, 0x1A2, 0x1A3, 0x162, 0x163, 0x1E2, 0x1E3,
	0x112, 0x113, 0x192, 0x193, 0x152, 0x153, 0x1D2, 0x1D3, 0x132, 0x133, 0x1B2, 0x1B3, 0x172, 0x173, 0x1F2, 0x1F3,
	0x10A, 0x10B, 0x18A, 0x18B, 0x14A, 0x14B, 0x1CA, 0x1CB, 0x12A, 0x12B, 0x1AA, 0x1AB, 0x16A, 0x16B, 0x1EA, 0x1EB,
	0x11A, 0x11B, 0x19A, 0x19B, 0x15A, 0x15B, 0x1DA, 0x1DB, 0x13A, 0x13B, 0x1BA, 0x1BB, 0x17A, 0x17B, 0x1FA, 0x1FB,
	0x106, 0x107, 0x186, 0x187, 0x146, 0x147, 0x1C6, 0x1C7, 0x126, 0x127, 0x1A6, 0x1A7, 0x166, 0x167, 0x1E6, 0x1E7,
	0x116, 0x117, 0x196, 0x197, 0x156, 0x157, 0x1D6, 0x1D7, 0x136, 0x137, 0x1B6, 0x1B7, 0x176, 0x177, 0x1F6, 0x1F7,
	0x10E, 0x10F, 0x18E, 0x18F, 0x14E, 0x14F, 0x1CE, 0x1CF, 0x12E, 0x12F, 0x1AE, 0x1AF, 0x16E, 0x16F, 0x1EE, 0x1EF,
	0x11E, 0x11F, 0x19E, 0x19F, 0x15E, 0x15F, 0x1DE, 0x1DF, 0x13E, 0x13F, 0x1BE, 0x1BF, 0x17E, 0x17F, 0x1FE, 0x1FF,
};

//fills a lookup table of size (1<<(numbits*2)) with a mapping from two base 2 numbers to a single base 3 number
//the two base 2 numbers are back to back and of size numbits. The bits from the front and back parts are taken
//as pairs of bits that are in base 3, so bit pairs 00, 01, 10 are valid but 11 is not.
//eg: gen_lookup3to2(2)
//would fill 0000 -> 0, 0001 -> 1, 0100 -> 2, 0010 -> 3, 0011 -> 4, 0110 -> 5, 1000 -> 6, 1001 -> 7, 1100 -> 8,
//and all others (ie 0101, 1010, 1011, 1110, 0111, 1101, 1111), would go to 0 as they're invalid in this scheme
uint16_t * gen_lookup3to2(unsigned int inbits, unsigned int outbits){
	uint16_t * lookup = new uint16_t[1 << (inbits*2)];
	for(uint16_t i = 0; i < (1 << inbits);  i++){ //top bits
		for(uint16_t j = 0; j < (1 << inbits); j++){ //bottom bits
			uint16_t bi = i, bj = j, val = 0, exp = 1;
			while(bi || bj){
				if((bi & 1) && (bj & 1)){ //not base 3 if both are set
					val = 0;
					break;
				}
				val += exp * (2*(bi & 1) + (bj & 1));
				exp *= 3;
				bi >>= 1;
				bj >>= 1;
			}
			assert(val < (1 << outbits));
			lookup[(i << inbits) + j] = val;
//			printf("%5X, ", val);
//			if(j % 16 == 15)
//				printf("\n");
		}
	}
	return lookup;
}
const uint16_t * Board::lookup3to2 = gen_lookup3to2(9, 15);


void check(uint64_t h, uint8_t o, Board &b){
	if(h != b.hash())
		printf("expected hash: %lu, got: %lu\n", h, b.hash());
	if(o != b.orient())
		printf("expected orient: %i, got: %i\n", o, b.orient());
	if(h != b.hash() || o != b.orient())
		printf("%s", b.to_s().c_str());
	assert(h == b.hash());
	assert(o == b.orient());
}
void check(uint64_t h, uint8_t o, string m){
	Board b;
	b.move(m);
	check(h, o, b);
}
void check(uint64_t h, uint8_t o, std::initializer_list<string> moves){
	Board b;
	for(string m : moves){
		b.move(m);
	}
	check(h, o, b);
}

void Board::test() {
//	printf("board tests\n");

	//a single non-rotated piece leads to known board orientations
	check(6, 0, "a2z");
	check(6, 1, "b6t");
	check(6, 2, "f5v");
	check(6, 3, "e1x");
	check(6, 4, "b1u");
	check(6, 5, "f2s");
	check(6, 6, "e6y");
	check(6, 7, "a5w");

	check(2, 0, "a1z");
	check(2, 1, "a6t");
	check(2, 2, "f6v");
	check(2, 3, "f1x");

	//a pair of non-rotated pieces lead to known board orientations
	check(15, 0, {"a2z", "a3z"});
	check(15, 1, {"b6t", "c6t"});
	check(15, 2, {"f5v", "f4v"});
	check(15, 3, {"e1x", "d1x"});
	check(15, 4, {"b1u", "c1u"});
	check(15, 5, {"f2s", "f3s"});
	check(15, 6, {"e6y", "d6y"});
	check(15, 7, {"a5w", "a4w"});

	//a single oriented piece leads to a known board orientation
	check(6, 0, "a2z:0");
	check(6, 3, "a2z:1");
	check(6, 2, "a2z:2");
	check(6, 1, "a2z:3");
	check(6, 4, "a2z:4");
	check(6, 5, "a2z:5");
	check(6, 6, "a2z:6");
	check(6, 7, "a2z:7");

	//a single oriented piece leads to a known board orientation
	check(6, 1, "b6t:0");
	check(6, 0, "b6t:1");
	check(6, 3, "b6t:2");
	check(6, 2, "b6t:3");
	check(6, 5, "b6t:4");
	check(6, 6, "b6t:5");
	check(6, 7, "b6t:6");
	check(6, 4, "b6t:7");

	//a single oriented piece leads to a known board orientation
	check(6, 2, "f5v:0");
	check(6, 1, "f5v:1");
	check(6, 0, "f5v:2");
	check(6, 3, "f5v:3");
	check(6, 6, "f5v:4");
	check(6, 7, "f5v:5");
	check(6, 4, "f5v:6");
	check(6, 5, "f5v:7");

	//a single oriented piece leads to a known board orientation
	check(6, 4, "b1u:0");
	check(6, 5, "b1u:1");
	check(6, 6, "b1u:2");
	check(6, 7, "b1u:3");
	check(6, 0, "b1u:4");
	check(6, 3, "b1u:5");
	check(6, 2, "b1u:6");
	check(6, 1, "b1u:7");

	//a single oriented piece leads to a known board orientation
	check(6, 7, "a5w:0");
	check(6, 4, "a5w:1");
	check(6, 5, "a5w:2");
	check(6, 6, "a5w:3");
	check(6, 3, "a5w:4");
	check(6, 2, "a5w:5");
	check(6, 1, "a5w:6");
	check(6, 0, "a5w:7");
}
