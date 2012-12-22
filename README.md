Pentagod
========
A pentago AI that implements several algorithms including MCTS (Monte-Carlo Tree Search), PNS (Proof Number Search) and AB (Alpha-Beta). Its primary interface is GTP (Go Text Protocol), but a python webserver that interfaces pentagoo with the gtp protocol is included.

Features
--------
* Simple GTP framework
* Very fast bitboard implementation
* Easy to create new players
* Players Includes:
	* MCTS
	* PNS
	* AB
* Very strong player, likely the strongest in the world
* Web server integrates very well with the [Pentagoo](http://code.google.com/p/pentagoo/) front end (a modified version is included in this project)

Requirements
------------
* Linux (probably works on any unix)
* C++ tool chain
* Python for the webserver

Usage
-----
* Check out the code from github
* Run "make" to compile the code
* To use the GTP interface:
	* run ./pentagod
* To use the webserver:
	* run ./web.py
	* point your browswer at http://localhost:8080/


TODO
----
* Better scoring function
	* Based on patterns
	* Meaningful range such as win percentage
* Win condition pruning (certain patterns make certain win conditions impossible)
* Negamax
	* Randomization when two moves are very similar
	* Better time cutoffs
	* Use the previous iterations for move ordering, might need better scoring function first
	* Improved transposition table
* MCTS
	* Better rollouts: patterns, forced moves
	* rotate and move as separate moves
	* Prove wins/losses to a greater depth
* PNS
	* Use scoring function
	* Use a small depth of AB?
* Opening book
* Web/Pentagoo
	* Choose opponent AI and time cutoff
	* Better move format
	* Undo when playing against computers

