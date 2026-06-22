#ifndef __GAME_TETRIS_H
#define __GAME_TETRIS_H

#include <stdbool.h>
#include <stdint.h>
#include "sys.h"

/** @brief 游戏区域逻辑网格尺寸 */
#define TETRIS_WIDTH    10
#define TETRIS_HEIGHT   15

/**
 * @struct Tetromino
 * @brief 俄罗斯方块单体结构体
 */
typedef struct {
    int x;            /**< 当前 X 坐标 */
    int y;            /**< 当前 Y 坐标 */
    int rotation;     /**< 当前旋转角度 (0-3) */
    int type;         /**< 方块形状类型 (0-6) */
} Tetromino;

/**
 * @enum Tetris_Board_Status
 * @brief 面板格子状态枚举
 */
enum Tetris_Board_Status {
    Tetris_Board_None = 7,  /**< 空白格标志 */
};

/**
 * @struct Tetris
 * @brief 俄罗斯方块游戏引擎核心结构体
 */
typedef struct {
    int board[TETRIS_WIDTH][TETRIS_HEIGHT]; /**< 存储已固定方块的面板 */
    Tetromino current;                      /**< 当前正在下落的方块 */
    int score;                              /**< 游戏得分 */
    bool isover;                            /**< 游戏结束标志 */
    bool stop;                              /**< 暂停标志 */
} Tetris;

extern int shapes[7][4][4][4];

/**
 * @brief 初始化游戏状态，重置面板和得分
 * @param tetris 指向游戏结构体的指针
 */
void tetris_initGame(Tetris* tetris);

/**
 * @brief 碰撞检测逻辑
 * @param tetris 指向游戏结构体的指针
 * @param newX 尝试移动的新 X 坐标
 * @param newY 尝试移动的新 Y 坐标
 * @param newRotation 尝试旋转的新角度
 * @return int 1: 可以移动/旋转, 0: 发生碰撞
 */
int tetris_canMove(Tetris* tetris, int newX, int newY, int newRotation);

/**
 * @brief 执行向下移动一步的逻辑，处理固定方块及消行
 * @param tetris 指向游戏结构体的指针
 */
void tetris_moveDown(Tetris* tetris);

/**
 * @brief 执行向左移动逻辑
 * @param tetris 指向游戏结构体的指针
 */
void tetris_moveLeft(Tetris* tetris);

/**
 * @brief 执行向右移动逻辑
 * @param tetris 指向游戏结构体的指针
 */
void tetris_moveRight(Tetris* tetris);

/**
 * @brief 执行顺时针旋转逻辑
 * @param tetris 指向游戏结构体的指针
 */
void tetris_rotate(Tetris* tetris);

/**
 * @brief 获取游戏是否结束
 * @param tetris 指向游戏结构体的指针
 * @return true 游戏已结束, false 游戏进行中
 */
bool tetris_gameOver(Tetris* tetris);

#endif
