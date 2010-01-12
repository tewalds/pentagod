/*
 * A UTC monte carlo player
 *
 */

#include "atomic.h"
#include "player.h"
#include <cmath>


class PlayerUCT : public Player {
	int conflicts;
	struct Node {
		int visits;
		int score;
		int numchildren;
		Node *children;
		Board board;
	
		Node(){
			visits = 0;
			score = 0;
			numchildren = 0;
			children = NULL;
		}
	
		Node(const Board & nboard){
			numchildren = 0;
			children = NULL;
			board = nboard;
			score = (board.score > 0) + (board.score > 10);
			visits = 1;
		}
	
		~Node(){
			if(numchildren)
				delete[] children;
		}

		bool addChildren(int num, Board *boards){
			if(children)
				return false;

			numchildren = num;
			Node * temp = new Node[num];
			for(int i = 0; i < num; i++)
				temp[i] = Node(boards[i]);

			CASv(children, NULL, temp);
			if(children != temp){
				delete[] temp;
				return false;
			}
			return true;
		}

		float winrate(){
			return (float)score/visits;
		}
	};

	int maxruns;
	int maxnodes;
	float explore;  //greater than one favours exploration, smaller than one favours exploitation
	
	int remainnodes;
	
	int minvisitschildren; //number of visits to a node before expanding its children nodes
	int minvisitspriority; //give priority to this node if it has less than this many visits
public:

	PlayerUCT(int numruns, int maxmem = 2000000000, float exploration = 1.0, int mvchildren = 2, int mvpriority = 2){
		maxnodes = maxmem/sizeof(Node);
		maxruns = numruns;
		explore = exploration;
		minvisitschildren = mvchildren;
		minvisitspriority = mvpriority;
	}

	~PlayerUCT(){
	}

	void describe(){
		printf("class PlayerUCT - a UCT monte carlo player playing %i runs with %i nodes and %.2f explore\n", maxruns, maxnodes, explore);
	}

	void print_stats(unsigned int runtime){
		printf("Moves: %u, Time: %u ms, Moves/s: %u\n", maxruns, runtime, (unsigned int)(1000.0*maxruns/runtime));
		totalmoves += maxruns;
	}

	int walk_tree(Node & node, int depth){
		int result, won = -1;

		INCR(node.visits);

		if(node.children){
		//choose a child and recurse
			int maxi = 0;
			if(node.visits < node.numchildren/5){
				maxi = rand() % node.numchildren;
			}else{
				float val, maxval = -1000000000;
				float logvisits = log(node.visits)/5;
				Node * child;

				for(int i = 0; i < node.numchildren; i++){
					child = & node.children[i];

					if(child->visits < minvisitspriority) // give priority to nodes that have few or no visits
						val = 10000 - child->visits*1000 + rand()%100;
					else
						val = child->winrate() + explore*sqrt(logvisits/child->visits);

					if(maxval < val){
						maxval = val;
						maxi = i;
					}
				}
			}

			result = - walk_tree(node.children[maxi], depth+1);
		}else if(node.visits <= minvisitschildren || remainnodes <= 0){
		//do random game on this node
			won = rand_game(node.board);
			INCR(depths[depth]);
		}else if((won = node.board.won()) >= 0){
			//already done
		}else{
		//create children
			Board children[(36-node.board.nummoves)*8];
			int numchildren = node.board.getchildren(children, (depth < 3), true);

			if(!node.addChildren(numchildren, children))
				INCR(conflicts);

			PLUS(remainnodes, - numchildren);

		//recurse on a random child
			result = - walk_tree(node.children[rand() % numchildren], depth+1);
		}
		if(won >= 0)
			result = (won == 0 ? 0 : won == node.board.turn() ? -1 : 1);

		PLUS(node.score, result+1);
		return result;
	}

	Board search_move(Board board, bool output){
		Node root(board);

		remainnodes = maxnodes;

conflicts = 0;

	//call a recursive function to find a node and do a random game from there
		#pragma omp parallel for schedule(dynamic)
		for(int i = 0; i < maxruns; i++)
			walk_tree(root, 0);

		if(depths[0] == 0)
			depths[0] = 1;

	printf("Conflicts: %i\n", conflicts);

		if(output){
		//print all the first depth results
//			for(int i = 0; i < root.numchildren; i++)
//				printf("%.2f/%i ", root.children[i].winrate(), root.children[i].visits);
//			printf("\n");

			printf("Nodes generated: %i/%i\n", maxnodes - remainnodes, maxnodes);
		}

	//return the best one
		int maxi = 0;
		for(int i = 1; i < root.numchildren; i++)
//			if(root.children[maxi].winrate() < root.children[i].winrate())
			if(root.children[maxi].visits < root.children[i].visits)
				maxi = i;

		return root.children[maxi].board;
	}
};

