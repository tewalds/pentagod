

#include "pentagogtp.h"
#include "fileio.h"
#include <fstream>

using namespace std;

GTPResponse PentagoGTP::gtp_time(vecstr args){
	if(args.size() == 0)
		return GTPResponse(true, string("\n") +
			"Update the time settings, eg: time -s 2.5 -m 10 -g 600 -f 1\n" +
			"Method for distributing remaining time, current: " + time.method_name() + " " + to_str(time.param) + "\n" +
			"  -p --percent  Percentage of the remaining time every move            [10.0]\n" +
			"  -e --even     Multiple of even split of the maximum  remaining moves [2.0]\n" +
			"  -s --stats    Multiple of even split of the expected remaining moves [2.0]\n" +
			"Time allocation\n" +
			"  -m --move     Time per move                                          [" + to_str(time.move) + "]\n" +
			"  -g --game     Time per game                                          [" + to_str(time.game) + "]\n" +
			"  -f --flexible Add remaining time per move to remaining time          [" + to_str(time.flexible) + "]\n" +
			"  -i --maxsims  Maximum number of simulations per move                 [" + to_str(time.max_sims) + "]\n" +
			"Current game\n" +
			"  -r --remain   Remaining time for this game                           [" + to_str(time_remain) + "]\n");

	for(unsigned int i = 0; i < args.size(); i++) {
		string arg = args[i];

		if(arg == "-p" || arg == "--percent"){
			time.method = TimeControl::PERCENT;
			time.param = 10;
			if(i+1 < args.size() && from_str<double>(args[i+1]) > 0) time.param = from_str<double>(args[++i]);
		}else if(arg == "-e" || arg == "--even"){
			time.method = TimeControl::EVEN;
			time.param = 2;
			if(i+1 < args.size() && from_str<double>(args[i+1]) > 0) time.param = from_str<double>(args[++i]);
		}else if(arg == "-s" || arg == "--stats"){
			time.method = TimeControl::STATS;
			time.param = 2;
			if(i+1 < args.size() && from_str<double>(args[i+1]) > 0) time.param = from_str<double>(args[++i]);
		}else if((arg == "-m" || arg == "--move") && i+1 < args.size()){
			time.move = from_str<double>(args[++i]);
		}else if((arg == "-g" || arg == "--game") && i+1 < args.size()){
			time.game = from_str<float>(args[++i]);
		}else if((arg == "-f" || arg == "--flexible") && i+1 < args.size()){
			time.flexible = from_str<bool>(args[++i]);
		}else if((arg == "-i" || arg == "--maxsims") && i+1 < args.size()){
			time.max_sims = from_str<int>(args[++i]);
		}else if((arg == "-r" || arg == "--remain") && i+1 < args.size()){
			time_remain = from_str<double>(args[++i]);
		}else{
			return GTPResponse(false, "Missing or unknown parameter");
		}
	}

	return GTPResponse(true);
}

double PentagoGTP::get_time(){
	double ret = 0;

	switch(time.method){
		case TimeControl::PERCENT:
			ret += time.param*time_remain/100;
			break;
		case TimeControl::STATS:{
			double gamelen = agent->gamelen();
			if(gamelen > 0){
				ret += 2.0*time.param*time_remain / gamelen;
				break;
			}
		}//fall back to even
		case TimeControl::EVEN:
			ret += 2.0*time.param*time_remain / game.moves_remain();
			break;
	}

	if(ret > time_remain)
		ret = time_remain;

	ret += time.move;

	return ret;
}

void PentagoGTP::time_used(double used){
	if(time.flexible)
		time_remain += time.move - used;
	else
		time_remain += min(0.0, time.move - used);
	if(time_remain < 0)
			time_remain = 0;
}

GTPResponse PentagoGTP::gtp_move_stats(vecstr args){
	return GTPResponse(true, agent->move_stats());
}

GTPResponse PentagoGTP::gtp_solve(vecstr args){
	if(game.getboard().won() >= 0)
		return GTPResponse(true, "resign");

	double use_time = (args.size() >= 2 ? from_str<double>(args[1]) : get_time());

	if(verbose)
		logerr("time remain: " + to_str(time_remain, 1) + ", time: " + to_str(use_time, 3) + ", sims: " + to_str(time.max_sims) + "\n");

	Time start;
	agent->search(use_time, time.max_sims, verbose);
	time_used(Time() - start);


	//TODO: find the outcome, not the best move...

	Move best = agent->return_move(verbose);

	if(verbose >= 2)
		logerr(game.getboard().to_s(colorboard) + "\n");

	return GTPResponse(true, best.to_s());
}


GTPResponse PentagoGTP::gtp_genmove(vecstr args){
	if(game.getboard().won() >= 0)
		return GTPResponse(true, "resign");

	double use_time = (args.size() >= 2 ? from_str<double>(args[1]) : get_time());

	if(verbose)
		logerr("time:        remain: " + to_str(time_remain, 1) + ", use: " + to_str(use_time, 3) + ", sims: " + to_str(time.max_sims) + "\n");

	Time start;
	agent->search(use_time, time.max_sims, verbose);
	time_used(Time() - start);


	Move best = agent->return_move(verbose);

	move(best);

	if(verbose >= 2)
		logerr(game.getboard().to_s(colorboard) + "\n");

	return GTPResponse(true, best.to_s());
}


GTPResponse PentagoGTP::gtp_pv(vecstr args){
	string pvstr = "";
	vector<Move> pv = agent->get_pv();
	for(unsigned int i = 0; i < pv.size(); i++)
		pvstr += pv[i].to_s() + " ";
	return GTPResponse(true, pvstr);
}

GTPResponse PentagoGTP::gtp_params(vecstr args){
	if(dynamic_cast<AgentAB   *>(agent)) return gtp_ab_params(args);
	if(dynamic_cast<AgentMCTS *>(agent)) return gtp_mcts_params(args);
	if(dynamic_cast<AgentPNS  *>(agent)) return gtp_pns_params(args);

	return GTPResponse(false, "Unknown Agent type");
}

GTPResponse PentagoGTP::gtp_ab_params(vecstr args){
	AgentAB * ab =  dynamic_cast<AgentAB *>(agent);

	if(args.size() == 0)
		return GTPResponse(true, string("\n") +
			"Set player parameters, eg: params -r 4\n" +
			"  -M --maxmem      Max memory in Mb to use for the tree              [" + to_str(ab->memlimit/(1024*1024)) + "]\n" +
			"  -r --randomness  How many bits of randomness to add to the eval    [" + to_str(ab->randomness) + "]\n"
			);

	string errs;
	for(unsigned int i = 0; i < args.size(); i++) {
		string arg = args[i];

		if((arg == "-M" || arg == "--maxmem") && i+1 < args.size()){
			ab->set_memlimit(from_str<uint64_t>(args[++i])*1024*1024);
		}else if((arg == "-r" || arg == "--randomness") && i+1 < args.size()){
			ab->randomness = from_str<int>(args[++i]);
		}else{
			return GTPResponse(false, "Missing or unknown parameter");
		}
	}
	return GTPResponse(true, errs);
}

GTPResponse PentagoGTP::gtp_mcts_params(vecstr args){
	AgentMCTS * mcts =  dynamic_cast<AgentMCTS *>(agent);

	if(args.size() == 0)
		return GTPResponse(true, string("\n") +
			"Set player parameters, eg: player_params -e 1 -f 0 -t 2 -o 1 -p 0\n" +
			"Processing:\n" +
#ifndef SINGLE_THREAD
			"  -t --threads     Number of MCTS threads                            [" + to_str(mcts->numthreads) + "]\n" +
#endif
			"  -o --ponder      Continue to ponder during the opponents time      [" + to_str(mcts->ponder) + "]\n" +
			"  -M --maxmem      Max memory in Mb to use for the tree              [" + to_str(mcts->maxmem/(1024*1024)) + "]\n" +
			"     --profile     Output the time used by each phase of MCTS        [" + to_str(mcts->profile) + "]\n" +
			"Tree traversal:\n" +
			"  -e --explore     Exploration rate for UCT                          [" + to_str(mcts->explore) + "]\n" +
			"  -A --parexplore  Multiply the explore rate by parents experience   [" + to_str(mcts->parentexplore) + "]\n" +
			"  -a --knowledge   Use knowledge: 0.01*know/sqrt(visits+1)           [" + to_str(mcts->knowledge) + "]\n" +
			"  -X --useexplore  Use exploration with this probability [0-1]       [" + to_str(mcts->useexplore) + "]\n" +
			"  -u --fpurgency   Value to assign to an unplayed move               [" + to_str(mcts->fpurgency) + "]\n" +
			"  -O --rollouts    Number of rollouts to run per simulation          [" + to_str(mcts->rollouts) + "]\n" +
			"Tree building:\n" +
			"  -k --keeptree    Keep the tree from the previous move              [" + to_str(mcts->keeptree) + "]\n" +
			"  -m --minimax     Backup the minimax proof in the UCT tree          [" + to_str(mcts->minimax) + "]\n" +
			"  -x --visitexpand Number of visits before expanding a node          [" + to_str(mcts->visitexpand) + "]\n" +
			"  -P --symmetry    Prune symmetric moves, good for proof, not play   [" + to_str(mcts->prunesymmetry) + "]\n" +
			"Rollout policy:\n" +
			"  -w --instantwin  Look for instant wins (1) and forced replies (2)  [" + to_str(mcts->instantwin) + "]\n"
			);

	string errs;
	for(unsigned int i = 0; i < args.size(); i++) {
		string arg = args[i];

		if((arg == "-t" || arg == "--threads") && i+1 < args.size()){
			mcts->numthreads = from_str<int>(args[++i]);
			bool p = mcts->ponder;
			mcts->set_ponder(false); //stop the threads while resetting them
			mcts->reset_threads();
			mcts->set_ponder(p);
		}else if((arg == "-o" || arg == "--ponder") && i+1 < args.size()){
			mcts->set_ponder(from_str<bool>(args[++i]));
		}else if((arg == "--profile") && i+1 < args.size()){
			mcts->profile = from_str<bool>(args[++i]);
		}else if((arg == "-M" || arg == "--maxmem") && i+1 < args.size()){
			mcts->maxmem = from_str<uint64_t>(args[++i])*1024*1024;
		}else if((arg == "-e" || arg == "--explore") && i+1 < args.size()){
			mcts->explore = from_str<float>(args[++i]);
		}else if((arg == "-A" || arg == "--parexplore") && i+1 < args.size()){
			mcts->parentexplore = from_str<bool>(args[++i]);
		}else if((arg == "-a" || arg == "--knowledge") && i+1 < args.size()){
			mcts->knowledge = from_str<bool>(args[++i]);
		}else if((arg == "-k" || arg == "--keeptree") && i+1 < args.size()){
			mcts->keeptree = from_str<bool>(args[++i]);
		}else if((arg == "-m" || arg == "--minimax") && i+1 < args.size()){
			mcts->minimax = from_str<int>(args[++i]);
		}else if((arg == "-P" || arg == "--symmetry") && i+1 < args.size()){
			mcts->prunesymmetry = from_str<bool>(args[++i]);
		}else if((arg == "-X" || arg == "--useexplore") && i+1 < args.size()){
			mcts->useexplore = from_str<float>(args[++i]);
		}else if((arg == "-u" || arg == "--fpurgency") && i+1 < args.size()){
			mcts->fpurgency = from_str<float>(args[++i]);
		}else if((arg == "-O" || arg == "--rollouts") && i+1 < args.size()){
			mcts->rollouts = from_str<int>(args[++i]);
		}else if((arg == "-x" || arg == "--visitexpand") && i+1 < args.size()){
			mcts->visitexpand = from_str<uint>(args[++i]);
		}else if((arg == "-w" || arg == "--instantwin") && i+1 < args.size()){
			mcts->instantwin = from_str<int>(args[++i]);
		}else{
			return GTPResponse(false, "Missing or unknown parameter");
		}
	}
	return GTPResponse(true, errs);
}

GTPResponse PentagoGTP::gtp_pns_params(vecstr args){
	AgentPNS * pns =  dynamic_cast<AgentPNS *>(agent);

	if(args.size() == 0)
		return GTPResponse(true, string("\n") +
			"Update the pns solver settings, eg: pns_params -m 100 -s 0 -d 1 -e 0.25 -a 2 -l 0\n"
			"  -m --memory   Memory limit in Mb                                       [" + to_str(pns->memlimit/(1024*1024)) + "]\n"
			"  -t --threads  How many threads to run                                  [" + to_str(pns->numthreads) + "]\n"
			"  -s --ties     Which side to assign ties to, 0 = handle, 1 = p1, 2 = p2 [" + to_str(pns->ties) + "]\n"
			"  -d --df       Use depth-first thresholds                               [" + to_str(pns->df) + "]\n"
			"  -e --epsilon  How big should the threshold be                          [" + to_str(pns->epsilon) + "]\n"
//			"  -a --abdepth  Run an alpha-beta search of this size at each leaf       [" + to_str(pns->ab) + "]\n"
			);

	string errs;
	for(unsigned int i = 0; i < args.size(); i++) {
		string arg = args[i];

		if((arg == "-t" || arg == "--threads") && i+1 < args.size()){
			pns->numthreads = from_str<int>(args[++i]);
			pns->reset_threads();
		}else if((arg == "-m" || arg == "--memory") && i+1 < args.size()){
			uint64_t mem = from_str<uint64_t>(args[++i]);
			if(mem < 1) return GTPResponse(false, "Memory can't be less than 1mb");
			pns->set_memlimit(mem*1024*1024);
		}else if((arg == "-s" || arg == "--ties") && i+1 < args.size()){
			pns->ties = from_str<int>(args[++i]);
			pns->clear_mem();
		}else if((arg == "-d" || arg == "--df") && i+1 < args.size()){
			pns->df = from_str<bool>(args[++i]);
		}else if((arg == "-e" || arg == "--epsilon") && i+1 < args.size()){
			pns->epsilon = from_str<float>(args[++i]);
//		}else if((arg == "-a" || arg == "--abdepth") && i+1 < args.size()){
//			pns->ab = from_str<int>(args[++i]);
		}else{
			return GTPResponse(false, "Missing or unknown parameter");
		}
	}

	return GTPResponse(true, errs);
}
