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
void lv_ex_get_started_2(void);


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

  lv_ex_get_started_2();

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

static void event_handler(lv_obj_t * obj, lv_event_t event)
{
  if(event == LV_EVENT_CLICKED) {
    printf("Clicked\n");
  }
  else if(event == LV_EVENT_VALUE_CHANGED) {
    printf("Toggled\n");
  }
}

/**
 * Create styles from scratch for buttons.
 */
void lv_ex_get_started_2(void)
{
    static lv_style_t style_btn;
    static lv_style_t style_btn_red;

    /*Create a simple button style*/
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, 10);
    lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_bg_grad_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_bg_grad_dir(&style_btn, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);

    /*Swap the colors in pressed state*/
    lv_style_set_bg_color(&style_btn, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_grad_color(&style_btn, LV_STATE_PRESSED, LV_COLOR_SILVER);

    /*Add a border*/
    lv_style_set_border_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_70);
    lv_style_set_border_width(&style_btn, LV_STATE_DEFAULT, 2);

    /*Different border color in focused state*/
    lv_style_set_border_color(&style_btn, LV_STATE_FOCUSED, LV_COLOR_BLUE);
    lv_style_set_border_color(&style_btn, LV_STATE_FOCUSED | LV_STATE_PRESSED, LV_COLOR_NAVY);

    /*Set the text style*/
    lv_style_set_text_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    /*Make the button smaller when pressed*/
    lv_style_set_transform_height(&style_btn, LV_STATE_PRESSED, -5);
    lv_style_set_transform_width(&style_btn, LV_STATE_PRESSED, -10);
#if LV_USE_ANIMATION
    /*Add a transition to the size change*/
    static lv_anim_path_t path;
    lv_anim_path_init(&path);
    lv_anim_path_set_cb(&path, lv_anim_path_overshoot);

    lv_style_set_transition_prop_1(&style_btn, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_HEIGHT);
    lv_style_set_transition_prop_2(&style_btn, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_WIDTH);
    lv_style_set_transition_time(&style_btn, LV_STATE_DEFAULT, 300);
    lv_style_set_transition_path(&style_btn, LV_STATE_DEFAULT, &path);
#endif

    /*Create a red style. Change only some colors.*/
    lv_style_init(&style_btn_red);
    lv_style_set_bg_color(&style_btn_red, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_bg_grad_color(&style_btn_red, LV_STATE_DEFAULT, LV_COLOR_MAROON);
    lv_style_set_bg_color(&style_btn_red, LV_STATE_PRESSED, LV_COLOR_MAROON);
    lv_style_set_bg_grad_color(&style_btn_red, LV_STATE_PRESSED, LV_COLOR_RED);
    lv_style_set_text_color(&style_btn_red, LV_STATE_DEFAULT, LV_COLOR_WHITE);
#if LV_USE_BTN
    /*Create buttons and use the new styles*/
    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_reset_style_list(btn, LV_BTN_PART_MAIN);         /*Remove the styles coming from the theme*/
    lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);

    lv_obj_t * label = lv_label_create(btn, NULL);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/

    /*Create a new button*/
    lv_obj_t * btn2 = lv_btn_create(lv_scr_act(), btn);
    lv_obj_set_pos(btn2, 10, 80);
    lv_obj_set_size(btn2, 120, 50);                             /*Set its size*/
    lv_obj_reset_style_list(btn2, LV_BTN_PART_MAIN);         /*Remove the styles coming from the theme*/
    lv_obj_add_style(btn2, LV_BTN_PART_MAIN, &style_btn);
    lv_obj_add_style(btn2, LV_BTN_PART_MAIN, &style_btn_red);   /*Add the red style on top of the current */
    lv_obj_set_style_local_radius(btn2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE); /*Add a local style*/

    label = lv_label_create(btn2, NULL);          /*Add a label to the button*/
    lv_label_set_text(label, "Button 2");                     /*Set the labels text*/
#endif
}
/**
  * @}
  */ 

/**
  * @}
  */ 

