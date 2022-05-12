/**
  **************************************************************************
  * @file     main.c
  * @version  v2.0.4
  * @date     2021-12-31
  * @brief    main program
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

#include "at32f435_437_board.h"
#include "at32f435_437_clock.h"
#include "xmc_lcd.h"
#include "touch.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"

/** @addtogroup AT32F435_periph_examples
  * @{
  */
  
/** @addtogroup 435_XMC_lcd_touch_16bit XMC_lcd_touch_16bit
  * @{
  */

/** typdef 
*/

/** define 
*/

/** macro 
*/

/** variables 
*/
uint16_t point_color;
uint16_t point_index = 0;

uint16_t color_arr[] = {
  WHITE, BLACK, BLUE, BRED, GRED, GBLUE, RED, MAGENTA, GREEN, CYAN, YELLOW, BROWN, BRRED, GRAY };

/** functions 
*/
void trm3_int_init(u16 arr, u16 psc);  

/** demo from external  
*/
extern void lv_ex_img_1(void);
extern void lv_ex_img_2(void);
extern void lv_ex_img_3(void);


/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  touch_struct = &touch_dev_struct;
  lcd_struct = &lcd_dev_struct;
  system_clock_config();
  at32_board_init();

  uart_print_init(115200);

  lcd_struct->lcd_init();
  point_color = GBLUE;
  lcd_struct->lcd_clear(point_color);
  
  touch_struct->init();
  touch_struct->touch_read_xy(&touch_struct->x_p[0], &touch_struct->y_p[0]);

  //* lvgl init */
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();

  //* demo
  static uint8_t img_demo = 2;
  if(img_demo == 0)       lv_ex_img_1();
  else if(img_demo == 1)  lv_ex_img_2();
  else                    lv_ex_img_3();

  //* for lvgl tick timer */
  trm3_int_init(288-1, 1000-1);

  while(1)
  {
    lv_task_handler(); 
  }
}

void trm3_int_init(u16 arr, u16 psc)
{
  /* enable tmr3 clock */
  crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK, TRUE);

  tmr_base_init(TMR3, arr, psc);
  tmr_cnt_dir_set(TMR3, TMR_COUNT_UP);
  tmr_interrupt_enable(TMR3, TMR_OVF_INT, TRUE);

  /* tmr3 overflow interrupt nvic init */
  nvic_irq_enable(TMR3_GLOBAL_IRQn, 1, 0);

  /* enable tmr3 */
  tmr_counter_enable(TMR3, TRUE);  
}
 
void TMR3_GLOBAL_IRQHandler(void)
{ 		  
  TMR3->ists = 0;;
  
  lv_tick_inc(1);	    
  //at32_led_toggle(LED2); 
}




/**
  * @}
  */ 

/**
  * @}
  */ 

