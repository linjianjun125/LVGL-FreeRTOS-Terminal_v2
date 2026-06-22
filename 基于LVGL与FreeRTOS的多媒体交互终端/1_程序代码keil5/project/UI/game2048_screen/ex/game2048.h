#ifndef __GAME_2048_H
#define __GAME_2048_H

#include "sys.h"

#define GAME2048_SIZE 4


void game2048_initBoard(int board[GAME2048_SIZE][GAME2048_SIZE]);
int game2048_canMove(int board[GAME2048_SIZE][GAME2048_SIZE]) ;
void game2048_mergeRow(int row[GAME2048_SIZE]) ;
int game2048_moveLeft(int board[GAME2048_SIZE][GAME2048_SIZE]);
int game2048_moveRight(int board[GAME2048_SIZE][GAME2048_SIZE]) ;
int game2048_moveUp(int board[GAME2048_SIZE][GAME2048_SIZE]) ;
int game2048_moveDown(int board[GAME2048_SIZE][GAME2048_SIZE]) ;
void game2048_generateNewNumber(int board[GAME2048_SIZE][GAME2048_SIZE]) ;
int game2048_getIndex(int number);

#endif

