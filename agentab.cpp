
#include "agentab.h"
#include "time.h"
#include "alarm.h"
#include "log.h"

void AgentAB::search(double time, uint64_t maxiters, int verbose) {
	reset();
	if(rootboard.won() >= 0)
		return;

	if(TT == NULL)
		TT = new Node[maxnodes];

	Alarm timer;
	if (time > 0)
		timer(time/15, std::tr1::bind(&AgentAB::timedout, this));

	Time start;

	uint64_t nodes_start, seen, prev_nodes_seen = 0;
	for(unsigned int depth = 2; !timeout && depth < 36 && (maxiters == 0 || depth <= maxiters); depth++){
		maxdepth = depth;
		nodes_start = nodes_seen;
		Time start_depth;
		if (verbose)
			logerr("Depth " + to_str(depth) + "      ");
		negamax(rootboard, SCORE_LOSS, SCORE_WIN, depth);
		seen = nodes_seen - nodes_start;
		if (verbose) {
			logerr("time: " + to_str((Time() - start_depth)*1000, 0) + " msec, ");
			logerr("BF: " + to_str(prev_nodes_seen ? (double)(1.0*seen / prev_nodes_seen) : 0.0, 2) + ", Nodes: " + to_str(seen) + "\n");
		}
		prev_nodes_seen = seen;
	}

	time_used += Time() - start;

	if(verbose){
		logerr("Finished:    " + to_str(nodes_seen) + " nodes in " + to_str(time_used*1000, 0) + " msec: " + to_str((uint64_t)((double)nodes_seen/time_used)) + " Nodes/s\n");

		vector<Move> pv = get_pv();
		string pvstr;
		for(vector<Move>::iterator m = pv.begin(); m != pv.end(); ++m)
			pvstr += " " + m->to_s();
		logerr("PV:         " + pvstr + "\n");

		if(verbose >= 3)
			logerr("Move stats:\n" + move_stats(vector<Move>()));
	}
}


int16_t AgentAB::negamax(const Board & board, int16_t alpha, int16_t beta, int depth) {
	nodes_seen++;

	int won = board.won();
	if(won >= 0){
		if(won == 0)
			return SCORE_DRAW;
		if(won == board.toplay())
			return SCORE_WIN;
		return SCORE_LOSS;
	}

	if (depth <= 0){ //terminal node?
		return -board.score();
	}

	int16_t score = SCORE_LOSS;
	Move bestmove = M_RESIGN;
	Node * node;

	if(TT && (node = tt_get(board)) && node->depth >= depth){
		switch(node->flag){
		case VALID:  return node->score;
		case LBOUND: alpha = max(alpha, node->score); break;
		case UBOUND: beta  = min(beta,  node->score); break;
		default:     assert(false && "Unknown flag!");
		}
		if(alpha >= beta)
			return node->score;

		//try the previous best move first
		if(board.valid_move_fast(node->bestmove)){ // probably the best move, but for a symmetric version
			bestmove = node->bestmove;
			Board n = board;
			bool move_success = n.move(bestmove);
			assert(move_success);
			score = -negamax(n, -beta, -alpha, depth-1);
		}
	}

	if (score < beta) { // no cutoff from bestmove
		//TODO: sort moves first?

		//generate moves
		for (MoveIterator move(board); !move.done(); ++move) {
			int16_t value = -negamax(move.board(), -beta, -max(alpha, score), depth-1);
			if (score < value) {
				score = value;
				bestmove = *move;
				if (score >= beta){
					break;
				}
			}
		}
	}

	if (TT) {
		uint8_t flag = (score <= alpha ? UBOUND :
		                score >= beta  ? LBOUND : VALID);
		tt_set(Node(board.hash(), score, bestmove, depth, flag));
	}
	return score;
}

string AgentAB::move_stats(vector<Move> moves) const {
	string s = "";

	Board b = rootboard;
	for(vector<Move>::iterator m = moves.begin(); m != moves.end(); ++m)
		b.move(*m);

	for(MoveIterator move(b); !move.done(); ++move){
		if(const Node * n = tt_get(move.board())) {
			s += n->to_s(*move) + "\n";
		} else {
			s += "move: " + move->to_s() + ", unknown\n";
		}
	}
	return s;
}

Move AgentAB::return_move(const Board & board, int verbose) const {
	if(const Node * n = tt_get(board))
		return n->bestmove;

	int score = SCORE_LOSS;
	Move best = M_RESIGN;
	for(MoveIterator move(board); !move.done(); ++move){
		if(const Node * n = tt_get(move.board())) {
			if(score < n->score){
				score = n->score;
				best = *move;
			}
		} else if (score == SCORE_LOSS && best == M_RESIGN) {
			best = M_UNKNOWN;
		}
	}
	return best;
}

vector<Move> AgentAB::get_pv() const {
	vector<Move> pv;

	Board b = rootboard;
	int i = 20;
	while (i--) {
		Move m = return_move(b);
		if(m == M_UNKNOWN || m == M_RESIGN)
			break;
		pv.push_back(m);
		bool move_success = b.move(m);
		assert(move_success);
	}

	if(pv.size() == 0)
		pv.push_back(Move(M_RESIGN));

	return pv;
}

uint64_t mix_bits(uint64_t h){
	h ^= (h >> 17);
	h ^= (h << 31);
	h ^= (h >>  8);
	return h;
}

AgentAB::Node * AgentAB::tt(uint64_t hash) const {
	hash = mix_bits(hash);
	return & TT[hash % maxnodes];
}

AgentAB::Node * AgentAB::tt_get(const Board & b) const {
	return tt_get(b.hash());
}
AgentAB::Node * AgentAB::tt_get(uint64_t h) const {
	Node * n = tt(h);
	return (n->hash == h ? n : NULL);
}
void AgentAB::tt_set(const Node & n) {
	*(tt(n.hash)) = n;
}
