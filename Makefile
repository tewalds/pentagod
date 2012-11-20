.PHONY: clean fresh run gendeps

LDFLAGS   += -pthread
OBJECTS		= pentagod.o \
	alarm.o \
	fileio.o \
	gtpgeneral.o \
	gtpagent.o \
	string.o \
	board.o \
	agentmcts.o \
	agentmctsthread.o \
	agentpns.o \
#	solverab.o \
#	solverpns.o \
#	solverpns2.o \
#	solverpns_tt.o \

ifdef DEBUG
	CPPFLAGS	+= -g3 -Wall
else
	CPPFLAGS	+= -O3 -funroll-loops -Wall

	OSTYPE := $(shell uname -s)
	ifeq ($(OSTYPE),Darwin)
		CPPFLAGS += -m64
		LDFLAGS += -m64
	else
		CPPFLAGS += -march=native
	endif
endif



all: pentagod

mm: mm.cpp
	g++ -O3 -Wall -o mm mm.cpp

pentagod: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)


clean:
	rm -f *.o castro mm mm-with-freq.dat

fresh: clean all

profile:
	valgrind --tool=callgrind ./pentagod

gendeps:
	ls *.cpp -1 | xargs -L 1 cpp -M -MM

############ everything below is generated by: make gendeps

agentmcts.o: agentmcts.cpp agentmcts.h time.h types.h move.h string.h \
 board.h xorshift.h depthstats.h thread.h compacttree.h log.h agent.h \
 alarm.h
agentmctsthread.o: agentmctsthread.cpp agentmcts.h time.h types.h move.h \
 string.h board.h xorshift.h depthstats.h thread.h compacttree.h log.h \
 agent.h moveiterator.h hashset.h
agentpns.o: agentpns.cpp agentpns.h agent.h types.h board.h move.h \
 string.h xorshift.h compacttree.h thread.h log.h moveiterator.h \
 hashset.h time.h alarm.h
alarm.o: alarm.cpp alarm.h time.h
board.o: board.cpp board.h move.h string.h xorshift.h
fileio.o: fileio.cpp fileio.h
gtpagent.o: gtpagent.cpp pentagogtp.h gtp.h string.h game.h board.h \
 move.h xorshift.h agent.h types.h agentmcts.h time.h depthstats.h \
 thread.h compacttree.h log.h agentpns.h fileio.h
gtpgeneral.o: gtpgeneral.cpp pentagogtp.h gtp.h string.h game.h board.h \
 move.h xorshift.h agent.h types.h agentmcts.h time.h depthstats.h \
 thread.h compacttree.h log.h agentpns.h moveiterator.h hashset.h
pentagod.o: pentagod.cpp pentagogtp.h gtp.h string.h game.h board.h \
 move.h xorshift.h agent.h types.h agentmcts.h time.h depthstats.h \
 thread.h compacttree.h log.h agentpns.h
string.o: string.cpp string.h types.h

