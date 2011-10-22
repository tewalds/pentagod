Pentagod
========
A pentago AI that implements several algorithms including [negamax](http://en.wikipedia.org/wiki/Negamax), [Negascout](http://en.wikipedia.org/wiki/Negascout), simple [Monte Carlo](http://en.wikipedia.org/wiki/Monte_Carlo_method) and [UCT](http://senseis.xmp.net/?UCT). As far as I know, the negamax version is the strongest pentago player in existence right now.

Features
--------
* Easy to create new players
* Easy to create new scoring functions
* Already includes several players and scoring functions
* Very fast - evaluates millions of positions per second per cpu
* Very strong player, likely the strongest in the world
* Tournament mode
* HTTP mode integrates very well with the [Pentagoo](http://code.google.com/p/pentagoo/) front end

Requirements
------------
* Linux
* C++ tool chain
* libevent and libevent-dev

Usage
-----
* Check out the code from google code
* Make sure you have libevent installed
* Tournament mode:
	* change pentago.cpp to have the players you want
	* Run "make" to compile the code
	* run ./pentago
* Webserver mode:
	* Run "make" to compile the code
	* grab a copy of [Pentagoo](http://code.google.com/p/pentagoo/)
	* run ./pentagohttpd --help to see the options

TODO
----
* Build an opening book
* Better parallelization
* Use a Bitboard for speed
* Win condition pruning (certain patterns make certain win conditions impossible)
* Implement the GTP protocol
* Negamax
	* implement iterative deepening
	* implement a variable depth time limited bot
	* try different scoring functions
	* Add a transposition table
* UCT
	* save previously explored nodes
	* pre-init board scores with the scoring function
	* rotate and move as separate moves
	* try different exploration rates
* Web API
	* make a CGI and FCGI version for those who don't want a standalone httpd
	* improvements aimed at distributed processing
	* add a command to check the server capabilities

