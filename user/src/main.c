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
static void event_handler(lv_obj_t * obj, lv_event_t event);
void lv_ex_btn_1(void);


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
  //touch_struct->touch_read_xy(&touch_struct->x_p[0], &touch_struct->y_p[0]);

  //* for lvgl tick timer */
  trm3_int_init(288-1, 1000-1);

  //* lvgl init */
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();

  lv_ex_btn_1();

  while(1)
  {
    static uint8_t testNo = 1;

    // if(testNo == 0)
    // {
    //   //* lcd & touch test, example from at32 provide
    //   touch_struct->touch_scan();
    // }
    // else
    // {
    //   //* lcd test, change color fast to see what err will be happend
    //   delay_ms(100);
    
    //   point_color = color_arr[point_index];
    //   lcd_struct->lcd_clear(point_color);

    //   if(++point_index >= (sizeof(color_arr) / sizeof(uint16_t)))
    //     point_index = 0;

    //   //! so far, once lcd change to white(0xffff) color that it will dispaly incorrect
    //   //! the root cause is not yet to find to fix. (now, skip the dispaly white color)
    // }

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

static void event_handler(lv_obj_t * obj, lv_event_t event)
{
  if(event == LV_EVENT_CLICKED) {
    printf("Clicked\n");
  }
  else if(event == LV_EVENT_VALUE_CHANGED) {
    printf("Toggled\n");
  }
}

void lv_ex_btn_1(void)
{
  lv_obj_t * label;

  lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_event_cb(btn1, event_handler);
  lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, 20);

  label = lv_label_create(btn1, NULL);
  lv_label_set_text(label, "Button");

  lv_obj_t * btn2 = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_event_cb(btn2, event_handler);
  lv_obj_align(btn2, NULL, LV_ALIGN_CENTER, 0, 20);
  lv_btn_set_checkable(btn2, true);
  lv_btn_toggle(btn2);
  lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_TIGHT);

  label = lv_label_create(btn2, NULL);
  lv_label_set_text(label, "Toggled");
}

/**
  * @}
  */ 

/**
  * @}
  */ 

