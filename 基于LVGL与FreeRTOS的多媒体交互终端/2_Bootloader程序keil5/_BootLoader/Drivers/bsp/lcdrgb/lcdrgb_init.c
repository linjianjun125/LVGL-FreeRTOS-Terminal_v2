#include "lcdrgb_init.h"
#include "../../system/delay/delay.h"




/**
 * @brief       SPI初始化LCD
 * @param       无
 * @retval      无
 */
void lcdrgb_param_init(void)
{

	LCD_INIT_GPIO_ENANLE();

	config_gpio_init(LCD_MOSI_GPIO, LCD_MOSI_PIN,  ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_FAS, ConfigIO_Pupd_PU); 
	config_gpio_init(LCD_SCK_GPIO,  LCD_SCK_PIN,   ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_FAS, ConfigIO_Pupd_PU); 
	config_gpio_init(LCD_NCS_GPIO,  LCD_NCS_PIN,   ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_FAS, ConfigIO_Pupd_PU);  
	
    SPI_WR_REG(0x11);
    delay_ms(120);

    //PAGE1
    SPI_WR_REG(0xFF);    
    SPI_WR_DATA8(0x77);
    SPI_WR_DATA8(0x01);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x10);

    SPI_WR_REG(0xC0);    
    SPI_WR_DATA8(0x3B);
    SPI_WR_DATA8(0x00);

    SPI_WR_REG(0xC1);    
    SPI_WR_DATA8(0x0D);
    SPI_WR_DATA8(0x02);

    SPI_WR_REG(0xC2);    
    SPI_WR_DATA8(0x31);
    SPI_WR_DATA8(0x05);

    SPI_WR_REG(0xCd);
    SPI_WR_DATA8(0x08);

    SPI_WR_REG(0xB0);    
    SPI_WR_DATA8(0x00); //Positive Voltage Gamma Control
    SPI_WR_DATA8(0x11);
    SPI_WR_DATA8(0x18);
    SPI_WR_DATA8(0x0E);
    SPI_WR_DATA8(0x11);
    SPI_WR_DATA8(0x06);
    SPI_WR_DATA8(0x07);
    SPI_WR_DATA8(0x08);
    SPI_WR_DATA8(0x07);
    SPI_WR_DATA8(0x22);
    SPI_WR_DATA8(0x04);
    SPI_WR_DATA8(0x12);
    SPI_WR_DATA8(0x0F);
    SPI_WR_DATA8(0xAA);
    SPI_WR_DATA8(0x31);
    SPI_WR_DATA8(0x18);
	
	

    SPI_WR_REG(0xB1);    
    SPI_WR_DATA8(0x00); //Negative Voltage Gamma Control
    SPI_WR_DATA8(0x11);
    SPI_WR_DATA8(0x19);
    SPI_WR_DATA8(0x0E);
    SPI_WR_DATA8(0x12);
    SPI_WR_DATA8(0x07);
    SPI_WR_DATA8(0x08);
    SPI_WR_DATA8(0x08);
    SPI_WR_DATA8(0x08);
    SPI_WR_DATA8(0x22);
    SPI_WR_DATA8(0x04);
    SPI_WR_DATA8(0x11);
    SPI_WR_DATA8(0x11);
    SPI_WR_DATA8(0xA9);
    SPI_WR_DATA8(0x32);
    SPI_WR_DATA8(0x18);

    //PAGE1
    SPI_WR_REG(0xFF);    
    SPI_WR_DATA8(0x77);
    SPI_WR_DATA8(0x01);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x11);

    SPI_WR_REG(0xB0);    SPI_WR_DATA8(0x60); //Vop=4.7375v
    SPI_WR_REG(0xB1);    SPI_WR_DATA8(0x32); //VCOM=32
    SPI_WR_REG(0xB2);    SPI_WR_DATA8(0x07); //VGH=15v
    SPI_WR_REG(0xB3);    SPI_WR_DATA8(0x80);
    SPI_WR_REG(0xB5);    SPI_WR_DATA8(0x49); //VGL=-10.17v
    SPI_WR_REG(0xB7);    SPI_WR_DATA8(0x85);
    SPI_WR_REG(0xB8);    SPI_WR_DATA8(0x21); //AVDD=6.6 & AVCL=-4.6
    SPI_WR_REG(0xC1);    SPI_WR_DATA8(0x78);
    SPI_WR_REG(0xC2);    SPI_WR_DATA8(0x78);

    SPI_WR_REG(0xE0);    
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x1B);
    SPI_WR_DATA8(0x02);

    SPI_WR_REG(0xE1);   
    SPI_WR_DATA8(0x08);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x07);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x44);
    SPI_WR_DATA8(0x44);

    SPI_WR_REG(0xE2);    
    SPI_WR_DATA8(0x11);
    SPI_WR_DATA8(0x11);
    SPI_WR_DATA8(0x44);
    SPI_WR_DATA8(0x44);
    SPI_WR_DATA8(0xED);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0xEC);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);

    SPI_WR_REG(0xE3);    
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x11);
    SPI_WR_DATA8(0x11);

    SPI_WR_REG(0xE4);    
    SPI_WR_DATA8(0x44);
    SPI_WR_DATA8(0x44);

    SPI_WR_REG(0xE5);    
    SPI_WR_DATA8(0x0A);
    SPI_WR_DATA8(0xE9);
    SPI_WR_DATA8(0xD8);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x0C);
    SPI_WR_DATA8(0xEB);
    SPI_WR_DATA8(0xD8);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x0E);
    SPI_WR_DATA8(0xED);
    SPI_WR_DATA8(0xD8);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x10);
    SPI_WR_DATA8(0xEF);
    SPI_WR_DATA8(0xD8);
    SPI_WR_DATA8(0xA0);

    SPI_WR_REG(0xE6);   
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x11);
    SPI_WR_DATA8(0x11);

    SPI_WR_REG(0xE7);    
    SPI_WR_DATA8(0x44);
    SPI_WR_DATA8(0x44);

    SPI_WR_REG(0xE8);    
    SPI_WR_DATA8(0x09);
    SPI_WR_DATA8(0xE8);
    SPI_WR_DATA8(0xD8);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x0B);
    SPI_WR_DATA8(0xEA);
    SPI_WR_DATA8(0xD8);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x0D);
    SPI_WR_DATA8(0xEC);
    SPI_WR_DATA8(0xD8);
    SPI_WR_DATA8(0xA0);
    SPI_WR_DATA8(0x0F);
    SPI_WR_DATA8(0xEE);
    SPI_WR_DATA8(0xD8);
    SPI_WR_DATA8(0xA0);

    SPI_WR_REG(0xEB);    
    SPI_WR_DATA8(0x02);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0xE4);
    SPI_WR_DATA8(0xE4);
    SPI_WR_DATA8(0x88);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x40);

    SPI_WR_REG(0xEC);    
    SPI_WR_DATA8(0x3C);
    SPI_WR_DATA8(0x00);

    SPI_WR_REG(0xED);    
    SPI_WR_DATA8(0xAB);
    SPI_WR_DATA8(0x89);
    SPI_WR_DATA8(0x76);
    SPI_WR_DATA8(0x54);
    SPI_WR_DATA8(0x02);
    SPI_WR_DATA8(0xFF);
    SPI_WR_DATA8(0xFF);
    SPI_WR_DATA8(0xFF);
    SPI_WR_DATA8(0xFF);
    SPI_WR_DATA8(0xFF);
    SPI_WR_DATA8(0xFF);
    SPI_WR_DATA8(0x20);
    SPI_WR_DATA8(0x45);
    SPI_WR_DATA8(0x67);
    SPI_WR_DATA8(0x98);
    SPI_WR_DATA8(0xBA);

    SPI_WR_REG(0x36);    
    SPI_WR_DATA8(0x00);

    //-----------VAP & VAN---------------
    SPI_WR_REG(0xFF);    
    SPI_WR_DATA8(0x77);
    SPI_WR_DATA8(0x01);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x13);

    SPI_WR_REG(0xE5);    
    SPI_WR_DATA8(0xE4);

    SPI_WR_REG(0xFF);
    SPI_WR_DATA8(0x77);
    SPI_WR_DATA8(0x01);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);
    SPI_WR_DATA8(0x00);

    SPI_WR_REG(0x3A);   //0x70 RGB888, 0x60 RGB666, 0x50 RGB565
    SPI_WR_DATA8(0x60);

    SPI_WR_REG(0x21);   //Display Inversion On

    SPI_WR_REG(0x11);   //Sleep Out
    delay_ms(120);

    SPI_WR_REG(0x29);   //Display On
    delay_ms(50);

}


/**
 * @brief       3-Wire SPI发送指令
 * @param       reg:要发送的指令
 * @retval      无
 */
void SPI_WR_REG(uint8_t reg)
{
    uint8_t i;
    lcdrgb_cs_clr();
    lcdrgb_sck_clr();
    lcdrgb_sda_clr();
    lcdrgb_sck_set();
    for (i = 0; i < 8; i++)
    {
        lcdrgb_sck_clr();
        if (reg & 0x80)
        {
            lcdrgb_sda_set();
        }
        else
        {
            lcdrgb_sda_clr();
        }
        lcdrgb_sck_set();
        reg <<= 1;
    }
    lcdrgb_cs_set();
}

/**
 * @brief       3-Wire SPI发送数据
 * @param       dat:要发送的数据
 * @retval      无
 */
void SPI_WR_DATA8(uint8_t dat)
{
    uint8_t i;
    lcdrgb_cs_clr();
    lcdrgb_sck_clr();
    lcdrgb_sda_set();
    lcdrgb_sck_set();
    for (i = 0; i < 8; i++)
    {
        lcdrgb_sck_clr();
        if (dat & 0x80)
        {
            lcdrgb_sda_set();
        }
        else
        {
            lcdrgb_sda_clr();
        }
        lcdrgb_sck_set();
        dat <<= 1;
    }
    lcdrgb_cs_set();
}


