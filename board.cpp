
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

#define p2(x) ( 1ull << (x))
const uint64_t Board::xybits[36] = {
	p2(Board::xytobit[0]), p2(Board::xytobit[1]), p2(Board::xytobit[2]), p2(Board::xytobit[3]),
	p2(Board::xytobit[4]), p2(Board::xytobit[5]), p2(Board::xytobit[6]), p2(Board::xytobit[7]),
	p2(Board::xytobit[8]), p2(Board::xytobit[9]), p2(Board::xytobit[10]),p2(Board::xytobit[11]),
	p2(Board::xytobit[12]),p2(Board::xytobit[13]),p2(Board::xytobit[14]),p2(Board::xytobit[15]),
	p2(Board::xytobit[16]),p2(Board::xytobit[17]),p2(Board::xytobit[18]),p2(Board::xytobit[19]),
	p2(Board::xytobit[20]),p2(Board::xytobit[21]),p2(Board::xytobit[22]),p2(Board::xytobit[23]),
	p2(Board::xytobit[24]),p2(Board::xytobit[25]),p2(Board::xytobit[26]),p2(Board::xytobit[27]),
	p2(Board::xytobit[28]),p2(Board::xytobit[29]),p2(Board::xytobit[30]),p2(Board::xytobit[31]),
	p2(Board::xytobit[32]),p2(Board::xytobit[33]),p2(Board::xytobit[34]),p2(Board::xytobit[35])
};

const int Board::scoremap[6] = { 0, 1, 3, 9, 27, 127 };



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
	switch(outcome){
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

/*
generated with ruby:

def get_bits(i)
	bits = []
	while i > 0
		bits << (i & 1)
		i /= 2
	end
	return (bits + ([0]*10))[0..10]
end
(0...64).each{|i|
	bi = get_bits(i)
	(0...64).each{|j|
		bj = get_bits(j)

		v = 0
		m = 1
		(0...10).each{|k|
			if bi[k] == 1 && bj[k] == 1
				v = 0
				break
			end
			v += m*(2*bi[k] + bj[k])
			m *= 3
		}
		print ('0x' + v.to_s(16).upcase).rjust(5) + ", "
		puts if j % 16 == 15
	}
}
puts
*/
const uint64_t Board::lookup3to2[4096] = {
	  0x0,   0x1,   0x3,   0x4,   0x9,   0xA,   0xC,   0xD,  0x1B,  0x1C,  0x1E,  0x1F,  0x24,  0x25,  0x27,  0x28,
	 0x51,  0x52,  0x54,  0x55,  0x5A,  0x5B,  0x5D,  0x5E,  0x6C,  0x6D,  0x6F,  0x70,  0x75,  0x76,  0x78,  0x79,
	 0xF3,  0xF4,  0xF6,  0xF7,  0xFC,  0xFD,  0xFF, 0x100, 0x10E, 0x10F, 0x111, 0x112, 0x117, 0x118, 0x11A, 0x11B,
	0x144, 0x145, 0x147, 0x148, 0x14D, 0x14E, 0x150, 0x151, 0x15F, 0x160, 0x162, 0x163, 0x168, 0x169, 0x16B, 0x16C,
	  0x2,   0x0,   0x5,   0x0,   0xB,   0x0,   0xE,   0x0,  0x1D,   0x0,  0x20,   0x0,  0x26,   0x0,  0x29,   0x0,
	 0x53,   0x0,  0x56,   0x0,  0x5C,   0x0,  0x5F,   0x0,  0x6E,   0x0,  0x71,   0x0,  0x77,   0x0,  0x7A,   0x0,
	 0xF5,   0x0,  0xF8,   0x0,  0xFE,   0x0, 0x101,   0x0, 0x110,   0x0, 0x113,   0x0, 0x119,   0x0, 0x11C,   0x0,
	0x146,   0x0, 0x149,   0x0, 0x14F,   0x0, 0x152,   0x0, 0x161,   0x0, 0x164,   0x0, 0x16A,   0x0, 0x16D,   0x0,
	  0x6,   0x7,   0x0,   0x0,   0xF,  0x10,   0x0,   0x0,  0x21,  0x22,   0x0,   0x0,  0x2A,  0x2B,   0x0,   0x0,
	 0x57,  0x58,   0x0,   0x0,  0x60,  0x61,   0x0,   0x0,  0x72,  0x73,   0x0,   0x0,  0x7B,  0x7C,   0x0,   0x0,
	 0xF9,  0xFA,   0x0,   0x0, 0x102, 0x103,   0x0,   0x0, 0x114, 0x115,   0x0,   0x0, 0x11D, 0x11E,   0x0,   0x0,
	0x14A, 0x14B,   0x0,   0x0, 0x153, 0x154,   0x0,   0x0, 0x165, 0x166,   0x0,   0x0, 0x16E, 0x16F,   0x0,   0x0,
	  0x8,   0x0,   0x0,   0x0,  0x11,   0x0,   0x0,   0x0,  0x23,   0x0,   0x0,   0x0,  0x2C,   0x0,   0x0,   0x0,
	 0x59,   0x0,   0x0,   0x0,  0x62,   0x0,   0x0,   0x0,  0x74,   0x0,   0x0,   0x0,  0x7D,   0x0,   0x0,   0x0,
	 0xFB,   0x0,   0x0,   0x0, 0x104,   0x0,   0x0,   0x0, 0x116,   0x0,   0x0,   0x0, 0x11F,   0x0,   0x0,   0x0,
	0x14C,   0x0,   0x0,   0x0, 0x155,   0x0,   0x0,   0x0, 0x167,   0x0,   0x0,   0x0, 0x170,   0x0,   0x0,   0x0,
	 0x12,  0x13,  0x15,  0x16,   0x0,   0x0,   0x0,   0x0,  0x2D,  0x2E,  0x30,  0x31,   0x0,   0x0,   0x0,   0x0,
	 0x63,  0x64,  0x66,  0x67,   0x0,   0x0,   0x0,   0x0,  0x7E,  0x7F,  0x81,  0x82,   0x0,   0x0,   0x0,   0x0,
	0x105, 0x106, 0x108, 0x109,   0x0,   0x0,   0x0,   0x0, 0x120, 0x121, 0x123, 0x124,   0x0,   0x0,   0x0,   0x0,
	0x156, 0x157, 0x159, 0x15A,   0x0,   0x0,   0x0,   0x0, 0x171, 0x172, 0x174, 0x175,   0x0,   0x0,   0x0,   0x0,
	 0x14,   0x0,  0x17,   0x0,   0x0,   0x0,   0x0,   0x0,  0x2F,   0x0,  0x32,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x65,   0x0,  0x68,   0x0,   0x0,   0x0,   0x0,   0x0,  0x80,   0x0,  0x83,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x107,   0x0, 0x10A,   0x0,   0x0,   0x0,   0x0,   0x0, 0x122,   0x0, 0x125,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x158,   0x0, 0x15B,   0x0,   0x0,   0x0,   0x0,   0x0, 0x173,   0x0, 0x176,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x18,  0x19,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,  0x33,  0x34,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x69,  0x6A,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,  0x84,  0x85,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x10B, 0x10C,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x126, 0x127,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x15C, 0x15D,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x177, 0x178,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x1A,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,  0x35,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x6B,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,  0x86,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x10D,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x128,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x15E,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x179,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x36,  0x37,  0x39,  0x3A,  0x3F,  0x40,  0x42,  0x43,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x87,  0x88,  0x8A,  0x8B,  0x90,  0x91,  0x93,  0x94,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x129, 0x12A, 0x12C, 0x12D, 0x132, 0x133, 0x135, 0x136,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x17A, 0x17B, 0x17D, 0x17E, 0x183, 0x184, 0x186, 0x187,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x38,   0x0,  0x3B,   0x0,  0x41,   0x0,  0x44,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x89,   0x0,  0x8C,   0x0,  0x92,   0x0,  0x95,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x12B,   0x0, 0x12E,   0x0, 0x134,   0x0, 0x137,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x17C,   0x0, 0x17F,   0x0, 0x185,   0x0, 0x188,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x3C,  0x3D,   0x0,   0x0,  0x45,  0x46,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x8D,  0x8E,   0x0,   0x0,  0x96,  0x97,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x12F, 0x130,   0x0,   0x0, 0x138, 0x139,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x180, 0x181,   0x0,   0x0, 0x189, 0x18A,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x3E,   0x0,   0x0,   0x0,  0x47,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x8F,   0x0,   0x0,   0x0,  0x98,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x131,   0x0,   0x0,   0x0, 0x13A,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x182,   0x0,   0x0,   0x0, 0x18B,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x48,  0x49,  0x4B,  0x4C,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x99,  0x9A,  0x9C,  0x9D,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x13B, 0x13C, 0x13E, 0x13F,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x18C, 0x18D, 0x18F, 0x190,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x4A,   0x0,  0x4D,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x9B,   0x0,  0x9E,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x13D,   0x0, 0x140,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x18E,   0x0, 0x191,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x4E,  0x4F,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x9F,  0xA0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x141, 0x142,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x192, 0x193,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0x50,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xA1,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x143,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x194,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xA2,  0xA3,  0xA5,  0xA6,  0xAB,  0xAC,  0xAE,  0xAF,  0xBD,  0xBE,  0xC0,  0xC1,  0xC6,  0xC7,  0xC9,  0xCA,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x195, 0x196, 0x198, 0x199, 0x19E, 0x19F, 0x1A1, 0x1A2, 0x1B0, 0x1B1, 0x1B3, 0x1B4, 0x1B9, 0x1BA, 0x1BC, 0x1BD,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xA4,   0x0,  0xA7,   0x0,  0xAD,   0x0,  0xB0,   0x0,  0xBF,   0x0,  0xC2,   0x0,  0xC8,   0x0,  0xCB,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x197,   0x0, 0x19A,   0x0, 0x1A0,   0x0, 0x1A3,   0x0, 0x1B2,   0x0, 0x1B5,   0x0, 0x1BB,   0x0, 0x1BE,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xA8,  0xA9,   0x0,   0x0,  0xB1,  0xB2,   0x0,   0x0,  0xC3,  0xC4,   0x0,   0x0,  0xCC,  0xCD,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x19B, 0x19C,   0x0,   0x0, 0x1A4, 0x1A5,   0x0,   0x0, 0x1B6, 0x1B7,   0x0,   0x0, 0x1BF, 0x1C0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xAA,   0x0,   0x0,   0x0,  0xB3,   0x0,   0x0,   0x0,  0xC5,   0x0,   0x0,   0x0,  0xCE,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x19D,   0x0,   0x0,   0x0, 0x1A6,   0x0,   0x0,   0x0, 0x1B8,   0x0,   0x0,   0x0, 0x1C1,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xB4,  0xB5,  0xB7,  0xB8,   0x0,   0x0,   0x0,   0x0,  0xCF,  0xD0,  0xD2,  0xD3,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1A7, 0x1A8, 0x1AA, 0x1AB,   0x0,   0x0,   0x0,   0x0, 0x1C2, 0x1C3, 0x1C5, 0x1C6,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xB6,   0x0,  0xB9,   0x0,   0x0,   0x0,   0x0,   0x0,  0xD1,   0x0,  0xD4,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1A9,   0x0, 0x1AC,   0x0,   0x0,   0x0,   0x0,   0x0, 0x1C4,   0x0, 0x1C7,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xBA,  0xBB,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,  0xD5,  0xD6,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1AD, 0x1AE,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x1C8, 0x1C9,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xBC,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,  0xD7,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1AF,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x1CA,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xD8,  0xD9,  0xDB,  0xDC,  0xE1,  0xE2,  0xE4,  0xE5,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1CB, 0x1CC, 0x1CE, 0x1CF, 0x1D4, 0x1D5, 0x1D7, 0x1D8,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xDA,   0x0,  0xDD,   0x0,  0xE3,   0x0,  0xE6,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1CD,   0x0, 0x1D0,   0x0, 0x1D6,   0x0, 0x1D9,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xDE,  0xDF,   0x0,   0x0,  0xE7,  0xE8,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1D1, 0x1D2,   0x0,   0x0, 0x1DA, 0x1DB,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xE0,   0x0,   0x0,   0x0,  0xE9,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1D3,   0x0,   0x0,   0x0, 0x1DC,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xEA,  0xEB,  0xED,  0xEE,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1DD, 0x1DE, 0x1E0, 0x1E1,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xEC,   0x0,  0xEF,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1DF,   0x0, 0x1E2,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xF0,  0xF1,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1E3, 0x1E4,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	 0xF2,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1E5,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1E6, 0x1E7, 0x1E9, 0x1EA, 0x1EF, 0x1F0, 0x1F2, 0x1F3, 0x201, 0x202, 0x204, 0x205, 0x20A, 0x20B, 0x20D, 0x20E,
	0x237, 0x238, 0x23A, 0x23B, 0x240, 0x241, 0x243, 0x244, 0x252, 0x253, 0x255, 0x256, 0x25B, 0x25C, 0x25E, 0x25F,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1E8,   0x0, 0x1EB,   0x0, 0x1F1,   0x0, 0x1F4,   0x0, 0x203,   0x0, 0x206,   0x0, 0x20C,   0x0, 0x20F,   0x0,
	0x239,   0x0, 0x23C,   0x0, 0x242,   0x0, 0x245,   0x0, 0x254,   0x0, 0x257,   0x0, 0x25D,   0x0, 0x260,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1EC, 0x1ED,   0x0,   0x0, 0x1F5, 0x1F6,   0x0,   0x0, 0x207, 0x208,   0x0,   0x0, 0x210, 0x211,   0x0,   0x0,
	0x23D, 0x23E,   0x0,   0x0, 0x246, 0x247,   0x0,   0x0, 0x258, 0x259,   0x0,   0x0, 0x261, 0x262,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1EE,   0x0,   0x0,   0x0, 0x1F7,   0x0,   0x0,   0x0, 0x209,   0x0,   0x0,   0x0, 0x212,   0x0,   0x0,   0x0,
	0x23F,   0x0,   0x0,   0x0, 0x248,   0x0,   0x0,   0x0, 0x25A,   0x0,   0x0,   0x0, 0x263,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1F8, 0x1F9, 0x1FB, 0x1FC,   0x0,   0x0,   0x0,   0x0, 0x213, 0x214, 0x216, 0x217,   0x0,   0x0,   0x0,   0x0,
	0x249, 0x24A, 0x24C, 0x24D,   0x0,   0x0,   0x0,   0x0, 0x264, 0x265, 0x267, 0x268,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1FA,   0x0, 0x1FD,   0x0,   0x0,   0x0,   0x0,   0x0, 0x215,   0x0, 0x218,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x24B,   0x0, 0x24E,   0x0,   0x0,   0x0,   0x0,   0x0, 0x266,   0x0, 0x269,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x1FE, 0x1FF,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x219, 0x21A,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x24F, 0x250,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x26A, 0x26B,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x200,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x21B,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x251,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x26C,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x21C, 0x21D, 0x21F, 0x220, 0x225, 0x226, 0x228, 0x229,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x26D, 0x26E, 0x270, 0x271, 0x276, 0x277, 0x279, 0x27A,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x21E,   0x0, 0x221,   0x0, 0x227,   0x0, 0x22A,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x26F,   0x0, 0x272,   0x0, 0x278,   0x0, 0x27B,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x222, 0x223,   0x0,   0x0, 0x22B, 0x22C,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x273, 0x274,   0x0,   0x0, 0x27C, 0x27D,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x224,   0x0,   0x0,   0x0, 0x22D,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x275,   0x0,   0x0,   0x0, 0x27E,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x22E, 0x22F, 0x231, 0x232,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x27F, 0x280, 0x282, 0x283,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x230,   0x0, 0x233,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x281,   0x0, 0x284,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x234, 0x235,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x285, 0x286,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x236,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x287,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x288, 0x289, 0x28B, 0x28C, 0x291, 0x292, 0x294, 0x295, 0x2A3, 0x2A4, 0x2A6, 0x2A7, 0x2AC, 0x2AD, 0x2AF, 0x2B0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x28A,   0x0, 0x28D,   0x0, 0x293,   0x0, 0x296,   0x0, 0x2A5,   0x0, 0x2A8,   0x0, 0x2AE,   0x0, 0x2B1,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x28E, 0x28F,   0x0,   0x0, 0x297, 0x298,   0x0,   0x0, 0x2A9, 0x2AA,   0x0,   0x0, 0x2B2, 0x2B3,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x290,   0x0,   0x0,   0x0, 0x299,   0x0,   0x0,   0x0, 0x2AB,   0x0,   0x0,   0x0, 0x2B4,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x29A, 0x29B, 0x29D, 0x29E,   0x0,   0x0,   0x0,   0x0, 0x2B5, 0x2B6, 0x2B8, 0x2B9,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x29C,   0x0, 0x29F,   0x0,   0x0,   0x0,   0x0,   0x0, 0x2B7,   0x0, 0x2BA,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2A0, 0x2A1,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x2BB, 0x2BC,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2A2,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0, 0x2BD,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2BE, 0x2BF, 0x2C1, 0x2C2, 0x2C7, 0x2C8, 0x2CA, 0x2CB,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2C0,   0x0, 0x2C3,   0x0, 0x2C9,   0x0, 0x2CC,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2C4, 0x2C5,   0x0,   0x0, 0x2CD, 0x2CE,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2C6,   0x0,   0x0,   0x0, 0x2CF,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2D0, 0x2D1, 0x2D3, 0x2D4,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2D2,   0x0, 0x2D5,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2D6, 0x2D7,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	0x2D8,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,
};

