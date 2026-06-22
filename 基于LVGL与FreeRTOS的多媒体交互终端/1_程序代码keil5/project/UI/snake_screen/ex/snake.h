#ifndef __GAME_SNAKE_H
#define __GAME_SNAKE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "sys.h"

/*--- 游戏配置宏 ---*/
#define SNAKE_CAN_THROUGH_WALL    1  // 1: 开启穿墙, 0: 撞墙死
#define SNAKE_WIDTH               18
#define SNAKE_HEIGHT              14

typedef struct {
    int x;
    int y;
} Food;

typedef struct {
    int8_t x[SNAKE_WIDTH * SNAKE_HEIGHT];
    int8_t y[SNAKE_WIDTH * SNAKE_HEIGHT];
    int length;
    int8_t direction; // 1:右, 2:下, 3:左, 4:上
    bool isover;
    bool stop;
    Food food;
} Snake;

enum {
    SNAKE_UP = 1,
    SNAKE_DOWN,
    SNAKE_LEFT,
    SNAKE_RIGHT
};

/*--- 函数接口 ---*/
void snake_initSnake(Snake* snake);
void snake_initFood(Snake *snake);
void snake_move(Snake* snake);
bool snake_isGameOver(Snake* snake);
void snake_input(Snake* snake, int input);

#endif
