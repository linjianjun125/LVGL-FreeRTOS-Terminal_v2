#ifndef __ICM42688_H
#define __ICM42688_H

#include "sys.h"




// --- 核心寄存器定义 ---
#define ICM42688_ID_REG                 0x75    // WHO_AM_I (应返回 0x47)
#define ICM42688_DEVICE_CONFIG_REG      0x11    // 包含软件复位
#define ICM42688_PWR_MGMT0_REG          0x4E    // 电源管理 (控制传感器开关)
#define ICM42688_GYRO_CONFIG0_REG       0x4F    // 陀螺仪配置
#define ICM42688_ACCEL_CONFIG0_REG      0x50    // 加速度计配置

// --- 数据寄存器地址 (高位在前) ---
#define ICM42688_ACCEL_DATA_X1_REG      0x1F    
#define ICM42688_GYRO_DATA_X1_REG       0x25    

// --- 陀螺仪配置宏 (GYRO_CONFIG0) ---
// 满量程 FS_SEL [7:5]
#define ICM42688_GYRO_FS_2000DPS        (0 << 5) // 灵敏度: 16.4 LSB/dps
#define ICM42688_GYRO_FS_1000DPS        (1 << 5) // 灵敏度: 32.8 LSB/dps
#define ICM42688_GYRO_FS_500DPS         (2 << 5) // 灵敏度: 65.5 LSB/dps
#define ICM42688_GYRO_FS_250DPS         (3 << 5) // 灵敏度: 131 LSB/dps
#define ICM42688_GYRO_FS_125DPS         (4 << 5)
#define ICM42688_GYRO_FS_62_5DPS        (5 << 5)
#define ICM42688_GYRO_FS_31_25DPS       (6 << 5)
#define ICM42688_GYRO_FS_15_625DPS      (7 << 5)
// 输出速率 ODR [3:0]
#define ICM42688_GYRO_ODR_32KHZ         (1 << 0)
#define ICM42688_GYRO_ODR_16KHZ         (2 << 0)
#define ICM42688_GYRO_ODR_1KHZ          (6 << 0)
#define ICM42688_GYRO_ODR_200HZ         (7 << 0)
#define ICM42688_GYRO_ODR_100HZ         (8 << 0)
#define ICM42688_GYRO_ODR_50HZ          (9 << 0)

// --- 加速度计配置宏 (ACCEL_CONFIG0) ---
// 满量程 FS_SEL [7:5]
#define ICM42688_ACCEL_FS_16G           (0 << 5) // 灵敏度: 2048 LSB/g
#define ICM42688_ACCEL_FS_8G            (1 << 5) // 灵敏度: 4096 LSB/g
#define ICM42688_ACCEL_FS_4G            (2 << 5) // 灵敏度: 8192 LSB/g
#define ICM42688_ACCEL_FS_2G            (3 << 5) // 灵敏度: 16384 LSB/g
// 输出速率 ODR [3:0]
#define ICM42688_ACCEL_ODR_32KHZ        (1 << 0)
#define ICM42688_ACCEL_ODR_1KHZ         (6 << 0)
#define ICM42688_ACCEL_ODR_200HZ        (7 << 0)
#define ICM42688_ACCEL_ODR_100HZ        (8 << 0)

void icm42688_init(void);
uint8_t icm42688_get_accelerometer(short *ax, short *ay, short *az);
uint8_t icm42688_get_gyroscope(short *gx, short *gy, short *gz);


#endif




