
#include "agentmcts.h"
#include <cmath>
#include <string>
#include "string.h"

void AgentMCTS::MCTSThread::iterate(){
	if(player->profile){
		timestamps[0] = Time();
		stage = 0;
	}

	movelist.reset(&(player->rootboard));
	player->root.exp.addvloss();
	Board copy = player->rootboard;
	walk_tree(copy, & player->root, 0);
	player->root.exp.addv(movelist.getexp(3-player->rootboard.toplay()));

	if(player->profile){
		times[0] += timestamps[1] - timestamps[0];
		times[1] += timestamps[2] - timestamps[1];
		times[2] += timestamps[3] - timestamps[2];
		times[3] += Time() - timestamps[3];
	}
}

void AgentMCTS::MCTSThread::walk_tree(Board & board, Node * node, int depth){
	int toplay = board.toplay();

	if(!node->children.empty() && node->outcome < 0){
	//choose a child and recurse
		Node * child;
		do{
			child = choose_move(node, toplay);

			if(child->outcome < 0){
				movelist.addtree(child->move, toplay);

				if(!board.move(child->move)){
					logerr("move failed: " + child->move.to_s() + "\n" + board.to_s(false));
					assert(false && "move failed");
				}

				child->exp.addvloss(); //balanced out after rollouts

				walk_tree(board, child, depth+1);

				child->exp.addv(movelist.getexp(toplay));
				player->do_backup(node, child, toplay);
				return;
			}
		}while(!player->do_backup(node, child, toplay));

		return;
	}

	if(player->profile && stage == 0){
		stage = 1;
		timestamps[1] = Time();
	}

	int won = (player->minimax ? node->outcome : board.won());

	//if it's not already decided
	if(won < 0){
		//create children if valid
		if(node->exp.num() >= player->visitexpand+1 && create_children(board, node, toplay)){
			walk_tree(board, node, depth);
			return;
		}

		if(player->profile){
			stage = 2;
			timestamps[2] = Time();
		}

		//do random game on this node
		for(int i = 0; i < player->rollouts; i++){
			Board copy = board;
			rollout(copy, node->move, depth);
		}
	}else{
		movelist.finishrollout(won); //got to a terminal state, it's worth recording
	}

	treelen.add(depth);

	movelist.subvlosses(1);

	if(player->profile){
		timestamps[3] = Time();
		if(stage == 1)
			timestamps[2] = timestamps[3];
		stage = 3;
	}

	return;
}

bool sort_node_know(const AgentMCTS::Node & a, const AgentMCTS::Node & b){
	return (a.know > b.know);
}

bool AgentMCTS::MCTSThread::create_children(Board & board, Node * node, int toplay){
	if(!node->children.lock())
		return false;

	CompactTree<Node>::Children temp;
	temp.alloc(board.moves_avail(), player->ctmem);

	Node * child = temp.begin(),
	     * end   = temp.end();
	Board::MoveIterator move = board.moveit(player->prunesymmetry);
	int nummoves = 0;
	for(; !move.done() && child != end; ++move, ++child){
		*child = Node(*move);

		if(player->minimax){
			child->outcome = board.test_win(*move);

			if(child->outcome == toplay){ //proven win from here, don't need children
				node->outcome = child->outcome;
				node->proofdepth = 1;
				node->bestmove = *move;
				node->children.unlock();
				temp.dealloc(player->ctmem);
				return true;
			}
		}

		add_knowledge(board, node, child);
		nummoves++;
	}

	if(player->prunesymmetry)
		temp.shrink(nummoves); //shrink the node to ignore the extra moves
	else{ //both end conditions should happen in parallel
		assert(move.done() && child == end);
	}

	//sort in decreasing order by knowledge
//	sort(temp.begin(), temp.end(), sort_node_know);

	PLUS(player->nodes, temp.num());
	node->children.swap(temp);
	assert(temp.unlock());

	return true;
}

AgentMCTS::Node * AgentMCTS::MCTSThread::choose_move(const Node * node, int toplay) const {
	float val, maxval = -1000000000;
	float logvisits = log(node->exp.num());

	float explore = player->explore;
	if(player->parentexplore)
		explore *= node->exp.avg();

	Node * ret = NULL,
		 * child = node->children.begin(),
		 * end   = node->children.end();

	for(; child != end; child++){
		if(child->outcome >= 0){
			if(child->outcome == toplay) //return a win immediately
				return child;

			val = (child->outcome == 0 ? -1 : -2); //-1 for tie so any unknown is better, -2 for loss so it's even worse
		}else{
			val = child->value(player->knowledge, player->fpurgency);
			if(explore > 0)
				val += explore*sqrt(logvisits/(child->exp.num() + 1));
		}

		if(maxval < val){
			maxval = val;
			ret = child;
		}
	}

	return ret;
}

/*
backup in this order:

6 win
5 win/draw
4 draw if draw/loss
3 win/draw/loss
2 draw
1 draw/loss
0 lose
return true if fully solved, false if it's unknown or partially unknown
*/
bool AgentMCTS::do_backup(Node * node, Node * backup, int toplay){
	int nodeoutcome = node->outcome;
	if(nodeoutcome >= 0) //already proven, probably by a different thread
		return true;

	if(backup->outcome == -3) //nothing proven by this child, so no chance
		return false;


	uint8_t proofdepth = backup->proofdepth;
	if(backup->outcome != toplay){
		uint64_t sims = 0, bestsims = 0, outcome = 0, bestoutcome = 0;
		backup = NULL;

		Node * child = node->children.begin(),
			 * end = node->children.end();

		for( ; child != end; child++){
			int childoutcome = child->outcome; //save a copy to avoid race conditions

			if(proofdepth < child->proofdepth+1)
				proofdepth = child->proofdepth+1;

			//these should be sorted in likelyness of matching, most likely first
			if(childoutcome == -3){ // win/draw/loss
				outcome = 3;
			}else if(childoutcome == toplay){ //win
				backup = child;
				outcome = 6;
				proofdepth = child->proofdepth+1;
				break;
			}else if(childoutcome == 3-toplay){ //loss
				outcome = 0;
			}else if(childoutcome == 0){ //draw
				if(nodeoutcome == toplay-3) //draw/loss
					outcome = 4;
				else
					outcome = 2;
			}else if(childoutcome == -toplay){ //win/draw
				outcome = 5;
			}else if(childoutcome == toplay-3){ //draw/loss
				outcome = 1;
			}else{
				logerr("childoutcome == " + to_str(childoutcome) + "\n");
				assert(false && "How'd I get here? All outcomes should be tested above");
			}

			sims = child->exp.num();
			if(bestoutcome < outcome){ //better outcome is always preferable
				bestoutcome = outcome;
				bestsims = sims;
				backup = child;
			}else if(bestoutcome == outcome && ((outcome == 0 && bestsims < sims) || bestsims > sims)){
				//find long losses or easy wins/draws
				bestsims = sims;
				backup = child;
			}
		}

		if(bestoutcome == 3) //no win, but found an unknown
			return false;
	}

	if(CAS(node->outcome, nodeoutcome, backup->outcome)){
		node->bestmove = backup->move;
		node->proofdepth = proofdepth;
	}else //if it was in a race, try again, might promote a partial solve to full solve
		return do_backup(node, backup, toplay);

	return (node->outcome >= 0);
}

void AgentMCTS::MCTSThread::add_knowledge(Board & board, Node * node, Node * child){
	child->know = board.score_calc();
}

///////////////////////////////////////////


//play a random game starting from a board state, and return the results of who won
int AgentMCTS::MCTSThread::rollout(Board & board, Move move, int depth){
	int won;
	while((won = board.won()) < 0) {
		board.move_rand(rand64);
	}
	gamelen.add(board.num_moves());
	movelist.finishrollout(won);
	return won;
}
/*
//play a random game starting from a board state, and return the results of who won
int AgentMCTS::MCTSThread::rollout(Board & board, Move move, int depth){
	int won;

	Move forced = M_UNKNOWN;
	while((won = board.won()) < 0){
		int turn = board.toplay();

		if(forced == M_UNKNOWN){
			//do a complex choice
			PairMove pair = rollout_choose_move(board, move);
			move = pair.a;
			forced = pair.b;
		}else{
			move = forced;
			forced = M_UNKNOWN;
		}

		if(move == M_UNKNOWN)
			move = board.move_rand(rand64);
		else
			board.move(move);
		movelist.addrollout(move, turn);
		board.won_calc();
		depth++;
	}

	gamelen.add(depth);

	movelist.finishrollout(won);
	return won;
}

PairMove AgentMCTS::MCTSThread::rollout_choose_move(Board & board, Move move, int depth){
	//look for possible win

}
*/
