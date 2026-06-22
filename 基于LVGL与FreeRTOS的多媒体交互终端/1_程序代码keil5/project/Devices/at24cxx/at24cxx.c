#include "at24cxx.h"
#include "at24cxx_iic.h"
#include "../../system/delay/delay.h"
#include <stdio.h>
#include <string.h>

// 根据丝印 ATMLH512 识别，该芯片为 AT24C512，页大小为 128 字节
#define AT24C512_PAGE_SIZE    128
#define AT24CXX_IIC_ADDR      0x50 // 7位地址

/**
 * @brief  底层写入寄存器
 */
uint8_t at24cxx_write_regs(uint16_t reg, uint8_t *buf, uint16_t len)
{
    at24cxx_iic_start();
    at24cxx_iic_send_byte((AT24CXX_IIC_ADDR << 1) | 0); // 写命令
    if (at24cxx_iic_wait_ack()) { at24cxx_iic_stop(); return 1; }
    
    at24cxx_iic_send_byte((uint8_t)(reg >> 8));   // 高位地址
    at24cxx_iic_wait_ack();
    at24cxx_iic_send_byte((uint8_t)(reg & 0xFF)); // 低位地址
    at24cxx_iic_wait_ack();

    for (uint16_t i = 0; i < len; i++) {
        at24cxx_iic_send_byte(buf[i]);
        if (at24cxx_iic_wait_ack()) break;
    }
    at24cxx_iic_stop();
    return 0;
}

/**
 * @brief  底层读取寄存器
 */
void at24cxx_read_regs(uint16_t reg, uint8_t *buf, uint16_t len)
{
    at24cxx_iic_start();
    at24cxx_iic_send_byte((AT24CXX_IIC_ADDR << 1) | 0);
    at24cxx_iic_wait_ack();
    
    at24cxx_iic_send_byte((uint8_t)(reg >> 8));
    at24cxx_iic_wait_ack();
    at24cxx_iic_send_byte((uint8_t)(reg & 0xFF));
    at24cxx_iic_wait_ack();
    
    at24cxx_iic_start();
    at24cxx_iic_send_byte((AT24CXX_IIC_ADDR << 1) | 1); // 读命令
    at24cxx_iic_wait_ack();

    for (uint16_t i = 0; i < len; i++) {
        buf[i] = at24cxx_iic_read_byte(i == (len - 1) ? 0 : 1);
    }
    at24cxx_iic_stop();
}

/**
 * @brief  跨页写入数据
 */
void at24cxx_write_data(uint16_t addr, uint8_t *pbuf, uint16_t len)
{
    uint16_t page_size = AT24C512_PAGE_SIZE; 
    uint16_t first_page_bytes = page_size - (addr % page_size);
	int datalen = len;

    if (datalen <= first_page_bytes) {
        at24cxx_write_regs(addr, pbuf, datalen);
        sys_delay_ms(10);
        return;
    }

    at24cxx_write_regs(addr, pbuf, first_page_bytes);
    sys_delay_ms(10);
    
    addr += first_page_bytes;
    pbuf += first_page_bytes;
    datalen -= first_page_bytes;

    while (datalen > 0) {
        uint16_t write_size = (datalen >= page_size) ? page_size : datalen;
        at24cxx_write_regs(addr, pbuf, write_size);
        sys_delay_ms(10);
        addr += write_size;
        pbuf += write_size;
        datalen -= write_size;
    }
}

void at24cxx_read_data(uint16_t addr, uint8_t *pbuf, uint16_t datalen)
{
    // 读取不涉及页限制，可以直接调用
    at24cxx_read_regs(addr, pbuf, datalen);
}

/**
 * @brief  探测容量并校验
 */
uint32_t at24cxx_check_capacity_and_integrity(void)
{
    // 探测点从 64KB 向下探测更准确
    uint32_t test_caps[] = {65536, 32768, 16384, 8192, 4096, 2048, 1024, 512, 256};
    uint8_t backup_0, test_pattern = 0x5A;
    uint32_t detected_size = 0;

    printf("\r\n[Check] Testing EEPROM Capacity...\n");

    at24cxx_read_data(0, &backup_0, 1);

    for (uint8_t i = 0; i < sizeof(test_caps)/sizeof(test_caps[0]); i++) {
        uint32_t addr = test_caps[i];
        uint8_t unique_val = (uint8_t)(0xAA + i);
        uint8_t read_back;

        at24cxx_write_data(0, &test_pattern, 1);
        // 这里强制转换 uint16_t 模拟硬件溢出回环
        at24cxx_write_data((uint16_t)addr, &unique_val, 1);
        at24cxx_read_data(0, &read_back, 1);

        if (read_back == unique_val) {
            detected_size = addr;
            break;
        }
    }

    at24cxx_write_data(0, &backup_0, 1); // 恢复现场

    if (detected_size == 0) {
        printf("[Error] Chip not found or size unknown.\n");
        return 0;
    }

    printf("[Success] Detected: %u Bytes (%u Kbit)\n", detected_size, detected_size * 8 / 1024);

    // 全片校验（仅建议量产测试使用）
    printf("[Check] Integrity Checking... (Wait ~10s)\n");
    uint8_t w_buf[64], r_buf[64];
    memset(w_buf, 0xA5, 64);
    for (uint32_t a = 0; a < detected_size; a += 64) {
        at24cxx_write_data(a, w_buf, 64);
        at24cxx_read_data(a, r_buf, 64);
        if (memcmp(w_buf, r_buf, 64) != 0) {
            printf("[Fail] Bad block at 0x%04X\n", a);
            return 0;
        }
    }
    printf("[Success] Integrity Passed.\n");
    return detected_size;
}

void at24cxx_init(void)
{
    at24cxx_iic_init();
    
    // 1. 简单读写测试
    uint8_t test_w[2] = {0xEE, 0xFF}, test_r[2];
    at24cxx_write_data(0x00, test_w, 2);
    at24cxx_read_data(0x00, test_r, 2);

    if (test_r[0] == 0xEE && test_r[1] == 0xFF) {
        printf("[Info] AT24CXX Basic Communication OK.\n");
        // 2. 探测实际容量
        //at24cxx_check_capacity_and_integrity();
    } else {
        printf("[Error] AT24CXX Comm Failed. Check IIC or Addr.\n");
    }
}

