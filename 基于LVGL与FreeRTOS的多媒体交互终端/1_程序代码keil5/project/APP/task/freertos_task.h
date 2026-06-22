#ifndef __FREERTOS_TASK_H
#define __FREERTOS_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// =================================================================
//                            任务句柄声明
// =================================================================
extern TaskHandle_t xMusicTaskHandle;      // 音乐播放任务句柄
extern TaskHandle_t vVolumeTaskHandle;     // 声音/音量任务句柄
extern TaskHandle_t xKeyTaskHandle;        // 按键扫描任务句柄
extern TaskHandle_t xBacklightTaskHandle;  // 背光管理任务句柄
extern TaskHandle_t xRTCTaskHandle;        // RTC 时间任务句柄
extern TaskHandle_t xSensorTaskHandle;     // 传感器采集任务句柄
extern TaskHandle_t xUITaskHandle;         // UI (LVGL) 任务句柄
extern TaskHandle_t xIMUTaskHandle;
extern TaskHandle_t xWifiAppaskHandle;
extern TaskHandle_t xWifiParserTaskHandle;

// =================================================================
//                            同步资源声明
// =================================================================
extern SemaphoreHandle_t es8388_Semaphore;  // ES8388 I2C 访问互斥锁
extern SemaphoreHandle_t sdmmc_Semaphore;   // SDMMC 递归互斥锁




// =================================================================
//                            任务函数入口
// =================================================================
/**
 * @brief 音乐播放后台任务：处理解码、切歌、模式切换
 */
void vMusicTask(void *pvParameters);

/**
 * @brief 声音/音量处理后台任务：处理影子变量同步及硬件 Codec 增益
 */
void vVolumeTask(void *pvParameters);

/**
 * @brief 按键扫描任务：20ms 周期扫描，支持 SensorHub 事件分发
 */
void vKeyTask(void *pvParameters);

/**
 * @brief 背光管理任务：处理自动熄屏计时、平滑亮度调节
 */
void vBacklightTask(void *pvParameters);

/**
 * @brief RTC 时间采集任务：1秒周期推送最新时间快照
 */
void vRTCTask(void *pvParameters);

/**
 * @brief 环境传感器任务：支持动态频率切换（按需采集）
 */
void vSensorTask(void *pvParameters);

/**
 * @brief 六轴传感器任务：支持动态频率切换（按需采集）
 */
void vIMUTask(void *pvParameters);

/**
 * @brief UI 渲染任务：运行 LVGL 核心循环及界面逻辑
 */
void vUITask(void *pvParameters);

/**
 * @brief 系统引导加载任务：在开机动画期间异步初始化耗时硬件
 */
void vSystemLoaderTask(void *pvParameters);


/**
 * @brief wifi任务
 */
void vWifiAppTask(void *pvParameters);

/**
 * @brief wifi解析任务
 */
void vWifiParserTask(void *pvParameters);

#endif // __FREERTOS_TASK_H

