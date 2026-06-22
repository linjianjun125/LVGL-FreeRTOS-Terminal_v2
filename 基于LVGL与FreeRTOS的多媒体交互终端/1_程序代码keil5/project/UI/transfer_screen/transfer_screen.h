#ifndef __UI_APP_TRANSFER_SCREEN_H
#define __UI_APP_TRANSFER_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../ui/screen.h"
#include <stdint.h>

#if USE_UI_TRANSFER_SCREEN
/**
 * @brief 传输状态枚举定义
 */
typedef enum {
    TRANSFER_STATUS_CONNECTING,   // 正在连接 (Waiting for device)
    TRANSFER_STATUS_PROCESSING,   // 正在传输 (Transferring with progress)
    TRANSFER_STATUS_SUCCESS,      // 传输成功 (Success!)
    TRANSFER_STATUS_FAILED        // 传输失败 (Transfer Failed)
} transfer_status_t;

/**
 * @brief 界面初始化
 */
void transfer_screen_init(lv_obj_t *parent);

/**
 * @brief 界面资源卸载
 */
void transfer_screen_deinit(void);

/**
 * @brief 外部更新接口：用于从底层通信逻辑同步进度到 UI
 * @param status 当前状态枚举
 * @param progress 进度百分比 (0-100)
 */
void transfer_update_ui(transfer_status_t status, uint8_t progress);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __UI_APP_TRANSFER_SCREEN_H */
