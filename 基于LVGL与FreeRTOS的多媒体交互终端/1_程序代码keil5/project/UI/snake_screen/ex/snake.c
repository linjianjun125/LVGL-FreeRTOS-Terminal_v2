#include "snake.h"
#include "devices_include.h"

void snake_initSnake(Snake* snake) {
    snake->length = 3;
    snake->direction = 1; 
    snake->isover = false;
    snake->stop = false;
    for (int i = 0; i < snake->length; i++) {
        snake->x[i] = SNAKE_WIDTH / 2 - i;
        snake->y[i] = SNAKE_HEIGHT / 2;
    }
    snake_initFood(snake);
}

void snake_initFood(Snake *snake) {
    bool collision;
    do {
        collision = false;
        snake->food.x = sys_random_number() % SNAKE_WIDTH;
        snake->food.y = sys_random_number() % SNAKE_HEIGHT;

        // 确保食物不生成在蛇头上或身体上
        for (int k = 0; k < snake->length; k++) {
            if (snake->food.x == snake->x[k] && snake->food.y == snake->y[k]) {
                collision = true;
                break;
            }
        }
    } while (collision); // 如果碰撞了，重新随机生成
}


void snake_move(Snake* snake) {
    if(snake->stop || snake->isover) return;

	// 记录移动前的蛇尾坐标，防止增长时闪烁 (0,0)
    int old_tail_x = snake->x[snake->length - 1];
    int old_tail_y = snake->y[snake->length - 1];
	
	
    // 身体移动
    for (int i = snake->length - 1; i > 0; i--) {
        snake->x[i] = snake->x[i - 1];
        snake->y[i] = snake->y[i - 1];
    }

    int next_x = snake->x[0];
    int next_y = snake->y[0];

    switch (snake->direction) {
        case 1: next_x++; break;
        case 2: next_y++; break;
        case 3: next_x--; break;
        case 4: next_y--; break;
    }

    // --- 穿墙逻辑处理 ---
#if SNAKE_CAN_THROUGH_WALL
    if(next_x >= SNAKE_WIDTH) next_x = 0;
    else if(next_x < 0)       next_x = SNAKE_WIDTH - 1;
    if(next_y >= SNAKE_HEIGHT) next_y = 0;
    else if(next_y < 0)        next_y = SNAKE_HEIGHT - 1;
#else
    if(next_x >= SNAKE_WIDTH || next_x < 0 || next_y >= SNAKE_HEIGHT || next_y < 0) {
        snake->isover = true;
        return;
    }
#endif

    snake->x[0] = next_x;
    snake->y[0] = next_y;

    // 吃到食物
    if (snake->x[0] == snake->food.x && snake->y[0] == snake->food.y) {
        // 先把旧尾部坐标给到新节点，再增加长度
        snake->x[snake->length] = old_tail_x;
        snake->y[snake->length] = old_tail_y;
        snake->length++; 
        
        snake_initFood(snake);
    }

    // 撞到自己
    for (int i = 1; i < snake->length; i++) {
        if (snake->x[0] == snake->x[i] && snake->y[0] == snake->y[i]) {
            snake->isover = true;
        }
    }
}

bool snake_isGameOver(Snake* snake) { return snake->isover; }

void snake_input(Snake* snake, int input) {
    if(snake->stop) return;
    if (input == SNAKE_UP && snake->direction != 2)    snake->direction = 4;
    else if (input == SNAKE_DOWN && snake->direction != 4) snake->direction = 2;
    else if (input == SNAKE_LEFT && snake->direction != 1) snake->direction = 3;
    else if (input == SNAKE_RIGHT && snake->direction != 3) snake->direction = 1;
}
