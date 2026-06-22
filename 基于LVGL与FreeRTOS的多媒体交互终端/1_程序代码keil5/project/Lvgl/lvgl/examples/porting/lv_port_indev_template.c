/**
 * @file lv_port_indev_templ.c
 *
 */

 /*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev_template.h"
#include "../../lvgl.h"

#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"


static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);


void lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;
	
    /*Initialize your touchpad if you have*/
    lcd_touch_init();

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);
}



static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    /*Save the pressed coordinates and the state*/
    if(lcd_touch_scan(&last_x, &last_y)) 
	{
        data->state = LV_INDEV_STATE_PR;
		BacklightControl.wakeup();
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}



#else /*Enable this file at the top*/
/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
