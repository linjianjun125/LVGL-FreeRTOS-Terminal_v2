#include "icm42688.h"
#include "sensor_spi_port.h"
#include "../../system/delay/delay.h"
#include <math.h>




void icm42688_init(void)
{
	
	
    uint8_t res;
    uint8_t config_val;
    
	icm42688_spi_cs_init();
	
	
    // 1. 软件复位传感器
    config_val = 0x01; 
    icm42688_write_regs(ICM42688_DEVICE_CONFIG_REG, &config_val, 1);
    sys_delay_ms(100); 
    
    // 2. 电源管理：开启陀螺仪和加速度计到 LN (Low Noise) 模式
    config_val = 0x0F; 
    icm42688_write_regs(ICM42688_PWR_MGMT0_REG, &config_val, 1);
    sys_delay_ms(10);

    // 3. 配置陀螺仪量程与速率
    config_val = ICM42688_GYRO_FS_2000DPS | ICM42688_GYRO_ODR_1KHZ;
    icm42688_write_regs(ICM42688_GYRO_CONFIG0_REG, &config_val, 1);

    // 4. 配置加速度计量程与速率
    config_val = ICM42688_ACCEL_FS_16G | ICM42688_ACCEL_ODR_1KHZ;
    icm42688_write_regs(ICM42688_ACCEL_CONFIG0_REG, &config_val, 1);
    
    // 5. 读取 ID 验证通讯是否正常
    icm42688_read_regs(ICM42688_ID_REG, &res, 1);
    if(res == 0x47) {
        printf("[info] ICM42688 Init Success, ID: 0x%02X\n", res);
    } else {
        printf("[error] ICM42688 Init Failed, ID: 0x%02X (Expected 0x47)\n", res);
    }
}





/**
 * @brief  读取 6 轴加速度原始数据
 */
uint8_t icm42688_get_accelerometer(short *ax, short *ay, short *az)
{
    uint8_t buf[6];
    // 连续读取从 XH 开始的 6 个字节
    uint8_t status = icm42688_read_regs(ICM42688_ACCEL_DATA_X1_REG, buf, 6);

    // 拼接 16 位有符号整数 (Big-Endian)
    *ax = (short)((buf[0] << 8) | buf[1]);
    *ay = (short)((buf[2] << 8) | buf[3]);
    *az = (short)((buf[4] << 8) | buf[5]);
    
    return status;
}

/**
 * @brief  读取 6 轴陀螺仪原始数据
 */
uint8_t icm42688_get_gyroscope(short *gx, short *gy, short *gz)
{
    uint8_t buf[6];
    uint8_t status = icm42688_read_regs(ICM42688_GYRO_DATA_X1_REG, buf, 6);

    *gx = (short)((buf[0] << 8) | buf[1]);
    *gy = (short)((buf[2] << 8) | buf[3]);
    *gz = (short)((buf[4] << 8) | buf[5]);
    
    return status;
}

