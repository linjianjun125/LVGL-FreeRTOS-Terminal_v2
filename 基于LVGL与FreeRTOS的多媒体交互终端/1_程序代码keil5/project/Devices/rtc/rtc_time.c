#include "rtc_time.h"
#include "../../system/debug/debug.h"


/**
 * @brief    获取当前时间
 * @param    hour: 小时指针 (BCD转十进制后)
 * @param    min : 分钟指针 (BCD转十进制后)
 * @param    sec : 秒钟指针 (BCD转十进制后)
 * @param    ampm: 上下午标志指针 (0=AM/24H, 1=PM)
 * @retval   无
 */
void rtc_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *ampm)
{
    uint32_t temp = 0;

    while (rtc_wait_synchro());             // 等待影子寄存器同步

    temp = RTC->TR;                         // 读取时间寄存器
    *hour = rtc_bcd2dec((temp >> 16) & 0X3F); // 提取并转换小时
    *min = rtc_bcd2dec((temp >> 8) & 0X7F);   // 提取并转换分钟
    *sec = rtc_bcd2dec(temp & 0X7F);          // 提取并转换秒钟
    *ampm = temp >> 22;                       // 提取AM/PM标志位
}

/**
 * @brief    获取当前日期
 * @param    year : 年份指针 (BCD转十进制后, 0-99)
 * @param    month: 月份指针 (BCD转十进制后)
 * @param    date : 日期指针 (BCD转十进制后)
 * @param    week : 星期指针 (1-7)
 * @retval   无
 */
void rtc_get_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week)
{
    uint32_t temp = 0;

    while (rtc_wait_synchro());             // 等待影子寄存器同步

    temp = RTC->DR;                         // 读取日期寄存器
    *year = rtc_bcd2dec((temp >> 16) & 0XFF);  // 提取并转换年份
    *month = rtc_bcd2dec((temp >> 8) & 0X1F); // 提取并转换月份
    *date = rtc_bcd2dec(temp & 0X3F);          // 提取并转换日期
    *week = (temp >> 13) & 0X07;               // 提取星期
}

/**
 * @brief    设置RTC时间
 * @param    hour: 小时 (十进制)
 * @param    min : 分钟 (十进制)
 * @param    sec : 秒钟 (十进制)
 * @param    ampm: 上下午标志 (0=AM/24H, 1=PM)
 * @retval   0,成功; 1,失败
 */
uint8_t rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec, uint8_t ampm)
{
    uint32_t temp = 0;

    RTC->WPR = 0xCA;                        // 解除写保护关键序列1
    RTC->WPR = 0x53;                        // 解除写保护关键序列2
    
    if (rtc_init_mode())return 1;           // 进入初始化模式

    // 将十进制参数转换为BCD并按位域拼接至临时变量
    temp = (((uint32_t)ampm & 0X01) << 22) | 
           ((uint32_t)rtc_dec2bcd(hour) << 16) |
           ((uint32_t)rtc_dec2bcd(min) << 8) | 
           (rtc_dec2bcd(sec));

    RTC->TR = temp;                         // 写入时间寄存器
    RTC->ISR &= ~(1 << 7);                  // 退出初始化模式
    RTC->WPR = 0xFF;                        // 重新启用写保护
    
    return 0;
}

/**
 * @brief    设置RTC日期
 * @param    year : 年份 (十进制, 0-99)
 * @param    month: 月份 (十进制)
 * @param    date : 日期 (十进制)
 * @param    week : 星期 (1-7)
 * @retval   0,成功; 1,失败
 */
uint8_t rtc_set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t week)
{
    uint32_t temp = 0;

    RTC->WPR = 0xCA;                        // 解除写保护关键序列1
    RTC->WPR = 0x53;                        // 解除写保护关键序列2
    
    if (rtc_init_mode())return 1;           // 进入初始化模式

    // 将十进制参数转换为BCD并按位域拼接至临时变量
    temp = (((uint32_t)week & 0X07) << 13) | 
           ((uint32_t)rtc_dec2bcd(year) << 16) |
           ((uint32_t)rtc_dec2bcd(month) << 8) | 
           (rtc_dec2bcd(date));

    RTC->DR = temp;                         // 写入日期寄存器
    RTC->ISR &= ~(1 << 7);                  // 退出初始化模式
    RTC->WPR = 0xFF;                        // 重新启用写保护

    return 0;
}

