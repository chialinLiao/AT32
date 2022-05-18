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

#include <stdbool.h>
#include <string.h>
#include "at32_sdio.h"
#include "at32f435_437_board.h"
#include "at32f435_437_clock.h"
#include "xmc_lcd.h"
#include "touch.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "ff.h" 

/** @addtogroup AT32F435_periph_examples
  * @{
  */
  
/** @addtogroup 435_XMC_lcd_touch_16bit XMC_lcd_touch_16bit
  * @{
  */

/** typdef 
*/
typedef enum
{
  TEST_FAIL = 0,
  TEST_SUCCESS,
} test_result_type;

/** define 
*/

/** macro 
*/

/** variables 
*/
uint16_t point_color;
uint16_t point_index = 0;

const uint16_t color_arr[] = {
  WHITE, BLACK, BLUE, BRED, GRED, GBLUE, RED, MAGENTA, GREEN, CYAN, YELLOW, BROWN, BRRED, GRAY };

//* fatfs
FATFS fs;
FIL file;
BYTE work[FF_MAX_SS];

/** functions 
*/
uint8_t buffer_compare(uint8_t* pbuffer1, uint8_t* pbuffer2, uint16_t buffer_length);
static void sd_test_error(void);
static void nvic_configuration(void);
void trm3_int_init(u16 arr, u16 psc);

/** demo from external  
*/
extern void lv_ex_img_1(void);
extern void lv_ex_img_2(void);
extern void lv_ex_img_3(void);


/**
  * @brief  compares two buffers.
  * @param  pbuffer1, pbuffer2: buffers to be compared.
  * @param  buffer_length: buffer's length
  * @retval 1: pbuffer1 identical to pbuffer2
  *         0: pbuffer1 differs from pbuffer2
  */
uint8_t buffer_compare(uint8_t* pbuffer1, uint8_t* pbuffer2, uint16_t buffer_length)
{
  while(buffer_length--)
  {
    if(*pbuffer1 != *pbuffer2)
    {
      return 0;
    }
    pbuffer1++;
    pbuffer2++;
  }
  return 1;
}

/**
  * @brief  led2 on off every 300ms for sd test error.
  * @param  none
  * @retval none
  */
static void sd_test_error(void)
{
  at32_led_on(LED2);
  delay_ms(300);
  at32_led_off(LED2);
  delay_ms(300);
}

/**
  * @brief  configures sdio1 irq channel.
  * @param  none
  * @retval none
  */
static void nvic_configuration(void)
{
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(SDIO1_IRQn, 1, 0);
}

/**
  * @brief  fatfs file read/write test.
  * @param  none
  * @retval TEST_FAIL: fail.
  *         TEST_SUCCESS: success.
  */
static test_result_type fatfs_test(void)
{
  FRESULT ret; 
  char filename[] = "1:/test1.txt";
  const char wbuf[] = "this is my file for test fatfs!\r\n";
  char rbuf[50];
  UINT bytes_written = 0;
  UINT bytes_read = 0;
  DWORD fre_clust, fre_sect, tot_sect;
  FATFS* pt_fs;
  
  ret = f_mount(&fs, "1:", 1);
  
  if(ret){
    printf("fs mount err:%d.\r\n", ret);
    
    if(ret == FR_NO_FILESYSTEM){
      printf("create fatfs..\r\n");
      
      ret = f_mkfs("1:", 0, work, sizeof(work));

      if(ret){
        printf("creates fatfs err:%d.\r\n", ret);
        return TEST_FAIL;
      }
      else{
        printf("creates fatfs ok.\r\n");
      }
      
      ret = f_mount(NULL, "1:", 1);
      ret = f_mount(&fs, "1:", 1);
      
      if(ret){
        printf("fs mount err:%d.\r\n", ret);
        return TEST_FAIL;
      }
      else{
        printf("fs mount ok.\r\n");
      }
    }
    else{
      return TEST_FAIL;
    }
  }
  else{
    printf("fs mount ok.\r\n");
  }
  
  ret = f_open(&file, filename, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
  if(ret){
    printf("open file err:%d.\r\n", ret);
  }
  else{
    printf("open file ok.\r\n");
  }
 
  ret = f_write(&file, wbuf, sizeof(wbuf), &bytes_written);
  if(ret){
    printf("write file err:%d.\r\n", ret);
  }
  else{
    printf("write file ok, byte:%u.\r\n", bytes_written);
  }
  
  f_lseek(&file, 0);
  ret = f_read(&file, rbuf, sizeof(rbuf), &bytes_read);
  if(ret){
    printf("read file err:%d.\r\n", ret);
  }
  else{
    printf("read file ok, byte:%u.\r\n", bytes_read);
  }
  
  ret = f_close(&file);
  if(ret){
    printf("close file err:%d.\r\n", ret);
  }
  else{
    printf("close file ok.\r\n");
  }
  
  pt_fs = &fs;
  /* get volume information and free clusters of drive 1 */
  ret = f_getfree("1:", &fre_clust, &pt_fs);
  if(ret == FR_OK)
  {
    /* get total sectors and free sectors */
    tot_sect = (pt_fs->n_fatent - 2) * pt_fs->csize;
    fre_sect = fre_clust * pt_fs->csize;

    /* print the free space (assuming 512 bytes/sector) */
    printf("%10u KiB total drive space.\r\n%10u KiB available.\r\n", tot_sect / 2, fre_sect / 2);
  }
  
  ret = f_mount(NULL, "1:", 1);
  
  if(1 == buffer_compare((uint8_t*)rbuf, (uint8_t*)wbuf, sizeof(wbuf))){
    printf("r/w file data test ok.\r\n");
  }
  else{
    printf("r/w file data test fail.\r\n");
    return TEST_FAIL;
  }
  
  return TEST_SUCCESS;
}

/* gloable functions ---------------------------------------------------------*/
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
  
  //* enable sdio
  nvic_configuration();
  
  printf("start test fatfs r0.14b..\r\n");
  
  if(TEST_SUCCESS != fatfs_test())
  {
    while(1)
    {
      sd_test_error();
    }
  }
  
  /* all tests pass, led3 and led4 fresh */
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
