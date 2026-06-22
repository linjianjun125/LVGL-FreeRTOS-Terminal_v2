#include "game2048.h"
#include "devices_include.h"


// 初始化游戏棋盘
void game2048_initBoard(int board[GAME2048_SIZE][GAME2048_SIZE])
{
    for (int i = 0; i < GAME2048_SIZE; i++)
    {
        for (int j = 0; j < GAME2048_SIZE; j++)
        {
            board[i][j] = 0;
        }
    }
    // 随机生成两个初始数字
    int count = 0;
    while (count < 2)
    {
        int row = sys_random_number() % GAME2048_SIZE;
        int col = sys_random_number() % GAME2048_SIZE;
        if (board[row][col] == 0)
        {
            board[row][col] = (rand() % 10 == 0) ? 4 : 2;
            count++;
        }
    }
}



// 检查是否可以移动
int game2048_canMove(int board[GAME2048_SIZE][GAME2048_SIZE])
{
    for (int i = 0; i < GAME2048_SIZE; i++)
    {
        for (int j = 0; j < GAME2048_SIZE; j++)
        {
            if (board[i][j] == 0) {
                return 1;
            }
            if (i < GAME2048_SIZE - 1 && board[i][j] == board[i + 1][j]) {
                return 1;
            }
            if (j < GAME2048_SIZE - 1 && board[i][j] == board[i][j + 1]) {
                return 1;
            }
        }
    }
    return 0;
}

// 合并一行
void game2048_mergeRow(int row[GAME2048_SIZE])
{
    int newRow[GAME2048_SIZE] = { 0 };
    int index = 0;
    for (int i = 0; i < GAME2048_SIZE; i++)
    {
        if (row[i] != 0) {
            newRow[index++] = row[i];
        }
    }
    for (int i = 0; i < GAME2048_SIZE - 1; i++)
    {
        if (newRow[i] == newRow[i + 1])
        {
            newRow[i] *= 2;
            newRow[i + 1] = 0;
            for (int j = i + 1; j < GAME2048_SIZE - 1; j++) {
                newRow[j] = newRow[j + 1];
            }
            newRow[GAME2048_SIZE - 1] = 0;
        }
    }
    for (int i = 0; i < GAME2048_SIZE; i++) {
        row[i] = newRow[i];
    }
}

// 向左移动
int game2048_moveLeft(int board[GAME2048_SIZE][GAME2048_SIZE])
{
    int moved = 0;
    for (int i = 0; i < GAME2048_SIZE; i++)
    {
        int temp[GAME2048_SIZE];
        for (int j = 0; j < GAME2048_SIZE; j++) {
            temp[j] = board[i][j];
        }
        game2048_mergeRow(temp);
        for (int j = 0; j < GAME2048_SIZE; j++) {
            if (board[i][j] != temp[j]) {
                moved = 1;
                break;
            }
        }
        for (int j = 0; j < GAME2048_SIZE; j++) {
            board[i][j] = temp[j];
        }
    }
    return moved;
}

// 向右移动
int game2048_moveRight(int board[GAME2048_SIZE][GAME2048_SIZE])
{
    int moved = 0;
    for (int i = 0; i < GAME2048_SIZE; i++) {
        int temp[GAME2048_SIZE];
        for (int j = 0; j < GAME2048_SIZE; j++) {
            temp[j] = board[i][GAME2048_SIZE - 1 - j];
        }
        game2048_mergeRow(temp);
        for (int j = 0; j < GAME2048_SIZE; j++) {
            if (board[i][GAME2048_SIZE - 1 - j] != temp[j]) {
                moved = 1;
                break;
            }
        }
        for (int j = 0; j < GAME2048_SIZE; j++) {
            board[i][GAME2048_SIZE - 1 - j] = temp[j];
        }
    }
    return moved;
}

// 向上移动
int game2048_moveUp(int board[GAME2048_SIZE][GAME2048_SIZE])
{
    int moved = 0;
    for (int j = 0; j < GAME2048_SIZE; j++)
    {
        int temp[GAME2048_SIZE];
        for (int i = 0; i < GAME2048_SIZE; i++) {
            temp[i] = board[i][j];
        }
        game2048_mergeRow(temp);
        for (int i = 0; i < GAME2048_SIZE; i++) {
            if (board[i][j] != temp[i]) {
                moved = 1;
                break;
            }
        }
        for (int i = 0; i < GAME2048_SIZE; i++) {
            board[i][j] = temp[i];
        }
    }
    return moved;
}

// 向下移动
int game2048_moveDown(int board[GAME2048_SIZE][GAME2048_SIZE])
{
    int moved = 0;
    for (int j = 0; j < GAME2048_SIZE; j++) {
        int temp[GAME2048_SIZE];
        for (int i = 0; i < GAME2048_SIZE; i++) {
            temp[i] = board[GAME2048_SIZE - 1 - i][j];
        }
        game2048_mergeRow(temp);
        for (int i = 0; i < GAME2048_SIZE; i++) {
            if (board[GAME2048_SIZE - 1 - i][j] != temp[i]) {
                moved = 1;
                break;
            }
        }
        for (int i = 0; i < GAME2048_SIZE; i++) {
            board[GAME2048_SIZE - 1 - i][j] = temp[i];
        }
    }
    return moved;
}

// 随机生成新数字
void game2048_generateNewNumber(int board[GAME2048_SIZE][GAME2048_SIZE])
{
    int emptyCells = 0;
    for (int i = 0; i < GAME2048_SIZE; i++)
    {
        for (int j = 0; j < GAME2048_SIZE; j++)
        {
            if (board[i][j] == 0) {
                emptyCells++;
            }
        }
    }
    if (emptyCells > 0)
    {
        int position = sys_random_number() % emptyCells;
        int count = 0;
        for (int i = 0; i < GAME2048_SIZE; i++)
        {
            for (int j = 0; j < GAME2048_SIZE; j++)
            {
                if (board[i][j] == 0)
                {
                    if (count == position) {
                        board[i][j] = (rand() % 10 == 0) ? 4 : 2;
                        return;
                    }
                    count++;
                }
            }
        }
    }
}

int game2048_getIndex(int number)
{
    static int game2048_number[32] =
    {
        0,      2,      4,      8,      16,
        32,     64,     128,    256,    512,
        1024,   2048,   4096,   8192
    };

    for(int i = 0; i < 32; i++)
    {
        if(game2048_number[i] == number)
            return i;
    }
    return 0;
}

