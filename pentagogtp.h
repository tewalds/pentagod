
#pragma once

#include "gtp.h"
#include "game.h"
#include "string.h"
#include "board.h"
#include "move.h"
#include "agent.h"
#include "agentmcts.h"
#include "agentpns.h"
#include "agentab.h"

struct TimeControl {
	enum Method { PERCENT, EVEN, STATS };
	Method method; //method to use to distribute the remaining time
	double param;  //param for the method, such as the percentage used or multiple of even
	double game;
	double move;
	bool   flexible; //whether time_per_move can be saved for future moves
	int    max_sims;

	TimeControl(){
		method   = STATS;
		param    = 2;
		game     = 0;
		move     = 5;
		flexible = true;
		max_sims = 0;
	}

	string method_name(){
		switch(method){
			case PERCENT: return "percent";
			case EVEN:    return "even";
			case STATS:   return "stats";
			default:      return "WTF? unknown time control method";
		}
	}
};

class PentagoGTP : public GTPclient {
	Game game;

public:
	int verbose;
	bool colorboard;

	TimeControl time;
	double      time_remain; //time remaining for this game
	int mem_allowed;

	Agent * agent;

	PentagoGTP(FILE * i = stdin, FILE * o = stdout, FILE * l = NULL){
		GTPclient(i, o, l);

		verbose = 1;
		colorboard = true;

		time_remain = time.game;
		mem_allowed = 1000;

		agent = new AgentMCTS();

		set_board();

		newcallback("name",            bind(&PentagoGTP::gtp_name,          this, _1), "Name of the program");
		newcallback("version",         bind(&PentagoGTP::gtp_version,       this, _1), "Version of the program");
		newcallback("verbose",         bind(&PentagoGTP::gtp_verbose,       this, _1), "Set verbosity, 0 for quiet, 1 for normal, 2+ for more output");
		newcallback("echo",            bind(&PentagoGTP::gtp_echo,          this, _1), "Return the arguments as the response");
		newcallback("colorboard",      bind(&PentagoGTP::gtp_colorboard,    this, _1), "Turn on or off the colored board");
		newcallback("showboard",       bind(&PentagoGTP::gtp_print,         this, _1), "Show the board");
		newcallback("print",           bind(&PentagoGTP::gtp_print,         this, _1), "Alias for showboard");
		newcallback("hash",            bind(&PentagoGTP::gtp_hash,          this, _1), "Output the hash for the current position");
		newcallback("clear_board",     bind(&PentagoGTP::gtp_clearboard,    this, _1), "Clear the board, but keep the size");
		newcallback("clear",           bind(&PentagoGTP::gtp_clearboard,    this, _1), "Alias for clear_board");
		newcallback("boardsize",       bind(&PentagoGTP::gtp_clearboard,    this, _1), "Alias for clear_board, board is fixed size");
		newcallback("play",            bind(&PentagoGTP::gtp_play,          this, _1), "Place a stone: play <color> <location>");
		newcallback("white",           bind(&PentagoGTP::gtp_playwhite,     this, _1), "Place a white stone: white <location>");
		newcallback("black",           bind(&PentagoGTP::gtp_playblack,     this, _1), "Place a black stone: black <location>");
		newcallback("undo",            bind(&PentagoGTP::gtp_undo,          this, _1), "Undo one or more moves: undo [amount to undo]");
		newcallback("time",            bind(&PentagoGTP::gtp_time,          this, _1), "Set the time limits and the algorithm for per game time");
		newcallback("genmove",         bind(&PentagoGTP::gtp_genmove,       this, _1), "Generate a move: genmove [color] [time]");
		newcallback("solve",           bind(&PentagoGTP::gtp_solve,         this, _1), "Try to solve this position");

		newcallback("mcts",            bind(&PentagoGTP::gtp_mcts,          this, _1), "Switch to use the Monte Carlo Tree Search agent to play/solve");
		newcallback("pns",             bind(&PentagoGTP::gtp_pns,           this, _1), "Switch to use the Proof Number Search agent to play/solve");
		newcallback("ab",              bind(&PentagoGTP::gtp_ab,            this, _1), "Switch to use the Alpha/Beta agent to play/solve");

		newcallback("all_legal",       bind(&PentagoGTP::gtp_all_legal,     this, _1), "List all legal moves");
		newcallback("history",         bind(&PentagoGTP::gtp_history,       this, _1), "List of played moves");
		newcallback("playgame",        bind(&PentagoGTP::gtp_playgame,      this, _1), "Play a list of moves");
		newcallback("winner",          bind(&PentagoGTP::gtp_winner,        this, _1), "Check the winner of the game");

		newcallback("pv",              bind(&PentagoGTP::gtp_pv,            this, _1), "Output the principle variation for the player tree as it stands now");
		newcallback("move_stats",      bind(&PentagoGTP::gtp_move_stats,    this, _1), "Output the move stats for the player tree as it stands now");
/*
		newcallback("player_params",   bind(&PentagoGTP::gtp_player_params, this, _1), "Set the algorithm for the player, no args gives options");
		newcallback("player_solve",    bind(&PentagoGTP::gtp_player_solve,  this, _1), "Run the player, but don't make the move, and give solve output");
		newcallback("player_solved",   bind(&PentagoGTP::gtp_player_solved, this, _1), "Output whether the player solved the current node");
		newcallback("player_hgf",      bind(&PentagoGTP::gtp_player_hgf,    this, _1), "Output an hgf of the current tree");
		newcallback("player_load_hgf", bind(&PentagoGTP::gtp_player_load_hgf,this, _1), "Load an hgf generated by player_hgf");
		newcallback("player_confirm",  bind(&PentagoGTP::gtp_confirm_proof, this, _1), "Confirm the outcome of the current tree, for use after loading a proof tree");

		newcallback("ab_solve",        bind(&PentagoGTP::gtp_solve_ab,        this, _1),  "Solve with alpha-beta");
		newcallback("ab_params",       bind(&PentagoGTP::gtp_solve_ab_params, this, _1),  "Set Parameters for alpha-beta");
		newcallback("ab_stats",        bind(&PentagoGTP::gtp_solve_ab_stats,  this, _1),  "Output the stats for the alpha-beta solver");
		newcallback("ab_clear",        bind(&PentagoGTP::gtp_solve_ab_clear,  this, _1),  "Stop the solver and release the memory");

		newcallback("pns_solve",       bind(&PentagoGTP::gtp_solve_pns,        this, _1),  "Solve with proof number search and an explicit tree");
		newcallback("pns_params",      bind(&PentagoGTP::gtp_solve_pns_params, this, _1),  "Set Parameters for PNS");
		newcallback("pns_stats",       bind(&PentagoGTP::gtp_solve_pns_stats,  this, _1),  "Output the stats for the PNS solver");
		newcallback("pns_clear",       bind(&PentagoGTP::gtp_solve_pns_clear,  this, _1),  "Stop the solver and release the memory");

		newcallback("pns2_solve",      bind(&PentagoGTP::gtp_solve_pns2,        this, _1),  "Solve with proof number search and an explicit tree");
		newcallback("pns2_params",     bind(&PentagoGTP::gtp_solve_pns2_params, this, _1),  "Set Parameters for PNS");
		newcallback("pns2_stats",      bind(&PentagoGTP::gtp_solve_pns2_stats,  this, _1),  "Output the stats for the PNS solver");
		newcallback("pns2_clear",      bind(&PentagoGTP::gtp_solve_pns2_clear,  this, _1),  "Stop the solver and release the memory");

		newcallback("pnstt_solve",     bind(&PentagoGTP::gtp_solve_pnstt,        this, _1),  "Solve with proof number search and a transposition table of fixed size");
		newcallback("pnstt_params",    bind(&PentagoGTP::gtp_solve_pnstt_params, this, _1),  "Set Parameters for PNSTT");
		newcallback("pnstt_stats",     bind(&PentagoGTP::gtp_solve_pnstt_stats,  this, _1),  "Outupt the stats for the PNSTT solver");
		newcallback("pnstt_clear",     bind(&PentagoGTP::gtp_solve_pnstt_clear,  this, _1),  "Stop the solver and release the memory");
*/
	}

	void set_board(bool clear = true){
		agent->set_board(game.getboard());
	}

	void move(const Move & m){
		game.move(m);
		agent->move(m);
	}

	GTPResponse gtp_echo(vecstr args);
	GTPResponse gtp_print(vecstr args);
	GTPResponse gtp_hash(vecstr args);
	string won_str(int outcome) const;
	GTPResponse gtp_swap(vecstr args);
	GTPResponse gtp_boardsize(vecstr args);
	GTPResponse gtp_clearboard(vecstr args);
	GTPResponse gtp_undo(vecstr args);
	GTPResponse gtp_all_legal(vecstr args);
	GTPResponse gtp_history(vecstr args);
	GTPResponse play(const string & pos, int toplay);
	GTPResponse gtp_playgame(vecstr args);
	GTPResponse gtp_play(vecstr args);
	GTPResponse gtp_playwhite(vecstr args);
	GTPResponse gtp_playblack(vecstr args);
	GTPResponse gtp_winner(vecstr args);
	GTPResponse gtp_name(vecstr args);
	GTPResponse gtp_version(vecstr args);
	GTPResponse gtp_verbose(vecstr args);
	GTPResponse gtp_colorboard(vecstr args);
	GTPResponse gtp_mcts(vecstr args);
	GTPResponse gtp_pns(vecstr args);
	GTPResponse gtp_ab(vecstr args);

	GTPResponse gtp_time(vecstr args);
	double get_time();
	void time_used(double used);
	GTPResponse gtp_move_stats(vecstr args);
	GTPResponse gtp_pv(vecstr args);
	GTPResponse gtp_genmove(vecstr args);
	GTPResponse gtp_solve(vecstr args);
	GTPResponse gtp_player_params(vecstr args);
};

