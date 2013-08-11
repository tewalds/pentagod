
#include "move.h"
#include "board.h"
#include "string.h"


void check(const std::string a, int ao, const std::string b, int bo) {
	Move am = Move(b, bo).rotate(ao);
	Move bm = Move(a, ao).rotate(bo);
	printf("%s %2i <=> %s %2i ", a.c_str(), ao, b.c_str(), bo);
	printf("  :  %s %2i <=> %s %2i\n", bm.to_s().c_str(), bm.o, am.to_s().c_str(), am.o);
	assert(bm == Move(b, bo));
	assert(am == Move(a, ao));
}

void check(const std::string a, const std::string b) {
	Move ma(a);
	Move mb(b);
	Move ar = ma.rotate(mb.orientation());
	Move br = mb.rotate(ma.orientation());
	if(ma != br || mb != ar){
		printf("%s => %s    %s => %s\n", a.c_str(),  ar.to_s(true).c_str(), b.c_str(), br.to_s(true).c_str());
		printf("%s\n", Board().to_s(true).c_str());
	}
	assert(ma == br);
	assert(mb == ar);
}


void check_many(std::string in, std::initializer_list<std::string> moves){
	assert(moves.size() == 8);
	int r = 0;
	for(std::string out : moves){
		check(in, out + ":" + to_str(r));
		r++;
	}
}

void Move::test() {
//	printf("move tests\n");

	//                     0      1      2      3      4      5      6      7
	check_many("a2z:0", {"a2z", "b6t", "f5v", "e1x", "b1u", "f2s", "e6y", "a5w"});
	check_many("a2z:1", {"e1x", "a2z", "b6t", "f5v", "a5w", "b1u", "f2s", "e6y"});
	check_many("a2z:2", {"f5v", "e1x", "a2z", "b6t", "e6y", "a5w", "b1u", "f2s"});
	check_many("a2z:3", {"b6t", "f5v", "e1x", "a2z", "f2s", "e6y", "a5w", "b1u"});
	check_many("a2z:4", {"b1u", "a5w", "e6y", "f2s", "a2z", "e1x", "f5v" ,"b6t"});
	check_many("a2z:5", {"f2s", "b1u", "a5w", "e6y", "b6t", "a2z", "e1x", "f5v"});
	check_many("a2z:6", {"e6y", "f2s", "b1u", "a5w", "f5v", "b6t", "a2z", "e1x"});
	check_many("a2z:7", {"a5w", "e6y", "f2s", "b1u", "e1x", "f5v", "b6t", "a2z"});
}
