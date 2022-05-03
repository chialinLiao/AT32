
/**
 * @file at32_spi.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-04-28
 *
 * @note
 *
 *
 * @copyright Copyright (c) 2022
 *
 */

/**========================================================================
 *                           include
 *========================================================================**/
#include <string.h>
#include <stdbool.h>
#include "at32_spi.h"

/**========================================================================
 *                           define
 *========================================================================**/

/**========================================================================
 *                           macro
 *========================================================================**/

/**========================================================================
 *                           typedef
 *========================================================================**/
typedef enum
{
  SPI_SLow = 0,
  SPI_Fast,
} spiSpeed_type;

/**========================================================================
 *                           variables
 *========================================================================**/
//* for SD-SPI control
static volatile DSTATUS Stat = STA_NOINIT; /* Disk Status */
static uint8_t CardType;                   /* Type 0:MMC, 1:SDC, 2:Block addressing */
static uint8_t PowerFlag = 0;              /* Power flag */
uint16_t Timer1, Timer2;                   /* 1ms Timer Counter */

//* for spi module init
spi_init_type spi_init_struct;

//* for spi gpio module init
gpio_type *cs_gpio_port = SD_CS_PORT;
uint16_t cs_gpio_pin = SD_CS_PIN;

volatile uint16_t spi_ticks;

/**========================================================================
 *                           funtions
 *========================================================================**/
static uint8_t SELECT(void);
static void DESELECT(void);
static void SPI_TxByte(uint8_t data);
static void SPI_TxBuffer(uint8_t *buffer, uint16_t len);
static uint8_t SPI_RxByte(void);
static void SPI_RxBytePtr(uint8_t *buff);

static uint8_t SD_ReadyWait(void);
static void SD_PowerOn(void);
static void SD_PowerOff(void);
static uint8_t SD_CheckPower(void);
static bool SD_RxDataBlock(BYTE *buff, UINT len);
static bool SD_TxDataBlock(const uint8_t *buff, BYTE token);
static BYTE SD_SendCmd(BYTE cmd, uint32_t arg);

/**========================================================================
 *                           Low level functions
 *========================================================================**/

/**
 * @brief  spi configuration.
 * @param  none
 * @retval none
 */
void SPI_Config(uint8_t mode)
{
  spi_enable(SPI3, FALSE);

  crm_periph_clock_enable(CRM_SPI3_PERIPH_CLOCK, TRUE);

  spi_default_para_init(&spi_init_struct);
  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  
  if(mode == 0)
  {
    // slow mode for init 144 / 512 = 281.25Khz
    spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_512;
  }
  else
  {
    // fast mode for operation, 144/32 = 4.5Mhz
    spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_32;
  }

  spi_init(SPI3, &spi_init_struct);

  spi_enable(SPI3, TRUE);
}

/**
 * @brief  gpio configuration.
 * @param  none
 * @retval none
 */
void SPI_GPIO_Config(void)
{
  gpio_init_type gpio_initstructure;
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_initstructure);

  /* spi3 cs pin */
  gpio_initstructure.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pull = GPIO_PULL_UP;
  gpio_initstructure.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_initstructure.gpio_pins = GPIO_PINS_4;
  gpio_init(GPIOA, &gpio_initstructure);
  cs_gpio_port->scr = cs_gpio_pin;

  /* spi3 sck pin */
  gpio_initstructure.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pull = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_mode = GPIO_MODE_MUX;
  gpio_initstructure.gpio_pins = GPIO_PINS_10;
  gpio_init(GPIOC, &gpio_initstructure);
  gpio_pin_mux_config(GPIOC, GPIO_PINS_SOURCE10, GPIO_MUX_6);

  /* spi3 miso pin */
  gpio_initstructure.gpio_pull = GPIO_PULL_UP;
  gpio_initstructure.gpio_pins = GPIO_PINS_11;
  gpio_init(GPIOC, &gpio_initstructure);
  gpio_pin_mux_config(GPIOC, GPIO_PINS_SOURCE11, GPIO_MUX_6);

  /* spi3 mosi pin */
  gpio_initstructure.gpio_pull = GPIO_PULL_UP;
  gpio_initstructure.gpio_pins = GPIO_PINS_12;
  gpio_init(GPIOC, &gpio_initstructure);
  gpio_pin_mux_config(GPIOC, GPIO_PINS_SOURCE12, GPIO_MUX_6);
}


/**
 * @brief slave select
 */
static uint8_t SELECT(void)
{
  // delay a while
  for(uint8_t i=0; i<100; i++);
  
  cs_gpio_port->clr = cs_gpio_pin;

  SPI_TxByte(0xFF);		/* Dummy clock (force DO enabled) */

  if (SD_ReadyWait() != 0xFF)
  {
    DESELECT();
    return FALSE; /* Timeout */
  }

  return TRUE;	
}

/**
 * @brief slave deselect
 */
static void DESELECT(void)
{
  // delay a while
  for(uint8_t i=0; i<100; i++);

  cs_gpio_port->scr = cs_gpio_pin;

  SPI_TxByte(0xFF);		/* Dummy clock (force DO hi-z for multiple slave SPI) */
}

/**
 * @brief SPI transmit a byte
 * @param data
 */
static void SPI_TxByte(uint8_t data)
{
  // check tx buf is empty
  while (spi_i2s_flag_get(SD_SPIx, SPI_I2S_TDBE_FLAG) == RESET);
  spi_i2s_data_transmit(SD_SPIx, data);

  //* check rx buf is full
  while (spi_i2s_flag_get(SD_SPIx, SPI_I2S_RDBF_FLAG) == RESET);
}

/**
 * @brief SPI transmit buffer
 * @param buffer
 * @param len
 */
static void SPI_TxBuffer(uint8_t *buffer, uint16_t len)
{
  if (len > 0)
  {
    for (uint16_t i = 0; i < len; i++)
      SPI_TxByte(buffer[i]);
  }
}

/**
 * @brief SPI receive a byte
 * @return uint8_t
 */
static uint8_t SPI_RxByte(void)
{
  uint8_t dummy, data;
  dummy = 0xFF;

  //* write a dummy byte to get the receive byte

  //* check tx buf is empty
  while (spi_i2s_flag_get(SD_SPIx, SPI_I2S_TDBE_FLAG) == RESET);
  spi_i2s_data_transmit(SD_SPIx, dummy);

  //* check rx buf is full
  while (spi_i2s_flag_get(SD_SPIx, SPI_I2S_RDBF_FLAG) == RESET);
  data = spi_i2s_data_receive(SD_SPIx);

  return data;
}

/**
 * @brief SPI receive a byte via pointer
 * @param buff
 */
static void SPI_RxBytePtr(uint8_t *buff)
{
  *buff = SPI_RxByte();
}

/**========================================================================
 *                           SD functions
 *========================================================================**/

/**
 * @brief wait SD ready
 * @return uint8_t
 */
static uint8_t SD_ReadyWait(void)
{
  uint8_t res;

  /* timeout 500ms */
  Timer2 = 500;

  /* if SD goes ready, receives 0xFF */
  do
  {
    res = SPI_RxByte();
  } while ((res != 0xFF) && Timer2);

  return res;
}

/**
 * @brief power on
 *
 */
static void SD_PowerOn(void)
{
  /* transmit bytes to wake up */
  DESELECT();
  for (int i = 0; i < 10; i++)
  {
    SPI_TxByte(0xFF);
  }
}

/* power off */
/**
 * @brief
 *
 */
static void SD_PowerOff(void)
{
  PowerFlag = 0;
}

/**
 * @brief check power flag
 * @return uint8_t
 */
static uint8_t SD_CheckPower(void)
{
  return PowerFlag;
}

/**
 * @brief receive data block
 * @param buff
 * @param len
 * @return
 */
static bool SD_RxDataBlock(BYTE *buff, UINT len)
{
  uint8_t token;

  /* timeout 200ms */
  Timer1 = 200;

  /* loop until receive a response or timeout */
  do
  {
    token = SPI_RxByte();
  } while ((token == 0xFF) && Timer1);

  /* invalid response */
  if (token != 0xFE)
    return FALSE;

  /* receive data */
  do
  {
    SPI_RxBytePtr(buff++);
  } while (len--);

  /* discard CRC */
  SPI_RxByte();
  SPI_RxByte();

  return TRUE;
}

/**
 * @brief transmit data block
 * @param buff
 * @param token
 * @return
 */
static bool SD_TxDataBlock(const uint8_t *buff, BYTE token)
{
  uint8_t resp;
  uint8_t i = 0;

  /* wait SD ready */
  if (SD_ReadyWait() != 0xFF)
    return FALSE;

  /* transmit token */
  SPI_TxByte(token);

  /* if it's not STOP token, transmit data */
  if (token != 0xFD)
  {
    SPI_TxBuffer((uint8_t *)buff, 512);

    /* discard CRC */
    SPI_RxByte();
    SPI_RxByte();

    /* receive response */
    while (i <= 64)
    {
      resp = SPI_RxByte();

      /* transmit 0x05 accepted */
      if ((resp & 0x1F) == 0x05)
        return TRUE;
      i++;
    }

    return FALSE;
  }

  return TRUE; 
}

/**
 * @brief transmit command
 * @param cmd
 * @param arg
 * @return BYTE
 */
static BYTE SD_SendCmd(BYTE cmd, uint32_t arg)
{
  uint8_t crc, res;
  
  /* ACMD<n> is the command sequense of CMD55-CMD<n> */
	if (cmd & 0x80) 
  {	
		cmd &= 0x7F;
		res = SD_SendCmd(CMD55, 0);
		if (res > 1) return res;
  }

	/* Select the card and wait for ready except to stop multiple block read */
	if (cmd != CMD12) 
  {
	  DESELECT();
		if (!SELECT()) return 0xFF;
	}

  /* transmit command */
  SPI_TxByte(0x40 | cmd);           /* Command */
  SPI_TxByte((uint8_t)(arg >> 24)); /* Argument[31..24] */
  SPI_TxByte((uint8_t)(arg >> 16)); /* Argument[23..16] */
  SPI_TxByte((uint8_t)(arg >> 8));  /* Argument[15..8] */
  SPI_TxByte((uint8_t)arg);         /* Argument[7..0] */

  /* prepare CRC */
  if (cmd == CMD0)
    crc = 0x95; /* CRC for CMD0(0) */
  else if (cmd == CMD8)
    crc = 0x87; /* CRC for CMD8(0x1AA) */
  else
    crc = 1;

  /* transmit CRC */
  SPI_TxByte(crc);

  /* Skip a stuff byte when STOP_TRANSMISSION */
  if (cmd == CMD12)
    SPI_RxByte();

  /* receive response */
  uint8_t n = 10;
  do
  {
    res = SPI_RxByte();
  } while ((res & 0x80) && --n);

  return res;
}

/**========================================================================
 *                           user_diskio.c functions
 *========================================================================**/

/**
 * @brief initialize SD
 *
 */
DSTATUS SD_disk_initialize(void)
{
  uint8_t n, cmd, type, ocr[4];

  /* no disk */
  if (Stat & STA_NODISK)
    return Stat;

  spi_i2s_reset(SD_SPIx);
  SPI_Config(SPI_SLow);

  /* power on */
  SD_PowerOn();

  /* check disk type */
  type = 0;

  /* send GO_IDLE_STATE command */
  if (SD_SendCmd(CMD0, 0) == 1)
  {
    /* timeout 1 sec */
    Timer1 = 1000;

    /* SDC V2+ accept CMD8 command, http://elm-chan.org/docs/mmc/mmc_e.html */
    if (SD_SendCmd(CMD8, 0x1AA) == 1)
    {
      /* operation condition register */
      for (n = 0; n < 4; n++)
      {
        ocr[n] = SPI_RxByte();
      }

      /* voltage range 2.7-3.6V */
      if (ocr[2] == 0x01 && ocr[3] == 0xAA)
      {
        /* Wait for leaving idle state (ACMD41 with HCS bit) */
        while (Timer1 && SD_SendCmd(ACMD41, 0x40000000));	

        /* READ_OCR */
        if (Timer1 && SD_SendCmd(CMD58, 0) == 0)
        {
          /* Check CCS bit */
          for (n = 0; n < 4; n++)
          {
            ocr[n] = SPI_RxByte();
          }

          /* SDv2 (HC or SC) */
          type = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;
        }
      }
    }
    else
    {
    	/* SDv1 or MMCv3 */
			if (SD_SendCmd(ACMD41, 0) <= 1) 	
      {
				type = CT_SD1; 
        cmd = ACMD41;	/* SDv1 */
			} 
      else 
      {
				type = CT_MMC; 
        cmd = CMD1;	/* MMCv3 */
			}
			
      while (Timer1 && SD_SendCmd(cmd, 0));		/* Wait for leaving idle state */
			
      if (!Timer1 || SD_SendCmd(CMD16, 512) != 0) /* Set read/write block length to 512 */
        type = 0;	
    }
  }

  CardType = type;
  DESELECT();

  /* Clear STA_NOINIT */
  if (type)
  {
    Stat &= ~STA_NOINIT;

    spi_i2s_reset(SD_SPIx);
    SPI_Config(SPI_Fast);
  }
  else
  {
    /* Initialization failed */
    SD_PowerOff();
  }

  return Stat;
}

/**
 * @brief return disk status
 *
 */
DSTATUS SD_disk_status(void)
{
  return Stat;
}

/**
 * @brief read sector
 *
 */
DRESULT SD_disk_read(BYTE *buff, DWORD sector, UINT count)
{
  /* count should not  be 0 */
  if (!count)
    return RES_PARERR;

  /* no disk */
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;

  /* convert to byte address */
  if (!(CardType & CT_BLOCK))
    sector *= 512;

  if (count == 1)
  {
    /* READ_SINGLE_BLOCK */
    if ((SD_SendCmd(CMD17, sector) == 0) && SD_RxDataBlock(buff, 512))
      count = 0;
  }
  else
  {
    /* READ_MULTIPLE_BLOCK */
    if (SD_SendCmd(CMD18, sector) == 0)
    {
      do
      {
        if (!SD_RxDataBlock(buff, 512))
          break;
        buff += 512;
      } while (--count);

      /* STOP_TRANSMISSION */
      SD_SendCmd(CMD12, 0);
    }
  }

  DESELECT();

  return count ? RES_ERROR : RES_OK;
}

/**
 * @brief write sector
 *
 */
DRESULT SD_disk_write(const BYTE *buff, DWORD sector, UINT count)
{
  /* count should not be 0 */
  if (!count)
    return RES_PARERR;

  /* no disk */
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;

  /* write protection */
  if (Stat & STA_PROTECT)
    return RES_WRPRT;

  /* convert to byte address */
  if (!(CardType & CT_BLOCK))
    sector *= 512;

  if (count == 1)
  {
    /* WRITE_BLOCK */
    if ((SD_SendCmd(CMD24, sector) == 0) && SD_TxDataBlock(buff, 0xFE))
      count = 0;
  }
  else
  {
    /* WRITE_MULTIPLE_BLOCK */
    if (CardType & CT_SDC)
    {
      SD_SendCmd(CMD23, count); /* ACMD23 */
    }

    if (SD_SendCmd(CMD25, sector) == 0)
    {
      do
      {
        if (!SD_TxDataBlock(buff, 0xFC))
          break;
        buff += 512;
      } while (--count);

      /* STOP_TRAN token */
      if (!SD_TxDataBlock(0, 0xFD))
      {
        count = 1;
      }
    }
  }

  DESELECT();

  return count ? RES_ERROR : RES_OK;
}

/**
 * @brief ioctl
 *
 */
DRESULT SD_disk_ioctl(BYTE ctrl, void *buff)
{
  DRESULT res;
  uint8_t n, csd[16], *ptr = buff;
  WORD csize;

  res = RES_ERROR;

  if (ctrl == CTRL_POWER)
  {
    switch (*ptr)
    {
    case 0:
      SD_PowerOff(); /* Power Off */
      res = RES_OK;
      break;

    case 1:
      SD_PowerOn(); /* Power On */
      res = RES_OK;
      break;

    case 2:
      *(ptr + 1) = SD_CheckPower();
      res = RES_OK; /* Power Check */
      break;

    default:
      res = RES_PARERR;
      break;
    }
  }
  else 
  {
    /* no disk */
    if (Stat & STA_NOINIT)
      return RES_NOTRDY;

    switch (ctrl)
    {
    case GET_SECTOR_COUNT: /* Get number of sectors on the disk (WORD) */
      if ((SD_SendCmd(CMD9, 0) == 0) && SD_RxDataBlock(csd, 16))
      {
        if ((csd[0] >> 6) == 1)
        {
          /* SDC V2 */
          csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
          *(DWORD *)buff = (DWORD)csize << 10;
        }
        else
        {
          /* MMC or SDC V1 */
          n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
          csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
          *(DWORD *)buff = (DWORD)csize << (n - 9);
        }
        res = RES_OK;
      }
      break;

    case GET_SECTOR_SIZE:
      *(WORD *)buff = 512;
      res = RES_OK;
      break;

    case CTRL_SYNC :    	/* Flush write-back cache, Wait for end of internal process */
      if (SELECT()) 
        res = RES_OK;
      break;

    case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
      *ptr = CardType;
      res = RES_OK;
      break;

    case MMC_GET_CSD:
      /* SEND_CSD */
      if (SD_SendCmd(CMD9, 0) == 0 && SD_RxDataBlock(ptr, 16))
        res = RES_OK;
      break;

    case MMC_GET_CID:
      /* SEND_CID */
      if (SD_SendCmd(CMD10, 0) == 0 && SD_RxDataBlock(ptr, 16))
        res = RES_OK;
      break;

    case MMC_GET_OCR:
      /* READ_OCR */
      if (SD_SendCmd(CMD58, 0) == 0)
      {
        for (n = 0; n < 4; n++)
        {
          *ptr++ = SPI_RxByte();
        }
        res = RES_OK;
      }
      break;

    default:
      res = RES_PARERR;
      break;
    }

    DESELECT();
  }

  return res;
}

/**
 * @}
 */

/**
 * @}
 */
