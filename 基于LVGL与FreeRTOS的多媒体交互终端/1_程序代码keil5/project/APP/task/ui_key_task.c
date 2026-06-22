#include "freertos_task.h"

#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"



bool __shutdown_msgbox_create_flag = false;
void ui_create_opening_animation(void);
void ui_destroy_opening_animation(void);
void ui_update_opening_progress(int progress);
void ui_notify_progress_update(uint8_t progress);
uint8_t ui_get_progress_blocked(TickType_t xTicksToWait);

void vUITask(void *pvParameters)
{
	 sensor_hub_msg_t msg;
    // --- 1. 立即显示开机动画 ---
    ui_create_opening_animation(); 
    
    // --- 2. 亮屏 (此时用户看到动画开始播放) ---
    lcdrgb_set_pwm_duty_cycle(30);
	
    // --- 3. 循环刷新动画，同时检测后台加载状态 ---
    while(1)
    {

        // 等待初始化任务的通知，最多等待 100ms
        uint8_t progress = ui_get_progress_blocked(pdMS_TO_TICKS(100));

        // 更新进度条和文字
        ui_update_opening_progress(progress);

        if (progress >= 100)
        {
            // 加载完成，销毁动画，进入主界面
            vTaskDelay(pdMS_TO_TICKS(500)); // 稍作停顿增加视觉舒适度
            ui_destroy_opening_animation();
            break; 
        }
        
			// 正常的 LVGL 处理
			lv_timer_handler();
		}


    // --- 4. 进入正常的主业务 UI 循环 ---
	ui_screen_init(10, &music_screen);
    while(1)
    {
	    // 处理所有上报的数据（包含温湿度、音量反馈、RTC 等）
		while (SensorHub.pop(&msg)) 
        {
            SensorHub.dispatch(&msg); 
        }
		if(__shutdown_msgbox_create_flag)
		{
			__shutdown_msgbox_create_flag = false;
			top_screen_create_shutdown_msgbox(NULL);
		}
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}


/**
 * @brief 按键扫描任务
 */
void vKeyTask(void *pvParameters) 
{
    uint8_t key_val;
    while (1) 
    {
		key_val = key_scan(KEY_MODE_SINGLE);
        if (key_val) 
        {
            if (!(SensorHub.get_active_mask() & SENSOR_KEY)) 
            {
                if (key_val == KEY_UP){ VolumeControl.step_up();}
                else if (key_val == KEY_DOWN){ VolumeControl.step_down();}
				else if(key_val == KEY_POWER){__shutdown_msgbox_create_flag = true;}
            } 
            else 
            {
                SensorHub.push(SENSOR_KEY, &key_val);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}




/*-------------------------------------------------

					开机动画
---------------------------------------------------*/




static lv_obj_t * opening_scr;        // 开机动画专用容器
static lv_obj_t * opening_bar;        // 进度条对象
static lv_obj_t * opening_label;      // 进度百分比文字
static lv_obj_t * opening_logo;       // Logo文字
static volatile uint8_t ui_loading_progress = 0; // 进度缓存 (0-100)

void ui_create_opening_animation(void)
{
    // 1. 创建全屏黑色/白色背景容器
    opening_scr = lv_obj_create(lv_scr_act());
    lv_obj_set_size(opening_scr, 480, 480);
    lv_obj_center(opening_scr);
    lv_obj_set_style_bg_color(opening_scr, lv_color_hex(0xFFFFFF), 0); // 纯白背景
    lv_obj_set_style_border_width(opening_scr, 0, 0);
    lv_obj_set_style_radius(opening_scr, 0, 0);
    lv_obj_clear_flag(opening_scr, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 创建 Logo (居中)
    opening_logo = lv_label_create(opening_scr);
    lv_label_set_text(opening_logo, "INIT SYSTEM！");
    lv_obj_set_style_text_font(opening_logo, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(opening_logo, lv_color_hex(0x95A5A6), 0); // 深色质感文字
    lv_obj_align(opening_logo, LV_ALIGN_CENTER, 0, -40);

    // 3. 创建进度条 (细长简约风)
    opening_bar = lv_bar_create(opening_scr);
    lv_obj_set_size(opening_bar, 300, 6); // 细条设计
    lv_obj_align(opening_bar, LV_ALIGN_CENTER, 0, 50);
    lv_bar_set_range(opening_bar, 0, 100);
    lv_bar_set_value(opening_bar, 0, LV_ANIM_OFF);

    // 样式设置：进度条背景颜色
    lv_obj_set_style_bg_color(opening_bar, lv_color_hex(0xEEEEEE), LV_PART_MAIN);
    // 样式设置：进度颜色
    lv_obj_set_style_bg_color(opening_bar, lv_color_hex(0x2ECC71), LV_PART_INDICATOR);
    lv_obj_set_style_radius(opening_bar, 10, 0);

    // 4. 创建进度百分比标签
    opening_label = lv_label_create(opening_scr);
    lv_label_set_text(opening_label, "0%");
    lv_obj_set_style_text_color(opening_label, lv_color_hex(0x95A5A6), 0);
    lv_obj_align_to(opening_label, opening_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}
/**
 * @brief 销毁开机动画并释放相关内存
 */
void ui_destroy_opening_animation(void)
{
    // 1. 安全检查：如果容器已经不存在，直接返回
    if (opening_scr == NULL) {
        return;
    }

    // 3. 删除父级容器
    lv_obj_del(opening_scr);

    // 4. 指针复位（极其重要，防止 ui_update 函数发生空指针解引用）
    opening_scr = NULL;
    opening_bar = NULL;
    opening_label = NULL;
    opening_logo = NULL;

    // 5. 强制刷新一次显示，确保屏幕干净地进入下一阶段
    lv_refr_now(NULL);
    
    // printf("Opening animation destroyed, memory recovered.\n");
}
/**
 * @brief 更新开机动画进度
 * @param progress 0-100 的整数
 */
void ui_update_opening_progress(int progress)
{
    if (opening_bar == NULL || opening_label == NULL) return;

    if (progress > 100) progress = 100;
    if (progress < 0) progress = 0;

    // 更新进度条数值
    lv_bar_set_value(opening_bar, progress, LV_ANIM_ON);

    // 更新文字百分比
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", progress);
    lv_label_set_text(opening_label, buf);

}


/**
 * @brief  向 UI 任务发送进度更新通知
 * @param  progress: 进度值 (0-100)
 */
void ui_notify_progress_update(uint8_t progress)
{
    if (xUITaskHandle == NULL) return;

    // 1. 更新全局进度变量
    ui_loading_progress = progress;

    // 2. 发送任务通知
    // 使用 eSetBits 模式，或者简单的 eNoAction 仅唤醒任务
    // 这里我们使用 eNoAction，因为进度值已经通过全局变量传递
    xTaskNotify(xUITaskHandle, 0, eNoAction);
}

/**
 * @brief  UI 任务获取当前的进度值 (阻塞等待)
 * @param  xTicksToWait: 等待超时时间
 * @return uint8_t: 返回最新的进度值
 */
uint8_t ui_get_progress_blocked(TickType_t xTicksToWait)
{
    // 阻塞等待通知。ulTaskNotifyTake(pdTRUE, ...) 模拟二值信号量
    // 如果收到通知，返回 1；如果超时，返回 0
    uint32_t notified = ulTaskNotifyTake(pdTRUE, xTicksToWait);

    if (notified > 0)
    {
        // 收到通知，返回最新的全局进度
        return ui_loading_progress;
    }

    // 超时或未收到通知，返回当前的进度
    return ui_loading_progress;
}
