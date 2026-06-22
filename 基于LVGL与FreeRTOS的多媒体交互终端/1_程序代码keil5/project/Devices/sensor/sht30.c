#include "sht30.h"
#include "sensor_iic_port.h"
#include "../../system/delay/delay.h"

/**
 * @brief SHT30 初始化函数
 * @return 0: 成功, 1: 失败 (传感器未响应)
 */
uint8_t sht30_init(void)
{
    uint8_t reset_lsb = 0xA2; // 软复位指令低字节: 0x30A2
    uint8_t ret;

    // 1. 发送软复位指令 0x30A2
    // 根据修改后的 write_regs：reg = 0x30, buf = {0xA2}
    ret = sht30_write_regs(0x30, &reset_lsb, 1);
    
    if (ret != 0) {
        // 检查传感器是否在线 (I2C 地址错误或未上电)
		printf("[error] 温湿度传感器初始化失败\r\n");
        return 1; 
    }
	printf("[error] 温湿度传感器初始化成功\r\n");

    // 2. 等待复位完成 (手册建议至少 1ms，建议给 10ms 以确保稳定)
    sys_delay_ms(10); 

    // 3. 可选：清除状态寄存器 0x3041
    uint8_t clear_lsb = 0x41;
    sht30_write_regs(0x30, &clear_lsb, 1);

    return 0;
}


void sht30_get_temperature_humidity(float *t, float *h) 
{
    uint8_t cmd_lsb = 0x06;
    uint8_t raw_data[6];

    // 1. 发送测量指令: 0x2C06 (高精度模式)
    // 0x2C 传给 reg, 0x06 传给 buf
    sht30_write_regs(0x2C, &cmd_lsb, 1);

    // 2. 等待 ADC 转换 (必须手动加延时，SHT30 转换约需 15ms)
    // 这里可以使用 HAL_Delay(20) 或 osDelay(20)
    sys_delay_ms(20); 

    // 3. 读取 6 字节数据 (Temp*2 + CRC + Hum*2 + CRC)
    // 使用修改后的 read 函数，不再重复发寄存器地址
    sht30_read_regs(raw_data, 6);

    // 4. 解析数据
    uint16_t t_ticks = (raw_data[0] << 8) | raw_data[1];
    uint16_t h_ticks = (raw_data[3] << 8) | raw_data[4];

    *t = -45.0f + 175.0f * (float)t_ticks / 65535.0f;
    *h = 100.0f * (float)h_ticks / 65535.0f;
}



