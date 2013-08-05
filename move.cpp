
#include "move.h"


void check(const std::string a, int ao, const std::string b, int bo) {
	printf("%s %2i <=> %s %2i ", a.c_str(), ao, b.c_str(), bo);
	Move am = Move(b, bo).rotate(ao);
	Move bm = Move(a, ao).rotate(bo);
	printf("  :  %s %2i <=> %s %2i\n", am.to_s().c_str(), am.o, bm.to_s().c_str(), bm.o);
	assert(bm == Move(b, bo));
	assert(am == Move(a, ao));
}

void Move::test() {
	printf("move tests\n");
	check("a2z", 0, "a2z", 0);
	check("a2z", 1, "b6t", 0);
	check("a2z", 2, "f5v", 0);
	check("a2z", 3, "e1x", 0);
	check("a2z", 4, "b1u", 0);
	check("a2z", 5, "a5w", 0);
	check("a2z", 6, "e6y", 0);
	check("a2z", 7, "f2s", 0);

	check("a2z", 0, "a2z", 0);
	check("a2z", 0, "b6t", 3);
	check("a2z", 0, "f5v", 2);
	check("a2z", 0, "e1x", 1);
	check("a2z", 0, "b1u", 4);
	check("a2z", 0, "a5w", 5);
	check("a2z", 0, "e6y", 6);
	check("a2z", 0, "f2s", 7);


	check("b1s", 0, "b1s", 0);
	check("b1s", 0, "f2y", 1);
	check("b1s", 0, "e6w", 2);
	check("b1s", 0, "a5u", 3);
	check("b1s", 0, "a2t", 4);
	check("b1s", 0, "b6v", 5);
	check("b1s", 0, "f5x", 6);
	check("b1s", 0, "e1z", 7);

	check("b1s", 1, "a5u", 0);
	check("b1s", 1, "b1s", 1);
	check("b1s", 1, "f2y", 2);
	check("b1s", 1, "e6w", 3);
	check("b1s", 1, "e1z", 4);
	check("b1s", 1, "a2t", 5);
	check("b1s", 1, "b6v", 6);
	check("b1s", 1, "f5x", 7);

	check("b1s", 2, "e6w", 0);
	check("b1s", 2, "a5u", 1);
	check("b1s", 2, "b1s", 2);
	check("b1s", 2, "f2y", 3);
	check("b1s", 2, "f5x", 4);
	check("b1s", 2, "e1z", 5);
	check("b1s", 2, "a2t", 6);
	check("b1s", 2, "b6v", 7);

	check("b1s", 4, "b1s", 4);
	check("b1s", 4, "a5u", 5);
	check("b1s", 4, "e6w", 6);
	check("b1s", 4, "f2y", 7);
	check("b1s", 4, "a2t", 0);
	check("b1s", 4, "e1z", 1);
	check("b1s", 4, "f5x", 2);
	check("b1s", 4, "b6v", 3);

	check("b1s", 5, "f2y", 4);
	check("b1s", 5, "b1s", 5);
	check("b1s", 5, "a5u", 6);
	check("b1s", 5, "e6w", 7);
	check("b1s", 5, "b6v", 0);
	check("b1s", 5, "a2t", 1);
	check("b1s", 5, "e1z", 2);
	check("b1s", 5, "f5x", 3);
}
