/**
  **************************************************************************
  * @file     at32_sdio.h
  * @version  v2.0.4
  * @date     2021-12-31
  * @brief    this file contains all the functions prototypes for the sd/mmc
  *           card at32_sdio driver firmware library.
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

/* define to prevent recursive inclusion -------------------------------------*/
#ifndef __AT32_SPI_H
#define __AT32_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes ------------------------------------------------------------------*/                                                                               
#include "ff.h"			      /* Obtains integer types */
#include "diskio.h"		    /* Declarations of disk functions */
#include "at32f435_437.h"

/** @addtogroup 435_SPI_fatfs
  * @{
  */

/** @addtogroup AT32F435_periph_examples
  * @{
  */
#define SD_SPIx         SPI3
#define	SD_CS_PORT      GPIOA
#define SD_CS_PIN       GPIO_PINS_4
#define SPI_TIMEOUT     100      // uint: ms

/* Definitions for MMC/SDC command */
#define CMD0   (0)			/* GO_IDLE_STATE */
#define CMD1   (1)			/* SEND_OP_COND */
#define ACMD41 (41|0x80)	/* SEND_OP_COND (SDC) */
#define CMD8   (8)			/* SEND_IF_COND */
#define CMD9   (9)			/* SEND_CSD */
#define CMD10  (10)			/* SEND_CID */
#define CMD12  (12)			/* STOP_TRANSMISSION */
#define ACMD13 (13|0x80)	/* SD_STATUS (SDC) */
#define CMD16  (16)			/* SET_BLOCKLEN */
#define CMD17  (17)			/* READ_SINGLE_BLOCK */
#define CMD18  (18)			/* READ_MULTIPLE_BLOCK */
#define CMD23  (23)			/* SET_BLOCK_COUNT */
#define ACMD23 (23|0x80)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)			/* WRITE_BLOCK */
#define CMD25  (25)			/* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)			/* SEND_OP_COND (ACMD) */
#define CMD55  (55)			/* APP_CMD */
#define CMD58  (58)			/* READ_OCR */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)		/* SD */
#define CT_BLOCK	0x08		/* Block addressing */

/* Variables */
extern uint16_t Timer1;		
extern uint16_t Timer2;
extern volatile uint16_t spi_ticks;

/* Functions */
void SPI_Config(uint8_t mode);
void SPI_GPIO_Config(void);

DSTATUS SD_disk_initialize (void);
DSTATUS SD_disk_status (void);
DRESULT SD_disk_read (BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_write (const BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_ioctl (BYTE cmd, void* buff);

#ifdef __cplusplus
}
#endif

#endif /* __AT32_SDIO_H */

