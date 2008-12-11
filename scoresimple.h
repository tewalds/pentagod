#ifndef __SCORESIMPLE_H_
#define __SCORESIMPLE_H_

#include "board.h"
#define xy(x,y) ((x) + ((y)*6))


/*
 * Implements 4 equivalent scoring functions:
 * - win condition oriented scan: going through each win condition checking if it's possible
 *   - getscore1 does each side independantly
 *   - getscore2 does both sides at the same time, saving a couple checks
 * - position oriented scan: go through each position and adding its value to the list of win conditions it is part of
 *   - getscore3 is an optimized version of getscore4
 *   - getscore4 is a fairly easy to read implementation, but pretty slow
 *
 * the getscore function is the general entry point, and should use the best scoring function for any point in time
 *
 */

const int scoremap[] = {0, 1, 4, 16, 256, 100000000, 100000000};
class ScoreSimple {
public:
	static int getscore(const Board & board){
		return getscore2(board); //just use getscore2, since it seems to be the fastest
	}

//protected:
/*
 * Gives the score for a single line of a varied length
 */
	static int scoreline(int count){
		return scoremap[count];
	}


/*
 * Gives a score for a player by going through each possible win condition (ie row, column or diagonal)
 * and adds the value from scoreline(length) to the score. With a fairly full board, this is fast since
 * it can bail early on each line.
 */
	static int getscore1(const Board & board){
		return scoreside(board, board.turn) - scoreside(board, (board.turn == 1 ? 2 : 1) );
	}

	static int scoreside(const Board & board, const char side){
		char otherside = (side == 1 ? 2 : 1);

		int scr = 0;
		int count, count2;

	//check horizontal lines
		for(int y = 0; y < 6; y++){
			count = 0;

			if(otherside == board.squares[xy(1,y)]) continue;
			else if(side == board.squares[xy(1,y)]) ++count;

			if(otherside == board.squares[xy(2,y)]) continue;
			else if(side == board.squares[xy(2,y)]) ++count;

			if(otherside == board.squares[xy(3,y)]) continue;
			else if(side == board.squares[xy(3,y)]) ++count;

			if(otherside == board.squares[xy(4,y)]) continue;
			else if(side == board.squares[xy(4,y)]) ++count;

			if(otherside != board.squares[xy(0,y)]){
				count2 = count;
				if(side == board.squares[xy(0,y)]) ++count2;

				scr += scoreline(count2);
			}
			
			if(otherside != board.squares[xy(5,y)]){
				count2 = count;
				if(side == board.squares[xy(5,y)]) ++count2;

				scr += scoreline(count2);
			}
		}

	//check vertical lines
		for(int x = 0; x < 6; x++){
			count = 0;

			if(otherside == board.squares[xy(x,1)]) continue;
			else if(side == board.squares[xy(x,1)]) ++count;

			if(otherside == board.squares[xy(x,2)]) continue;
			else if(side == board.squares[xy(x,2)]) ++count;

			if(otherside == board.squares[xy(x,3)]) continue;
			else if(side == board.squares[xy(x,3)]) ++count;

			if(otherside == board.squares[xy(x,4)]) continue;
			else if(side == board.squares[xy(x,4)]) ++count;

			if(otherside != board.squares[xy(x,0)]){
				count2 = count;
				if(side == board.squares[xy(x,0)]) ++count2;

				scr += scoreline(count2);
			}
			
			if(otherside != board.squares[xy(x,5)]){
				count2 = count;
				if(side == board.squares[xy(x,5)]) ++count2;

				scr += scoreline(count2);
			}
		}

	//check center diagonal lines
		do{
			count = 0;

			if(otherside == board.squares[xy(1,1)]) break;
			else if(side == board.squares[xy(1,1)]) ++count;

			if(otherside == board.squares[xy(2,2)]) break;
			else if(side == board.squares[xy(2,2)]) ++count;

			if(otherside == board.squares[xy(3,3)]) break;
			else if(side == board.squares[xy(3,3)]) ++count;

			if(otherside == board.squares[xy(4,4)]) break;
			else if(side == board.squares[xy(4,4)]) ++count;

			if(otherside != board.squares[xy(0,0)]){
				count2 = count;
				if(side == board.squares[xy(0,0)]) ++count2;

				scr += scoreline(count2);
			}
			
			if(otherside != board.squares[xy(5,5)]){
				count2 = count;
				if(side == board.squares[xy(5,5)]) ++count2;

				scr += scoreline(count2);
			}
		}while(0);

		do{
			count = 0;

			if(otherside == board.squares[xy(4,1)]) break;
			else if(side == board.squares[xy(4,1)]) ++count;

			if(otherside == board.squares[xy(3,2)]) break;
			else if(side == board.squares[xy(3,2)]) ++count;

			if(otherside == board.squares[xy(2,3)]) break;
			else if(side == board.squares[xy(2,3)]) ++count;

			if(otherside == board.squares[xy(1,4)]) break;
			else if(side == board.squares[xy(1,4)]) ++count;

			if(otherside != board.squares[xy(0,5)]){
				count2 = count;
				if(side == board.squares[xy(0,5)]) ++count2;

				scr += scoreline(count2);
			}
			
			if(otherside != board.squares[xy(5,0)]){
				count2 = count;
				if(side == board.squares[xy(5,0)]) ++count2;

				scr += scoreline(count2);
			}
		}while(0);


	//check the off-center diagonal lines
		do{
			count = 0;

			if(otherside == board.squares[xy(0,1)]) break;
			else if(side == board.squares[xy(0,1)]) ++count;

			if(otherside == board.squares[xy(1,2)]) break;
			else if(side == board.squares[xy(1,2)]) ++count;

			if(otherside == board.squares[xy(2,3)]) break;
			else if(side == board.squares[xy(2,3)]) ++count;

			if(otherside == board.squares[xy(3,4)]) break;
			else if(side == board.squares[xy(3,4)]) ++count;

			if(otherside == board.squares[xy(4,5)]) break;
			else if(side == board.squares[xy(4,5)]) ++count;

			scr += scoreline(count);
		}while(0);
	
		do{
			count = 0;

			if(otherside == board.squares[xy(1,0)]) break;
			else if(side == board.squares[xy(1,0)]) ++count;

			if(otherside == board.squares[xy(2,1)]) break;
			else if(side == board.squares[xy(2,1)]) ++count;
                                 
			if(otherside == board.squares[xy(3,2)]) break;
			else if(side == board.squares[xy(3,2)]) ++count;
                                 
			if(otherside == board.squares[xy(4,3)]) break;
			else if(side == board.squares[xy(4,3)]) ++count;
                                 
			if(otherside == board.squares[xy(5,4)]) break;
			else if(side == board.squares[xy(5,4)]) ++count;

			scr += scoreline(count);
		}while(0);

		do{
			count = 0;

			if(otherside == board.squares[xy(0,4)]) break;
			else if(side == board.squares[xy(0,4)]) ++count;

			if(otherside == board.squares[xy(1,3)]) break;
			else if(side == board.squares[xy(1,3)]) ++count;

			if(otherside == board.squares[xy(2,2)]) break;
			else if(side == board.squares[xy(2,2)]) ++count;

			if(otherside == board.squares[xy(3,1)]) break;
			else if(side == board.squares[xy(3,1)]) ++count;

			if(otherside == board.squares[xy(4,0)]) break;
			else if(side == board.squares[xy(4,0)]) ++count;

			scr += scoreline(count);
		}while(0);

		do{
			count = 0;

			if(otherside == board.squares[xy(1,5)]) break;
			else if(side == board.squares[xy(1,5)]) ++count;

			if(otherside == board.squares[xy(2,4)]) break;
			else if(side == board.squares[xy(2,4)]) ++count;
                                 
			if(otherside == board.squares[xy(3,3)]) break;
			else if(side == board.squares[xy(3,3)]) ++count;
                                 
			if(otherside == board.squares[xy(4,2)]) break;
			else if(side == board.squares[xy(4,2)]) ++count;
                                 
			if(otherside == board.squares[xy(5,1)]) break;
			else if(side == board.squares[xy(5,1)]) ++count;

			scr += scoreline(count);
		}while(0);

		return scr;
	}


/*
 * Gives a score for a board by going through each possible win condition (ie row, column or diagonal)
 * and adds the value from scoreline(length) to the score. With a fairly full board, this is fast since
 * it can bail early on each line.
 */
 	static int getscore2(const Board & board){
		char otherside = (board.turn == 1 ? 2 : 1);

		int scr = 0;

		int count, count2;
		char side, side2, val;

	//check horizontal lines
		for(int y = 0; y < 6; y++){
			count = 1;
			side = 0;
			side2 = 0;

			if((val = board.squares[xy(1,y)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else continue;
			}

			if((val = board.squares[xy(2,y)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else continue;
			}

			if((val = board.squares[xy(3,y)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else continue;
			}

			if((val = board.squares[xy(4,y)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else continue;
			}

			count2 = count;
			side2 = side;

			if((val = board.squares[xy(0,y)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else side = 0;
			}

			if((val = board.squares[xy(5,y)])){
				if(!side2) side2 = val;
				else if(side2 == val) ++count2;
				else side2 = 0;
			}

			if(side)
				scr += (side == otherside ? -scoreline(count) : scoreline(count));
				
			if(side2)
				scr += (side2 == otherside ? -scoreline(count2) : scoreline(count2));
		}

	//check vertical lines
		for(int x = 0; x < 6; x++){
			count = 1;
			side = 0;
			side2 = 0;

			if((val = board.squares[xy(x,1)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else continue;
			}

			if((val = board.squares[xy(x,2)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else continue;
			}

			if((val = board.squares[xy(x,3)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else continue;
			}

			if((val = board.squares[xy(x,4)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else continue;
			}

			count2 = count;
			side2 = side;

			if((val = board.squares[xy(x,0)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else side = 0;
			}

			if((val = board.squares[xy(x,5)])){
				if(!side2) side2 = val;
				else if(side2 == val) ++count2;
				else side2 = 0;
			}

			if(side)
				scr += (side == otherside ? -scoreline(count) : scoreline(count));
				
			if(side2)
				scr += (side2 == otherside ? -scoreline(count2) : scoreline(count2));
		}

	//check center diagonal lines
		do{
			count = 1;
			side = 0;
			side2 = 0;

			if((val = board.squares[xy(1,1)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(2,2)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(3,3)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(4,4)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			count2 = count;
			side2 = side;

			if((val = board.squares[xy(0,0)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else side = 0;
			}

			if((val = board.squares[xy(5,5)])){
				if(!side2) side2 = val;
				else if(side2 == val) ++count2;
				else side2 = 0;
			}

			if(side)
				scr += (side == otherside ? -scoreline(count) : scoreline(count));
				
			if(side2)
				scr += (side2 == otherside ? -scoreline(count2) : scoreline(count2));
		}while(0);

		do{
			count = 1;
			side = 0;
			side2 = 0;

			if((val = board.squares[xy(4,1)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(3,2)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(2,3)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(1,4)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			count2 = count;
			side2 = side;

			if((val = board.squares[xy(5,0)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else side = 0;
			}

			if((val = board.squares[xy(0,5)])){
				if(!side2) side2 = val;
				else if(side2 == val) ++count2;
				else side2 = 0;
			}

			if(side)
				scr += (side == otherside ? -scoreline(count) : scoreline(count));
				
			if(side2)
				scr += (side2 == otherside ? -scoreline(count2) : scoreline(count2));
		}while(0);


	//check the off-center diagonal lines
		do{
			count = 1;
			side = 0;

			if((val = board.squares[xy(0,1)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(1,2)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(2,3)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(3,4)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(4,5)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}
								
			if(side)
				scr += (side == otherside ? -scoreline(count) : scoreline(count));
		}while(0);

		do{
			count = 1;
			side = 0;

			if((val = board.squares[xy(1,0)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(2,1)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(3,2)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(4,3)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(5,4)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}
								
			if(side)
				scr += (side == otherside ? -scoreline(count) : scoreline(count));
		}while(0);


		do{
			count = 1;
			side = 0;

			if((val = board.squares[xy(0,4)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(1,3)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(2,2)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(3,1)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(4,0)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}
								
			if(side)
				scr += (side == otherside ? -scoreline(count) : scoreline(count));
		}while(0);

		do{
			count = 1;
			side = 0;

			if((val = board.squares[xy(1,5)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(2,4)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(3,3)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(4,2)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}

			if((val = board.squares[xy(5,1)])){
				if(!side) side = val;
				else if(side == val) ++count;
				else break;
			}
								
			if(side)
				scr += (side == otherside ? -scoreline(count) : scoreline(count));
		}while(0);

		return scr;
	}


/*
 * Gives a score that takes both sides into consideration by going square by square, adding
 * the it's length value to each line. For boards with few pieces, it is fast, since it doesn't
 * do many operations for empty spaces. It can't bail early on lines that are used by both players though.
 */
	static int getscore3(const Board & board){
		int scr = 0;
		
		unsigned char count[32] = {
			0,0,0,0,0,0,   0,0,0,0,0,0, //horizontal, left, right
			0,0,0,0,0,0,   0,0,0,0,0,0, //vertical, top, bottom
			0,0,0,0,       0,0,0,0      //diagonal, -x, x (left -> right, top -> bottom of start)
			};

		switch(board.squares[0]){
			case 1: count[0]++;                   count[12]++;                  count[24]++;                  break;
			case 2: count[0]+=16;                 count[12]+=16;                count[24]+=16;                break;
		}

		switch(board.squares[1]){
			case 1: count[0]++;    count[6]++;    count[13]++;                  count[25]++;                  break;
			case 2: count[0]+=16;  count[6]+=16;  count[13]+=16;                count[25]+=16;                break;
		}

		switch(board.squares[2]){
			case 1: count[0]++;    count[6]++;    count[14]++;                                                break;
			case 2: count[0]+=16;  count[6]+=16;  count[14]+=16;                                              break;
		}

		switch(board.squares[3]){
			case 1: count[0]++;    count[6]++;    count[15]++;                                                break;
			case 2: count[0]+=16;  count[6]+=16;  count[15]+=16;                                              break;
		}

		switch(board.squares[4]){
			case 1: count[0]++;    count[6]++;    count[16]++;                  count[28]++;                  break;
			case 2: count[0]+=16;  count[6]+=16;  count[16]+=16;                count[28]+=16;                break;
		}

		switch(board.squares[5]){
			case 1:                count[6]++;    count[17]++;                  count[29]++;                  break;
			case 2:                count[6]+=16;  count[17]+=16;                count[29]+=16;                break;
		}


/*			0,0,0,0,0,0,   0,0,0,0,0,0, //horizontal, left, right
			0,0,0,0,0,0,   0,0,0,0,0,0, //vertical, top, bottom
			0,0,0,0        0,0,0,0      //diagonal, -x, x (left -> right, top -> bottom of start)
*/

		switch(board.squares[6]){
			case 1: count[1]++;                   count[12]++;   count[18]++;   count[26]++;                  break;
			case 2: count[1]+=16;                 count[12]+=16; count[18]+=16; count[26]+=16;                break;
		}

		switch(board.squares[7]){
			case 1: count[1]++;    count[7]++;    count[13]++;   count[19]++;   count[24]++;   count[27]++;   break;
			case 2: count[1]+=16;  count[7]+=16;  count[13]+=16; count[19]+=16; count[24]+=16; count[27]+=16; break;
		}

		switch(board.squares[8]){
			case 1: count[1]++;    count[7]++;    count[14]++;   count[20]++;   count[25]++;                  break;
			case 2: count[1]+=16;  count[7]+=16;  count[14]+=16; count[20]+=16; count[25]+=16;                break;
		}

		switch(board.squares[9]){
			case 1: count[1]++;    count[7]++;    count[15]++;   count[21]++;   count[28]++;                  break;
			case 2: count[1]+=16;  count[7]+=16;  count[15]+=16; count[21]+=16; count[28]+=16;                break;
		}

		switch(board.squares[10]){
			case 1: count[1]++;    count[7]++;    count[16]++;   count[22]++;   count[29]++;   count[30]++;   break;
			case 2: count[1]+=16;  count[7]+=16;  count[16]+=16; count[22]+=16; count[29]+=16; count[30]+=16; break;
		}

		switch(board.squares[11]){
			case 1:                count[7]++;    count[17]++;   count[23]++;   count[31]++;                  break;
			case 2:                count[7]+=16;  count[17]+=16; count[23]+=16; count[31]+=16;                break;
		}


/*			0,0,0,0,0,0,   0,0,0,0,0,0, //horizontal, left, right
			0,0,0,0,0,0,   0,0,0,0,0,0, //vertical, top, bottom
			0,0,0,0        0,0,0,0      //diagonal, -x, x (left -> right, top -> bottom of start)
*/

		switch(board.squares[12]){
			case 1: count[2]++;                   count[12]++;   count[18]++;                                 break;
			case 2: count[2]+=16;                 count[12]+=16; count[18]+=16;                               break;
		}

		switch(board.squares[13]){
			case 1: count[2]++;    count[8]++;    count[13]++;   count[19]++;   count[26]++;                  break;
			case 2: count[2]+=16;  count[8]+=16;  count[13]+=16; count[19]+=16; count[26]+=16;                break;
		}

		switch(board.squares[14]){
			case 1: count[2]++;    count[8]++;    count[14]++;   count[20]++;   count[24]++;   count[27]++;   count[28]++;   break;
			case 2: count[2]+=16;  count[8]+=16;  count[14]+=16; count[20]+=16; count[24]+=16; count[27]+=16; count[28]+=16; break;
		}

		switch(board.squares[15]){
			case 1: count[2]++;    count[8]++;    count[15]++;   count[21]++;   count[29]++;   count[30]++;   count[25]++;   break;
			case 2: count[2]+=16;  count[8]+=16;  count[15]+=16; count[21]+=16; count[29]+=16; count[30]+=16; count[25]+=16; break;
		}

		switch(board.squares[16]){
			case 1: count[2]++;    count[8]++;    count[16]++;   count[22]++;   count[31]++;                  break;
			case 2: count[2]+=16;  count[8]+=16;  count[16]+=16; count[22]+=16; count[31]+=16;                break;
		}

		switch(board.squares[17]){
			case 1:                count[8]++;    count[17]++;   count[23]++;                                 break;
			case 2:                count[8]+=16;  count[17]+=16; count[23]+=16;                               break;
		}


/*			0,0,0,0,0,0,   0,0,0,0,0,0, //horizontal, left, right
			0,0,0,0,0,0,   0,0,0,0,0,0, //vertical, top, bottom
			0,0,0,0        0,0,0,0      //diagonal, -x, x (left -> right, top -> bottom of start)
*/

		switch(board.squares[18]){
			case 1: count[3]++;                   count[12]++;   count[18]++;                                 break;
			case 2: count[3]+=16;                 count[12]+=16; count[18]+=16;                               break;
		}

		switch(board.squares[19]){
			case 1: count[3]++;    count[9]++;    count[13]++;   count[19]++;   count[28]++;                  break;
			case 2: count[3]+=16;  count[9]+=16;  count[13]+=16; count[19]+=16; count[28]+=16;                break;
		}

		switch(board.squares[20]){
			case 1: count[3]++;    count[9]++;    count[14]++;   count[20]++;   count[29]++;   count[30]++;   count[26]++;   break;
			case 2: count[3]+=16;  count[9]+=16;  count[14]+=16; count[20]+=16; count[29]+=16; count[30]+=16; count[26]+=16; break;
		}

		switch(board.squares[21]){
			case 1: count[3]++;    count[9]++;    count[15]++;   count[21]++;   count[24]++;   count[27]++;   count[31]++;   break;
			case 2: count[3]+=16;  count[9]+=16;  count[15]+=16; count[21]+=16; count[24]+=16; count[27]+=16; count[31]+=16; break;
		}

		switch(board.squares[22]){
			case 1: count[3]++;    count[9]++;    count[16]++;   count[22]++;   count[25]++;                  break;
			case 2: count[3]+=16;  count[9]+=16;  count[16]+=16; count[22]+=16; count[25]+=16;                break;
		}

		switch(board.squares[23]){
			case 1:                count[9]++;    count[17]++;   count[23]++;                                 break;
			case 2:                count[9]+=16;  count[17]+=16; count[23]+=16;                               break;
		}


/*			0,0,0,0,0,0,   0,0,0,0,0,0, //horizontal, left, right
			0,0,0,0,0,0,   0,0,0,0,0,0, //vertical, top, bottom
			0,0,0,0        0,0,0,0      //diagonal, -x, x (left -> right, top -> bottom of start)
*/

		switch(board.squares[24]){
			case 1: count[4]++;                   count[12]++;   count[18]++;   count[28]++;                  break;
			case 2: count[4]+=16;                 count[12]+=16; count[18]+=16; count[28]+=16;                break;
		}

		switch(board.squares[25]){
			case 1: count[4]++;    count[10]++;   count[13]++;   count[19]++;   count[29]++;   count[30]++;   break;
			case 2: count[4]+=16;  count[10]+=16; count[13]+=16; count[19]+=16; count[29]+=16; count[30]+=16; break;
		}

		switch(board.squares[26]){
			case 1: count[4]++;    count[10]++;   count[14]++;   count[20]++;   count[31]++;                  break;
			case 2: count[4]+=16;  count[10]+=16; count[14]+=16; count[20]+=16; count[31]+=16;                break;
		}

		switch(board.squares[27]){
			case 1: count[4]++;    count[10]++;   count[15]++;   count[21]++;   count[26]++;                  break;
			case 2: count[4]+=16;  count[10]+=16; count[15]+=16; count[21]+=16; count[26]+=16;                break;
		}

		switch(board.squares[28]){
			case 1: count[4]++;    count[10]++;   count[16]++;   count[22]++;   count[24]++;   count[27]++;   break;
			case 2: count[4]+=16;  count[10]+=16; count[16]+=16; count[22]+=16; count[24]+=16; count[27]+=16; break;
		}

		switch(board.squares[29]){
			case 1:                count[10]++;   count[17]++;   count[23]++;   count[25]++;                  break;
			case 2:                count[10]+=16; count[17]+=16; count[23]+=16; count[25]+=16;                break;
		}


/*			0,0,0,0,0,0,   0,0,0,0,0,0, //horizontal, left, right
			0,0,0,0,0,0,   0,0,0,0,0,0, //vertical, top, bottom
			0,0,0,0        0,0,0,0      //diagonal, -x, x (left -> right, top -> bottom of start)
*/

		switch(board.squares[30]){
			case 1: count[5]++;                                  count[18]++;   count[30]++;                  break;
			case 2: count[5]+=16;                                count[18]+=16; count[30]+=16;                break;
		}

		switch(board.squares[31]){
			case 1: count[5]++;    count[11]++;                  count[19]++;   count[31]++;                  break;
			case 2: count[5]+=16;  count[11]+=16;                count[19]+=16; count[31]+=16;                break;
		}

		switch(board.squares[32]){
			case 1: count[5]++;    count[11]++;                  count[20]++;                                 break;
			case 2: count[5]+=16;  count[11]+=16;                count[20]+=16;                               break;
		}

		switch(board.squares[33]){
			case 1: count[5]++;    count[11]++;                  count[21]++;                                 break;
			case 2: count[5]+=16;  count[11]+=16;                count[21]+=16;                               break;
		}

		switch(board.squares[34]){
			case 1: count[5]++;    count[11]++;                  count[22]++;   count[26]++;                  break;
			case 2: count[5]+=16;  count[11]+=16;                count[22]+=16; count[26]+=16;                break;
		}

		switch(board.squares[35]){
			case 1:                count[11]++;                  count[23]++;   count[27]++;                  break;
			case 2:                count[11]+=16;                count[23]+=16; count[27]+=16;                break;
		}


	//tally the scores
		for(int i = 0; i < 32; i++){
			if((count[i] & 0xF0) == 0) //nothing for player 2
				scr += scoreline(count[i]);

			if((count[i] & 0x0F) == 0) //nothing for player 1
				scr -= scoreline(count[i] >> 4);
		}



		if(board.turn == 1)
			return scr;
		else
			return -scr;
	}


//a slower, but easier to read version of getscore3 above
	static int getscore4(const Board & board){
		int scr = 0;
		
		unsigned char count[32] = {
			0,0,0,0,0,0,   0,0,0,0,0,0, //horizontal, left, right
			0,0,0,0,0,0,   0,0,0,0,0,0, //vertical, top, bottom
			0,0,0,0,       0,0,0,0      //diagonal, -x, x (left -> right, top -> bottom of start)
			};

		int v;

		for(int i = 0 ; i < 36; i++){
			if(board.squares[i]){
				v = (board.squares[i] == 1 ? 1 : 16);

				switch(i){
					case 0:  count[0]+=v;               count[12]+=v;               count[24]+=v;               break;
					case 1:  count[0]+=v; count[6]+=v;  count[13]+=v;               count[25]+=v;               break;
					case 2:  count[0]+=v; count[6]+=v;  count[14]+=v;                                           break;
					case 3:  count[0]+=v; count[6]+=v;  count[15]+=v;                                           break;
					case 4:  count[0]+=v; count[6]+=v;  count[16]+=v;               count[28]+=v;               break;
					case 5:               count[6]+=v;  count[17]+=v;               count[29]+=v;               break;

					case 6:  count[1]+=v;               count[12]+=v; count[18]+=v; count[26]+=v;               break;
					case 7:  count[1]+=v; count[7]+=v;  count[13]+=v; count[19]+=v; count[24]+=v; count[27]+=v; break;
					case 8:  count[1]+=v; count[7]+=v;  count[14]+=v; count[20]+=v; count[25]+=v;               break;
					case 9:  count[1]+=v; count[7]+=v;  count[15]+=v; count[21]+=v; count[28]+=v;               break;
					case 10: count[1]+=v; count[7]+=v;  count[16]+=v; count[22]+=v; count[29]+=v; count[30]+=v; break;
					case 11:              count[7]+=v;  count[17]+=v; count[23]+=v; count[31]+=v;               break;

					case 12: count[2]+=v;               count[12]+=v; count[18]+=v;                             break;
					case 13: count[2]+=v; count[8]+=v;  count[13]+=v; count[19]+=v; count[26]+=v;               break;
					case 14: count[2]+=v; count[8]+=v;  count[14]+=v; count[20]+=v; count[24]+=v; count[27]+=v; count[28]+=v; break;
					case 15: count[2]+=v; count[8]+=v;  count[15]+=v; count[21]+=v; count[29]+=v; count[30]+=v; count[25]+=v; break;
					case 16: count[2]+=v; count[8]+=v;  count[16]+=v; count[22]+=v; count[31]+=v;               break;
					case 17:              count[8]+=v;  count[17]+=v; count[23]+=v;                             break;

					case 18: count[3]+=v;               count[12]+=v; count[18]+=v;                             break;
					case 19: count[3]+=v; count[9]+=v;  count[13]+=v; count[19]+=v; count[28]+=v;               break;
					case 20: count[3]+=v; count[9]+=v;  count[14]+=v; count[20]+=v; count[29]+=v; count[30]+=v; count[26]+=v; break;
					case 21: count[3]+=v; count[9]+=v;  count[15]+=v; count[21]+=v; count[24]+=v; count[27]+=v; count[31]+=v; break;
					case 22: count[3]+=v; count[9]+=v;  count[16]+=v; count[22]+=v; count[25]+=v;               break;
					case 23:              count[9]+=v;  count[17]+=v; count[23]+=v;                             break;

					case 24: count[4]+=v;               count[12]+=v; count[18]+=v; count[28]+=v;               break;
					case 25: count[4]+=v; count[10]+=v; count[13]+=v; count[19]+=v; count[29]+=v; count[30]+=v; break;
					case 26: count[4]+=v; count[10]+=v; count[14]+=v; count[20]+=v; count[31]+=v;               break;
					case 27: count[4]+=v; count[10]+=v; count[15]+=v; count[21]+=v; count[26]+=v;               break;
					case 28: count[4]+=v; count[10]+=v; count[16]+=v; count[22]+=v; count[24]+=v; count[27]+=v; break;
					case 29:              count[10]+=v; count[17]+=v; count[23]+=v; count[25]+=v;               break;

					case 30: count[5]+=v;                             count[18]+=v; count[30]+=v;               break;
					case 31: count[5]+=v;  count[11]+=v;              count[19]+=v; count[31]+=v;               break;
					case 32: count[5]+=v;  count[11]+=v;              count[20]+=v;                             break;
					case 33: count[5]+=v;  count[11]+=v;              count[21]+=v;                             break;
					case 34: count[5]+=v;  count[11]+=v;              count[22]+=v; count[26]+=v;               break;
					case 35:               count[11]+=v;              count[23]+=v; count[27]+=v;               break;
				}
			}
		}


	//tally the scores
		for(int i = 0; i < 32; i++){
			if((count[i] & 0xF0) == 0) //nothing for player 2
				scr += scoreline(count[i]);

			if((count[i] & 0x0F) == 0) //nothing for player 1
				scr -= scoreline(count[i] >> 4);
		}



		if(board.turn == 1)
			return scr;
		else
			return -scr;
	}

};

#endif


