
#include "pentagogtp.h"
#include "moveiterator.h"

GTPResponse PentagoGTP::gtp_mcts(vecstr args){
	delete agent;
	agent = new AgentMCTS();
	agent->set_board(game.getboard());
	return GTPResponse(true);
}

GTPResponse PentagoGTP::gtp_pns(vecstr args){
	delete agent;
	agent = new AgentPNS();
	agent->set_board(game.getboard());
	return GTPResponse(true);
}
GTPResponse PentagoGTP::gtp_ab(vecstr args){
	delete agent;
	agent = new AgentAB();
	agent->set_board(game.getboard());
	return GTPResponse(true);
}

GTPResponse PentagoGTP::gtp_echo(vecstr args){
	return GTPResponse(true, implode(args, " "));
}

GTPResponse PentagoGTP::gtp_print(vecstr args){
	Board board = game.getboard();
	for(unsigned int i = 0; i < args.size() && board.move(args[i]); i++)
		;
	return GTPResponse(true, "\n" + board.to_s(colorboard));
}

string PentagoGTP::won_str(int outcome) const {
	switch(outcome){
		case -3: return "none";
		case -2: return "black_or_draw";
		case -1: return "white_or_draw";
		case 0:  return "draw";
		case 1:  return "white";
		case 2:  return "black";
		case 3:  return "draw by simultaneous win";
		default: return "unknown";
	}
}


GTPResponse PentagoGTP::gtp_clearboard(vecstr args){
	game.clear();
	set_board();

	time_remain = time.game;

	log("clear_board");
	return GTPResponse(true);
}

GTPResponse PentagoGTP::gtp_undo(vecstr args){
	int num = 1;
	if(args.size() >= 1)
		num = from_str<int>(args[0]);

	while(num--){
		game.undo();
		log("undo");
	}
	set_board(false);
	if(verbose >= 2)
		logerr(game.getboard().to_s(colorboard) + "\n");
	return GTPResponse(true);
}

GTPResponse PentagoGTP::gtp_all_legal(vecstr args){
	string ret;
	for(MoveIterator move(game.getboard()); !move.done(); ++move)
		ret += move->to_s() + " ";
	return GTPResponse(true, ret);
}

GTPResponse PentagoGTP::gtp_history(vecstr args){
	string ret;
	vector<Move> hist = game.get_hist();
	for(unsigned int i = 0; i < hist.size(); i++)
		ret += hist[i].to_s() + " ";
	return GTPResponse(true, ret);
}

GTPResponse PentagoGTP::play(const string & pos, int toplay){
	if(toplay != game.toplay())
		return GTPResponse(false, "It is the other player's turn!");

	if(game.getboard().won() >= 0)
		return GTPResponse(false, "The game is already over.");

	Move m(pos);

	if(!game.valid(m))
		return GTPResponse(false, "Invalid move");

	move(m);

	log(string("play ") + (toplay == 1 ? 'w' : 'b') + ' ' + m.to_s());

	if(verbose >= 2)
		logerr("Placement: " + m.to_s() + ", outcome: " + game.getboard().won_str() + "\n" + game.getboard().to_s(colorboard));

	return GTPResponse(true);
}

GTPResponse PentagoGTP::gtp_playgame(vecstr args){
	GTPResponse ret(true);

	for(unsigned int i = 0; ret.success && i < args.size(); i++)
		ret = play(args[i], game.toplay());

	return ret;
}

GTPResponse PentagoGTP::gtp_play(vecstr args){
	if(args.size() != 2)
		return GTPResponse(false, "Wrong number of arguments");

	char toplay = 0;
	switch(tolower(args[0][0])){
		case 'w': toplay = 1; break;
		case 'b': toplay = 2; break;
		default:
			return GTPResponse(false, "Invalid player selection");
	}

	return play(args[1], toplay);
}

GTPResponse PentagoGTP::gtp_playwhite(vecstr args){
	if(args.size() != 1)
		return GTPResponse(false, "Wrong number of arguments");

	return play(args[0], 1);
}

GTPResponse PentagoGTP::gtp_playblack(vecstr args){
	if(args.size() != 1)
		return GTPResponse(false, "Wrong number of arguments");

	return play(args[0], 2);
}

GTPResponse PentagoGTP::gtp_winner(vecstr args){
	return GTPResponse(true, game.getboard().won_str());
}

GTPResponse PentagoGTP::gtp_name(vecstr args){
	return GTPResponse(true, "Pentagod");
}

GTPResponse PentagoGTP::gtp_version(vecstr args){
	return GTPResponse(true, "1.5");
}

GTPResponse PentagoGTP::gtp_verbose(vecstr args){
	if(args.size() >= 1)
		verbose = from_str<int>(args[0]);
	else
		verbose = !verbose;
	return GTPResponse(true, "Verbose " + to_str(verbose));
}

GTPResponse PentagoGTP::gtp_colorboard(vecstr args){
	if(args.size() >= 1)
		colorboard = from_str<int>(args[0]);
	else
		colorboard = !colorboard;
	return GTPResponse(true, "Color " + to_str(colorboard));
}

GTPResponse PentagoGTP::gtp_hash(vecstr args){
	return GTPResponse(true, to_str(game.getboard().hash()));
}

