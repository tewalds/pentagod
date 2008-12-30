
CC			= g++
CFLAGS		= -Wall -fno-strict-aliasing

GAME        = pentago
GAME_L	    = -lpthread -lrt

GENETIC     = pentagogenetic
GENETIC_L   = -lpthread -lrt

DAEMON		= pentagohttpd
DAEMON_L	= -lpthread -lrt -levent

DATE		= `date +%Y-%m-%d-%H-%M`

#debug with gdb
ifdef DEBUG
	CFLAGS		+= -DUSE_DEBUG -g3 
else
	CFLAGS		+= -O3 -funroll-loops
endif

#profile with callgrind, works well with DEBUG mode
ifdef PROFILE
	CFLAGS		+= -pg
endif

#For profile directed optimizations. To use:
# 1. compile with PROFILE_GEN
# 2. run the program, generates .gcno and .gcda
# 3. compile with PROFILE_USE
ifdef PROFILE_GEN
	CFLAGS		+= -fprofile-generate
endif
ifdef PROFILE_USE
	CFLAGS		+= -fprofile-use
endif

all : $(GAME) $(DAEMON) $(GENETIC)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(GAME): $(GAME_O) $(GAME).cpp
	$(CC) $(LDFLAGS) $(CFLAGS) $(GAME_L) $(GAME_O) $(GAME).cpp -o $(GAME)

$(GENETIC): $(GENETIC_O) $(GENETIC).cpp
	$(CC) $(LDFLAGS) $(CFLAGS) $(GENETIC_L) $(GENETIC_O) $(GENETIC).cpp -o $(GENETIC)

$(DAEMON): $(DAEMON_O) $(DAEMON).cpp
	$(CC) $(LDFLAGS) $(CFLAGS) $(DAEMON_L) $(DAEMON_O) $(DAEMON).cpp -o $(DAEMON)


clean:
	rm -f *.o $(GAME) $(DAEMON) $(GENETIC)
#	rm -f *~

fresh: clean all

run:
	./$(GAME)

profile:
	valgrind --tool=callgrind ./$(GAME)

