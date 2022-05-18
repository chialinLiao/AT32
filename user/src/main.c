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
#include "lv_fs_if.h"


/**========================================================================
 *                           typdef
 *========================================================================**/
typedef enum
{
  TEST_FAIL = 0,
  TEST_SUCCESS,
} test_result_type;


/**========================================================================
 *                           define
 *========================================================================**/


/**========================================================================
 *                           macro
 *========================================================================**/


/**========================================================================
 *                           variable
 *========================================================================**/
FATFS fs;


/**========================================================================
 *                           function
 *========================================================================**/
uint8_t buffer_compare(uint8_t* pbuffer1, uint8_t* pbuffer2, uint16_t buffer_length);
static void sd_test_error(void);
static void nvic_configuration(void);
static test_result_type fatfs_test(void);
void trm3_int_init(u16 arr, u16 psc);
static void my_lvgl_test (void);


/**
 * @brief 
 * 
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
  FIL file;
  BYTE work[FF_MAX_SS];
  FRESULT ret; 
  
  char filename[] = "1:/test1.txt";
  const char wbuf[] = "this is my file for test fatfs! \r\n";
  char rbuf[50];
  UINT bytes_written = 0;
  UINT bytes_read = 0;
  
  DWORD fre_clust, fre_sect, tot_sect;
  FATFS* pt_fs;

  // mount
  ret = f_mount(&fs, "1:", 1);
  
  if(ret)
  {
    if(ret == FR_NO_FILESYSTEM)
    {
      printf("create fatfs..\r\n");
      
      ret = f_mkfs("1:", 0, work, sizeof(work));

      if(ret)
      {
        printf("creates fatfs err:%d.\r\n", ret);
        return TEST_FAIL;
      }
      else
        printf("creates fatfs ok.\r\n");
      
      ret = f_mount(NULL, "1:", 1); // unmount
      ret = f_mount(&fs, "1:", 1);  // mount
      
      if(ret)
      {
        printf("fs mount err:%d.\r\n", ret);
        return TEST_FAIL;
      }
      else
        printf("fs mount ok.\r\n");
    }
    else
    {
      printf("fs mount err:%d.\r\n", ret);
      return TEST_FAIL;
    }
  }
  else
    printf("fs mount ok.\r\n");
  
  // open
  ret = f_open(&file, filename, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
  if(ret)
    printf("open file err:%d.\r\n", ret);
  else
    printf("open file ok.\r\n");

  // write
  ret = f_write(&file, wbuf, sizeof(wbuf), &bytes_written);
  if(ret)
    printf("write file err:%d.\r\n", ret);
  else
    printf("write file ok, byte:%u.\r\n", bytes_written);
  
  // read
  f_lseek(&file, 0);
  ret = f_read(&file, rbuf, sizeof(rbuf), &bytes_read);
  if(ret)
    printf("read file err:%d.\r\n", ret);
  else
    printf("read file ok, byte:%u.\r\n", bytes_read);
    
  // close
  ret = f_close(&file);
  if(ret)
    printf("close file err:%d.\r\n", ret);
  else
    printf("close file ok.\r\n");
  
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


/**
 * @brief 
 * 
 */
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
 

/**
 * @brief 
 * 
 */
void TMR3_GLOBAL_IRQHandler(void)
{ 		  
  TMR3->ists = 0;
  lv_tick_inc(1);	    
}


/**
 * @brief 
 * 
 */
static void my_lvgl_test (void)
{
  lv_obj_t * img = lv_img_create(lv_scr_act(), NULL);
  
  //* put a file name is called images.bin will be load and play
  //* please use the lvgl bmp online convert tool to do it.
  lv_img_set_src(img, "S:/images.bin");
  
  lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);
}


/**
 * @brief 
 * @return 
 */
int main(void)
{
  system_clock_config();
  at32_board_init();
  uart_print_init(115200);

  touch_struct = &touch_dev_struct;
  lcd_struct = &lcd_dev_struct;

  lcd_struct->lcd_init();
  lcd_struct->lcd_clear(GBLUE);
  
  touch_struct->init();
  touch_struct->touch_read_xy(&touch_struct->x_p[0], &touch_struct->y_p[0]);

  //* lvgl init
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
  lv_fs_if_init();

  //* for lvgl tick timer (1ms)
  trm3_int_init(288-1, 1000-1);
  
  //* enable sdio int
  nvic_configuration();
  
  //! fat fs test
  if(TEST_SUCCESS != fatfs_test()){
    while(1)  sd_test_error();
  }

  //! not detecting weather the sd card is inserted or not
  //! so must be keeping sd card in slot, if not fatfs can not be work normally  
  f_mount(&fs, "1:", 1);
  
  my_lvgl_test();
    
  while(1)
  {
    lv_task_handler(); 
  }
}


/**
  * @}
  */ 
