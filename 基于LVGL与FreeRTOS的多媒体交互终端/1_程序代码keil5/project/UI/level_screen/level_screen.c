#include "level_screen.h"


#if USE_UI_LEVEL_SCREEN
#include <math.h>

lv_obj_t *level_screen_obj;
ui_screen_t level_screen =
{
    .init = level_screen_init,
    .deinit = level_screen_deinit,
    .obj = &level_screen_obj,
};


// --- 物理常数配置 ---
#define R               180     // 背景大圆半径 (px)
#define r               30      // 水平仪小球半径 (px)
#define MAX_ANGLE       45.0f   // 满量程角度 (倾斜45度时小球到达边缘)
#define FILTER_ALPHA    0.25f   // 滤波系数 (越小越平滑)

// --- 私有变量 ---
static lv_obj_t *level_circle;
static lv_obj_t *level_ball;

// --- 私有函数声明 ---
static void level_screen_quit_event(lv_event_t * e);
static void imu_sensor_cb(sensor_id_t id, const uint8_t* data);

/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void level_screen_init(lv_obj_t *parent)
{
    // 基础全屏容器 - 白色背景
    level_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(level_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(level_screen_obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_clear_flag(level_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 返回按钮
    lv_obj_t *level_quit_btn = lv_btn_create(level_screen_obj);
    lv_obj_align(level_quit_btn, LV_ALIGN_TOP_RIGHT, -15, 15);
    lv_obj_set_size(level_quit_btn, 65, 45);
    lv_obj_t *label_obj = lv_label_create(level_quit_btn);
    lv_obj_align(label_obj, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label_obj, "退出");
	lv_obj_set_style_text_font(label_obj, &_lvgl_font2, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(level_quit_btn, level_screen_quit_event, LV_EVENT_CLICKED, NULL);

    // 创建外围大圆轨道
    level_circle = lv_obj_create(level_screen_obj);
    lv_obj_set_size(level_circle, R * 2, R * 2);
    lv_obj_center(level_circle);
    lv_obj_set_style_radius(level_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(level_circle, lv_color_hex(0xFAFAFA), 0);
    lv_obj_set_style_border_width(level_circle, 2, 0);
    lv_obj_set_style_border_color(level_circle, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_pad_all(level_circle, 0, 0);
    lv_obj_clear_flag(level_circle, LV_OBJ_FLAG_SCROLLABLE);

    // 绘制十字参考线
    static lv_point_t line_h[] = {{0, R}, {R * 2, R}};
    static lv_point_t line_v[] = {{R, 0}, {R, R * 2}};
    lv_obj_t *lh = lv_line_create(level_circle);
    lv_line_set_points(lh, line_h, 2);
    lv_obj_set_style_line_color(lh, lv_color_hex(0xCCCCCC), 0);
    lv_obj_t *lv = lv_line_create(level_circle);
    lv_line_set_points(lv, line_v, 2);
    lv_obj_set_style_line_color(lv, lv_color_hex(0xCCCCCC), 0);

    // 创建中心指示小球
    level_ball = lv_obj_create(level_circle);
    lv_obj_set_size(level_ball, r * 2, r * 2);
    lv_obj_set_style_radius(level_ball, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(level_ball, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_border_width(level_ball, 0, 0);
    lv_obj_set_style_shadow_width(level_ball, 15, 0);
    lv_obj_set_style_shadow_color(level_ball, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_shadow_opa(level_ball, LV_OPA_30, 0);
    lv_obj_clear_flag(level_ball, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(level_ball, R - r, R - r);

    // 通过 SensorHub 订阅 IMU 数据
    // 注意：订阅后，SensorTask 会进入高频更新模式（如20-50ms）
    SensorHub.subscribe(SENSOR_IMU, imu_sensor_cb);
}

void level_screen_deinit(void)
{
    SensorHub.unsubscribe(SENSOR_IMU, imu_sensor_cb);
    
    if (level_screen_obj)
    {
        lv_obj_del(level_screen_obj);
        level_screen_obj = NULL;
    }
}


/*-------------------------------------------------------------
                        回调处理函数
--------------------------------------------------------------*/

static void level_screen_quit_event(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        ui_screen_back();
    }
}

/**
 * @brief IMU 数据更新回调（由 SensorHub 推送）
 */
static void imu_sensor_cb(sensor_id_t id, const uint8_t* data)
{
    if (id != SENSOR_IMU || data == NULL || level_ball == NULL) return;

    // 假设 sensor_imu_data_t 结构体包含 pitch, roll, yaw
    sensor_imu_data_t* imu = (sensor_imu_data_t*)data;
    
    static float smooth_x = 0.0f;
    static float smooth_y = 0.0f;

    float pitch = imu->pitch;
    float roll  = imu->roll;

    float L = (float)(R - r) - 3.0f;

    // 1. 角度映射：Roll 对应 X 轴移动，Pitch 对应 Y 轴移动
    float target_x = (roll / MAX_ANGLE) * L;
    float target_y = (pitch / MAX_ANGLE) * L;

    // 2. 边界限幅 (勾股定理，确保小球在大圆内部)
    float current_dist = sqrtf(target_x * target_x + target_y * target_y);
    if (current_dist > L)
    {
        float angle_rad = atan2f(target_y, target_x);
        target_x = cosf(angle_rad) * L;
        target_y = sinf(angle_rad) * L;
    }

    // 3. 一阶低通滤波，减少抖动
    smooth_x = (target_x * FILTER_ALPHA) + (smooth_x * (1.0f - FILTER_ALPHA));
    smooth_y = (target_y * FILTER_ALPHA) + (smooth_y * (1.0f - FILTER_ALPHA));

    // 4. 坐标转换 (相对于 level_circle 父对象的坐标)
    int final_x = (int)(R + smooth_x - r);
    int final_y = (int)(R + smooth_y - r);

    lv_obj_set_pos(level_ball, final_x, final_y);

    // 5. 状态反馈：接近水平时变绿，否则保持蓝色
    if (fabsf(pitch) < 3.0f && fabsf(roll) < 3.5f)
    {
        lv_obj_set_style_bg_color(level_ball, lv_palette_main(LV_PALETTE_GREEN), 0);
    }
    else
    {
        lv_obj_set_style_bg_color(level_ball, lv_palette_main(LV_PALETTE_BLUE), 0);
    }
}

#else
ui_screen_t level_screen =
{
    .init = NULL,
    .deinit = NULL,
    .obj = NULL,
};
#endif

