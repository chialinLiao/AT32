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
  WHITE,
  BLACK,                               
  BLUE,                              
  BRED,                            
  GRED,                            
  GBLUE,                           
  RED,                             
  MAGENTA,                         
  GREEN,                           
  CYAN,                            
  YELLOW,                          
  BROWN,                           
  BRRED,                           
  GRAY 
};

/** functions 
*/
void trm3_int_init(u16 arr, u16 psc);  



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

  lcd_struct->lcd_init();
  point_color = GBLUE;
  lcd_struct->lcd_clear(point_color);
  
  touch_struct->init();
  touch_struct->touch_read_xy(&touch_struct->x_p[0], &touch_struct->y_p[0]);

  /* for lvgl tick timer */
  trm3_int_init(288-1, 1000-1);

  while(1)
  {
    static uint8_t testNo = 1;

    if(testNo == 0)
    {
      //* lcd & touch test, example from at32 provide
      touch_struct->touch_scan();
    }
    else
    {
      //* lcd test, change color fast to see what err will be happend
      delay_ms(100);
    
      point_color = color_arr[point_index];
      lcd_struct->lcd_clear(point_color);

      if(++point_index >= (sizeof(color_arr) / sizeof(uint16_t)))
        point_index = 0;

      //! so far, once lcd change to white(0xffff) color that it will dispaly incorrect
      //! the root cause is not yet to find to fix. (now, skip the dispaly white color)
    }
  }
}

void trm3_int_init(u16 arr, u16 psc)
{

/* enable tmr1 clock */
  crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK, TRUE);

  tmr_base_init(TMR3, arr, psc);
  tmr_cnt_dir_set(TMR3, TMR_COUNT_UP);
  tmr_interrupt_enable(TMR3, TMR_OVF_INT, TRUE);

  /* tmr1 overflow interrupt nvic init */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_0);
  nvic_irq_enable(TMR3_GLOBAL_IRQn, 1, 0);

  /* enable tmr3 */
  tmr_counter_enable(TMR3, TRUE);  
}
 
void TMR3_GLOBAL_IRQHandler(void)
{ 		  
  TMR3->ists = 0;;
  //lv_tick_inc(1);	    

  at32_led_toggle(LED2); 
}


/**
  * @}
  */ 

/**
  * @}
  */ 

