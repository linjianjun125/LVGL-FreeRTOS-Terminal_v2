#include "tetris.h"
#include "devices_include.h"

// 方块形状定义
int shapes[7][4][4][4] = {
    // I 形方块
    {
        { {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0} },
        { {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0} },
        { {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0} }
    },
    // J 形方块
    {
        { {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0} },
        { {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0} }
    },
    // L 形方块
    {
        { {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0} },
        { {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {1, 1, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0} }
    },
    // O 形方块
    {
        { {1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
        { {1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
        { {1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
        { {1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    },
    // S 形方块
    {
        { {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
        { {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
        { {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
        { {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}
    },
    // T 形方块
    {
        { {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {1, 0, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0} },
        { {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {0, 1, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0} }
    },
    // Z 形方块
    {
        { {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0} },
        { {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0} }
    }
};


/**
 * @brief 初始化游戏状态
 */
void tetris_initGame(Tetris* tetris)
{
    // 清空面板
    for (int i = 0; i < TETRIS_WIDTH; i++) {
        for (int j = 0; j < TETRIS_HEIGHT; j++) {
            tetris->board[i][j] = Tetris_Board_None;
        }
    }

    tetris->isover = false;
    tetris->stop = false;
    tetris->score = 0;
    
    // 随机生成第一个方块
    tetris->current.type = sys_random_number() % 7;
    tetris->current.x = TETRIS_WIDTH / 2 - 2;
    tetris->current.y = 0;
    tetris->current.rotation = 0;
}

/**
 * @brief 碰撞检测
 */
int tetris_canMove(Tetris* tetris, int newX, int newY, int newRotation)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shapes[tetris->current.type][newRotation][i][j] == 1) {
                int x = newX + i;
                int y = newY + j;
                
                // 检查边界越界
                if (x < 0 || x >= TETRIS_WIDTH || y >= TETRIS_HEIGHT) return 0;
                
                // 检查与已有方块重叠
                if (y >= 0 && tetris->board[x][y] != Tetris_Board_None) return 0;
            }
        }
    }
    return 1;
}

/**
 * @brief 向下移动逻辑（含消行与游戏判定）
 */
void tetris_moveDown(Tetris* tetris)
{
    if (tetris->isover || tetris->stop) return;

    // 1. 若能下落
    if (tetris_canMove(tetris, tetris->current.x, tetris->current.y + 1, tetris->current.rotation)) {
        tetris->current.y++;
    }
    // 2. 落地固定
    else {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                int x = tetris->current.x + i;
                int y = tetris->current.y + j;
                if (x >= 0 && x < TETRIS_WIDTH && y >= 0 && y < TETRIS_HEIGHT) {
                    if (shapes[tetris->current.type][tetris->current.rotation][i][j] == 1) {
                        tetris->board[x][y] = tetris->current.type; 
                    }
                }
            }
        }

        // 3. 消行检测
        int fullLines = 0;
        for (int i = 0; i < TETRIS_HEIGHT; i++) {
            int isFull = 1;
            for (int j = 0; j < TETRIS_WIDTH; j++) {
                if (tetris->board[j][i] == Tetris_Board_None) {
                    isFull = 0;
                    break;
                }
            }
            if (isFull) {
                fullLines++;
                for (int k = i; k > 0; k--) {
                    for (int l = 0; l < TETRIS_WIDTH; l++) {
                        tetris->board[l][k] = tetris->board[l][k - 1];
                    }
                }
                for (int l = 0; l < TETRIS_WIDTH; l++) tetris->board[l][0] = Tetris_Board_None;
            }
        }
        tetris->score += fullLines * 100;

        // 4. 生成下一方块
        tetris->current.type = sys_random_number() % 7;
        tetris->current.x = TETRIS_WIDTH / 2 - 2;
        tetris->current.y = 0;
        tetris->current.rotation = 0;

        if (!tetris_canMove(tetris, tetris->current.x, tetris->current.y, tetris->current.rotation)) {
            tetris->isover = true;
        }
    }
}

/**
 * @brief 向左移动
 */
void tetris_moveLeft(Tetris* tetris)
{
    if (tetris_canMove(tetris, tetris->current.x - 1, tetris->current.y, tetris->current.rotation)) {
        tetris->current.x--;
    }
}

/**
 * @brief 向右移动
 */
void tetris_moveRight(Tetris* tetris)
{
    if (tetris_canMove(tetris, tetris->current.x + 1, tetris->current.y, tetris->current.rotation)) {
        tetris->current.x++;
    }
}

/**
 * @brief 顺时针旋转
 */
void tetris_rotate(Tetris* tetris)
{
    int nextRot = (tetris->current.rotation + 1) % 4;
    if (tetris_canMove(tetris, tetris->current.x, tetris->current.y, nextRot)) {
        tetris->current.rotation = nextRot;
    }
}

/**
 * @brief 判定游戏结束
 */
bool tetris_gameOver(Tetris* tetris)
{
    return tetris->isover;
}





