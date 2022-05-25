/**
  **************************************************************************
  * @file     xmc_lcd.c
  * @version  v2.0.4
  * @date     2021-12-31
  * @brief    xmc_lcd program file
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to 
  * download from Artery official website is the copyrighted work of Artery. 
  * Artery authorizes customers to use, copy, and distribute the BSP 
  * software and its related documentation for the purpose of design and 
  * development in conjunction with Artery microcontrollers. Use of the 
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
  
#include "xmc_lcd.h"

/** @addtogroup AT32F435_periph_examples
  * @{
  */
  
/** @addtogroup 435_XMC_lcd_touch_16bit
  * @{
  */
  
lcd_dev_type lcd_dev_struct = 
{
  0,
  xmc_init,       /*!< function for xmc and gpios init */
  lcd_init,       /*!< function for configures the lcd */
  lcd_setblock,   /*!< lcd function to set block or set window */
  lcd_drawpoint,  /*!< lcd function to drawpoint */
  lcd_clear,      /*!< lcd function to clear */
};

lcd_dev_type *lcd_struct;

/**
  * @brief  configures the xmc and gpios to interface with the lcd.
  *         this function must be called before any write/read operation
  *         on the lcd.
  * @param  none 
  * @retval none
  */
void xmc_init(void)
{
  gpio_init_type  gpio_init_struct = {0};
  xmc_norsram_init_type  xmc_norsram_init_struct;
  xmc_norsram_timing_init_type rw_timing_struct, w_timing_struct;
  
  /* enable the gpio clock */
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOE_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOF_PERIPH_CLOCK, TRUE);
  
  /* enable the xmc clock */
  crm_periph_clock_enable(CRM_XMC_PERIPH_CLOCK, TRUE);
  
  /*-- gpio configuration ------------------------------------------------------*/
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE4, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE5, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE7, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE0, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE1, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE8, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE9, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE10, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE14, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE15, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOE, GPIO_PINS_SOURCE7, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOE, GPIO_PINS_SOURCE8, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOE, GPIO_PINS_SOURCE9, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOE, GPIO_PINS_SOURCE10, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOE, GPIO_PINS_SOURCE11, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOE, GPIO_PINS_SOURCE12, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOE, GPIO_PINS_SOURCE13, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOE, GPIO_PINS_SOURCE14, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOE, GPIO_PINS_SOURCE15, GPIO_MUX_12);
  gpio_pin_mux_config(GPIOF, GPIO_PINS_SOURCE0, GPIO_MUX_12);
  
  /* lcd cs(ne1)/ wr(nwe)/ rd(noe) lines configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_4 | GPIO_PINS_5 | GPIO_PINS_7 ;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOD, &gpio_init_struct);
  
  /* lcd rs(a0) lines configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_0;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOF, &gpio_init_struct);
  
  /* data lines configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_8 | GPIO_PINS_9 | GPIO_PINS_10 | GPIO_PINS_14 | GPIO_PINS_15;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOD, &gpio_init_struct);
  
  gpio_init_struct.gpio_pins = GPIO_PINS_7 | GPIO_PINS_8 | GPIO_PINS_9 | GPIO_PINS_10 | GPIO_PINS_11 | GPIO_PINS_12 | GPIO_PINS_13 | GPIO_PINS_14 | GPIO_PINS_15;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOE, &gpio_init_struct);
  
  /* lcd reset lines configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_9;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOB, &gpio_init_struct);
  
  /* lcd bl lines configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_8;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOB, &gpio_init_struct);
  
  /*-- xmc configuration ------------------------------------------------------*/
  xmc_norsram_default_para_init(&xmc_norsram_init_struct);
  xmc_norsram_init_struct.subbank = XMC_BANK1_NOR_SRAM1;
  xmc_norsram_init_struct.data_addr_multiplex = XMC_DATA_ADDR_MUX_DISABLE;
  xmc_norsram_init_struct.device = XMC_DEVICE_SRAM;
  xmc_norsram_init_struct.bus_type = XMC_BUSTYPE_16_BITS;
  xmc_norsram_init_struct.burst_mode_enable = XMC_BURST_MODE_DISABLE;
  xmc_norsram_init_struct.asynwait_enable = XMC_ASYN_WAIT_DISABLE;
  xmc_norsram_init_struct.wait_signal_lv = XMC_WAIT_SIGNAL_LEVEL_LOW;
  xmc_norsram_init_struct.wrapped_mode_enable = XMC_WRAPPED_MODE_DISABLE;
  xmc_norsram_init_struct.wait_signal_config = XMC_WAIT_SIGNAL_SYN_BEFORE;
  xmc_norsram_init_struct.write_enable = XMC_WRITE_OPERATION_ENABLE;
  xmc_norsram_init_struct.wait_signal_enable = XMC_WAIT_SIGNAL_DISABLE;
  xmc_norsram_init_struct.write_timing_enable = XMC_WRITE_TIMING_ENABLE;
  xmc_norsram_init_struct.write_burst_syn = XMC_WRITE_BURST_SYN_DISABLE;
  xmc_nor_sram_init(&xmc_norsram_init_struct);
  
  /* timing configuration */
  xmc_norsram_timing_default_para_init(&rw_timing_struct, &w_timing_struct);
  rw_timing_struct.subbank = XMC_BANK1_NOR_SRAM1;
  rw_timing_struct.write_timing_enable = XMC_WRITE_TIMING_ENABLE;
  rw_timing_struct.addr_setup_time = 0x2;
  rw_timing_struct.addr_hold_time = 0x0;
  rw_timing_struct.data_setup_time = 0x2;
  rw_timing_struct.bus_latency_time = 0x0;
  rw_timing_struct.clk_psc = 0x0;
  rw_timing_struct.data_latency_time = 0x0;
  rw_timing_struct.mode = XMC_ACCESS_MODE_A;
  w_timing_struct.subbank = XMC_BANK1_NOR_SRAM1;
  w_timing_struct.write_timing_enable = XMC_WRITE_TIMING_ENABLE;
  w_timing_struct.addr_setup_time = 0x6;
  w_timing_struct.addr_hold_time = 0x0;
  w_timing_struct.data_setup_time = 0x6;
  w_timing_struct.bus_latency_time = 0x0;
  w_timing_struct.clk_psc = 0x0;
  w_timing_struct.data_latency_time = 0x0;
  w_timing_struct.mode = XMC_ACCESS_MODE_A;  
  xmc_nor_sram_timing_config(&rw_timing_struct, &w_timing_struct);
  
  /* enable xmc_bank1_nor_sram4 */
  xmc_nor_sram_enable(XMC_BANK1_NOR_SRAM1, TRUE);
}

/**
  * @brief  configures the lcd.
  *         this function must be called before any write/read operation
  *         on the lcd.
  * @param  none 
  * @retval none
  */
void lcd_init(void)
{
  /* init xmc */
  lcd_struct->xmc_init();
  
  LCD_RESET_LOW;
  
  delay_ms(50);

  LCD_RESET_HIGH;
  
  delay_ms(50);
  
  /* read id */
  lcd_wr_command(0x0000);
  delay_ms(5);
  
  lcd_struct->lcd_id = lcd_rd_data();
  lcd_wr_command(0xd3);
  
  lcd_struct->lcd_id = lcd_rd_data();
  lcd_struct->lcd_id = lcd_rd_data();
  lcd_struct->lcd_id = lcd_rd_data();
  lcd_struct->lcd_id = lcd_rd_data();
  lcd_struct->lcd_id = lcd_struct->lcd_id << 8;
  lcd_struct->lcd_id |= lcd_rd_data();
  
  lcd_wr_command(LCD_POWERB);
  lcd_wr_data(0x00);
  lcd_wr_data(0x99);
  lcd_wr_data(0X30);

  lcd_wr_command(LCD_POWER_SEQ);
  lcd_wr_data(0x67);
  lcd_wr_data(0x03);
  lcd_wr_data(0X12);
  lcd_wr_data(0X81);

  lcd_wr_command(LCD_DTCA);
  lcd_wr_data(0x85);
  lcd_wr_data(0x01);
  lcd_wr_data(0x78);

  lcd_wr_command(LCD_POWERA);
  lcd_wr_data(0x39);
  lcd_wr_data(0x2C);
  lcd_wr_data(0x00);
  lcd_wr_data(0x34);
  lcd_wr_data(0x02);

  lcd_wr_command(LCD_PRC);
  lcd_wr_data(0x20);

  lcd_wr_command(LCD_DTCB);
  lcd_wr_data(0x00);
  lcd_wr_data(0x00);

  lcd_wr_command(LCD_POWER1);
  lcd_wr_data(0x25);

  lcd_wr_command(LCD_POWER2);
  lcd_wr_data(0x10);

  lcd_wr_command(LCD_VCOM1);
  lcd_wr_data(0x40);
  lcd_wr_data(0x3F);

  lcd_wr_command(LCD_VCOM2);
  lcd_wr_data(0xB0);

  // Display Function Control
  lcd_wr_command(LCD_DFC); 
  lcd_wr_data(0x00);
  lcd_wr_data(0XC2);
  
  //* control data to gram dispay direction
  lcd_wr_command(LCD_MAC);
  lcd_wr_data(0x68);
  
  lcd_wr_command(LCD_PIXEL_FORMAT);
  lcd_wr_data(0x55);
    
  lcd_wr_command(LCD_3GAMMA_EN);
  lcd_wr_data(0x00);
    
  lcd_wr_command(LCD_GAMMA);
  lcd_wr_data(0x01);
  
  //Set Gamma
  lcd_wr_command(LCD_PGAMMA);
  lcd_wr_data(0x0F);
  lcd_wr_data(0x27);
  lcd_wr_data(0x23);
  lcd_wr_data(0x0B);
  lcd_wr_data(0x0F);
  lcd_wr_data(0x05);
  lcd_wr_data(0x54);
  lcd_wr_data(0x74);
  lcd_wr_data(0x45);
  lcd_wr_data(0x0A);
  lcd_wr_data(0x17);
  lcd_wr_data(0x0A);
  lcd_wr_data(0x1C);
  lcd_wr_data(0x0E);
  lcd_wr_data(0x08);  
  
  lcd_wr_command(LCD_NGAMMA);
  lcd_wr_data(0x08);
  lcd_wr_data(0x1A);
  lcd_wr_data(0x1E);
  lcd_wr_data(0x03);
  lcd_wr_data(0x0F);
  lcd_wr_data(0x05);
  lcd_wr_data(0x2E);
  lcd_wr_data(0x25);
  lcd_wr_data(0x3B);
  lcd_wr_data(0x01);
  lcd_wr_data(0x06);
  lcd_wr_data(0x05);
  lcd_wr_data(0x25);
  lcd_wr_data(0x33);
  lcd_wr_data(0x0F);
      
  lcd_wr_command(LCD_SLEEP_OUT);
  lcd_wr_command(LCD_DISPLAY_ON);
  lcd_wr_command(LCD_DINVOFF);
  
  LCD_BL_HIGH;  
}

/**
  * @brief  this function is write command to lcd.
  * @param  command : the command to write.
  * @retval none
  */
void lcd_wr_command(uint16_t command)
{
  *(__IO uint16_t*) XMC_LCD_COMMAND = command;
}

/**
  * @brief  this function is write data to lcd.
  * @param  data : the data to write.
  * @retval none
  */
void lcd_wr_data(uint16_t data)
{
  *(__IO uint16_t*) XMC_LCD_DATA = data;
}

uint16_t lcd_rd_data(void)
{
  uint16_t data;  
  
  data = *(uint16_t*)XMC_LCD_DATA;
  
  return data;
}

/**
  * @brief  this function is set row&column coordinates for lcd.
  * @param  xstart : row coordinates starting vaule.
  * @param  ystart : column coordinates starting vaule.
  * @param  xend : row coordinates ending vaule.
  * @param  yend : column coordinates ending vaule.
  * @retval none
  */
void lcd_setblock(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend)
{
  /* set row coordinates */
  lcd_wr_command(0x2a);
  lcd_wr_data(xstart >> 8);
  lcd_wr_data(xstart);
  lcd_wr_data(xend >> 8);
  lcd_wr_data(xend);
  
  /* set column coordinates */
  lcd_wr_command(0x2b);
  lcd_wr_data(ystart >> 8);
  lcd_wr_data(ystart);
  lcd_wr_data(yend >> 8);
  lcd_wr_data(yend);
  
  /* enable write menory */
  lcd_wr_command(0x2c);
}

/**
  * @brief  this function is write one point to lcd.
  * @param  data : the data to write.
  * @retval none
  */
void lcd_writeonepoint(uint16_t color)
{
  lcd_wr_data(color);
}

/**
  * @brief  this function is draw point to lcd.
  * @param  data : the data to write.
  * @retval None
  */
void lcd_drawpoint(uint16_t x, uint16_t y, uint16_t color)
{
  lcd_struct->lcd_setblock(x, y ,x ,y); 
  
  lcd_writeonepoint(color);
}

/**
  * @brief  this function is clear the lcd.
  * @param  data : the data to write.
  * @retval none
  */
void lcd_clear(uint16_t color)
{
  uint32_t i;
  
  lcd_struct->lcd_setblock(0, 0, LCD_HOR_RES, LCD_VER_RES);
  
  for(i = 0; i < LCD_HOR_RES*LCD_VER_RES; i++)
  {
    lcd_wr_data(color);
  }
}

/**
  * @}
  */ 

/**
  * @}
  */ 
