/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			  /* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include <string.h>
#include "at32_spi.h"		/* Example: Header file of existing MMC/SDC contorl module */
 
/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_RAM :
    result = 1;
    stat = (DSTATUS)result;
		return stat;

	case DEV_MMC :
    result = 0;
    stat = SD_disk_status();
		return stat;

	case DEV_USB :
    result = 1;
    stat = (DSTATUS)result;
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_RAM :
    result = 1;
    stat = (DSTATUS)result;
		return stat;

	case DEV_MMC :
    result = SD_disk_initialize();
    stat = (DSTATUS)result;
		return stat;

	case DEV_USB :
    result = 1;
    stat = (DSTATUS)result;
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
    result = 1;
    res = (DRESULT)result;
		return res;

	case DEV_MMC :
    result = SD_disk_read(buff, sector, count); 
    res = (DRESULT)result;
		return res;

	case DEV_USB :
    result = 1;
    res = (DRESULT)result;
		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			  /* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		  /* Start sector in LBA */
	UINT count			  /* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
    result = 1;
    res = (DRESULT)result;
		return res;

	case DEV_MMC :
    result = SD_disk_write(buff, sector, count);
    res = (DRESULT)result;
		return res;

	case DEV_USB :
    result = 1;
    res = (DRESULT)result;
		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		  /* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
    result = 1;
    res = (DRESULT)result;
		return res;

	case DEV_MMC :
    result = SD_disk_ioctl(cmd, buff);
    res = (DRESULT)result;
		return res;

	case DEV_USB :
    result = 1;
    res = (DRESULT)result;
		return res;
	}

	return RES_PARERR;
}


/**
  * @brief  Gets Time from RTC 
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{

  return 0; 
}