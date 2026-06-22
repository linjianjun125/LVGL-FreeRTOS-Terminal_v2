#include "led.h"



void led_init(void)
{
    // 2. 初始化 LED0
    config_gpio_init(LED0_GPIO_PARAMS, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_MEDIUM);

    // 3. 初始化 LED1
  //  config_gpio_init(LED1_GPIO_PARAMS, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_MEDIUM);

    led0(1);      
   // led1(0);         
}


// --- LED0 专用接口 ---

// 控制 LED0 电平
void led0(uint8_t x)
{
    HAL_GPIO_WritePin(CFG_GET_PORT_PIN(LED0_GPIO_PARAMS), x ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 翻转 LED0 状态
void led0_toggle(void)
{
    HAL_GPIO_TogglePin(CFG_GET_PORT_PIN(LED0_GPIO_PARAMS));
}

// --- LED1 专用接口 ---

// 控制 LED1 电平
void led1(uint8_t x)
{
    HAL_GPIO_WritePin(CFG_GET_PORT_PIN(LED1_GPIO_PARAMS), x ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 翻转 LED1 状态
void led1_toggle(void)
{
    HAL_GPIO_TogglePin(CFG_GET_PORT_PIN(LED1_GPIO_PARAMS));
}

