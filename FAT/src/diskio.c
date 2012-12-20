/*-----------------------------------------------------------------------*/
/* MMC/SDSC/SDHC (in SPI mode) control module for STM32 Version 1.1.6    */
/* (C) Martin Thomas, 2010 - based on the AVR MMC module (C)ChaN, 2007   */
/*-----------------------------------------------------------------------*/

/* Copyright (c) 2010, Martin Thomas, ChaN
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */


#include "stm32f10x.h"
#include "ffconf.h"
#include "diskio.h"
#include "stm32_eval_spi_flash.h"
#include "SPI_flash.h"
#include "ECG_board.h"



#if (_MAX_SS != 512) || (_FS_READONLY == 0)
#define STM32_SD_DISK_IOCTRL   1
#else
#define STM32_SD_DISK_IOCTRL   0
#endif

/*--------------------------------------------------------------------------

   Module Private Functions and Variables

---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
DWORD Timer1, Timer2;	/* 100Hz decrement timers */


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive number (0) */
)
{
//	BYTE n, cmd, ty, ocr[4];

	if (drv) return STA_NOINIT;			/* Supports only single drive */
//	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	if(Stat == 0)		/* check if disk is already initialized*/
		return Stat;
	else
		sFLASH_Init();
	
	/* If there is any extra code to initialize the flash memory, must be put here*/
	
	
	
	// power_on();							/* Force socket power on and initialize interface */
	// interface_speed(INTERFACE_SLOW);
	// for (n = 10; n; n--) rcvr_spi();	/* 80 dummy clocks */

	// ty = 0;
	// if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		// Timer1 = 100;						/* Initialization timeout of 1000 milliseconds */
		// if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDHC */
			// for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();		/* Get trailing return value of R7 response */
			// if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at VDD range of 2.7-3.6V */
				// while (Timer1 && send_cmd(ACMD41, 1UL << 30));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				// if (Timer1 && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					// for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					// ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				// }
			// }
		// } else {							/* SDSC or MMC */
			// if (send_cmd(ACMD41, 0) <= 1) 	{
				// ty = CT_SD1; cmd = ACMD41;	/* SDSC */
			// } else {
				// ty = CT_MMC; cmd = CMD1;	/* MMC */
			// }
			// while (Timer1 && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			// if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
				// ty = 0;
		// }
	// }
	// CardType = ty;
	// release_spi();

	// if (ty) {			/* Initialization succeeded */
		// Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
		// interface_speed(INTERFACE_FAST);
	// } else {			/* Initialization failed */
		// power_off();
	// }

	Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive number (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,			/* Physical drive number (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;


	if (count == 1) {	/* Single block read */
		sector *= SECTOR_SIZE;	/* Convert to byte address */
		sFLASH_ReadBuffer(buff, sector, SECTOR_SIZE);
		//rcvr_datablock(buff, 512)
		--count;
		
	}
	else {		/* READ_MULTIPLE_BLOCK */
		do {
			sector *= SECTOR_SIZE;	/* Convert to byte address */
			sFLASH_ReadBuffer(buff, sector, SECTOR_SIZE);
			buff += SECTOR_SIZE;
			++sector;
		} while (--count);
	}
//	release_spi();

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _FS_READONLY == 0

DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	DWORD erase_sector[11];		//Last 8 items of array are used to store the cache data
	BYTE auxbuff[SECTOR_SIZE], i;

	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;
	sFLASH_DisableWriteProtection();

	if (count == 1)
	{	/* Single block read */

		//Aca tengo que ver como hago para que el sector donde voy a escribir este borrado.
		//yo puedo borrar de a 4Kb y el sector es de 512b, con lo cual voy a tener que poner
		//la info que sirve, dentro de un buffer

		//we assume that the if the first 32 bytes of the memory region we want to write,
		//are clear, then the hole 512 bytes are clear and we don't need to clear
		//all the 4kb erasable sector

//		if(sector > 8)
//		{
		//check if the first 32B are clear, if yes, jump
		sFLASH_ReadBuffer((BYTE*)&auxbuff, sector*SECTOR_SIZE, 32);	//read the first 32 bytes
		if(compare((BYTE*)&auxbuff) != RES_OK)		//erase sector
		{
			erase_sector[0]= (uint16_t)sector & 0xFFF8;	//base sector to be clear
			erase_sector[1]= ((uint16_t)sector & 0xFFF8) +1;//(unsigned int)sector/8 +1;
			erase_sector[10]= sector - ((uint16_t)sector & 0xFFF8);			//relative sector to be write

//			erase_sector[0]= (uint16_t)sector/8;	//base sector to be clear
//			erase_sector[1]= (uint16_t)sector/8 +1;
//			erase_sector[10]= sector - erase_sector[0]*8;			//relative sector to be write
//			sFLASH_WriteBuffer(buff, erase_sector[1]*SECTOR_SIZE+LAST_BLOCK, SECTOR_SIZE);
			for(i=0; i<8; ++i)
			{
				erase_sector[2+i]= 0;
				if(i == erase_sector[10])
					continue;
				sFLASH_ReadBuffer((BYTE*)&auxbuff, (erase_sector[0]+i)*SECTOR_SIZE, SECTOR_SIZE);	//read the first 32 bytes
				if(compare((BYTE*)&auxbuff) != RES_OK)		//erase sector
				{
					erase_sector[2+i]= i+1;		//el +1 es un artilujio para poder saber si ese sector lo cache o no
					sFLASH_WriteBuffer((BYTE*)&auxbuff, (erase_sector[2+i]-1)*SECTOR_SIZE+LAST_BLOCK, SECTOR_SIZE);
				}
			}

			disk_ioctl (drv, CTRL_ERASE_SECTOR, erase_sector);
			for(i=0; i<8; ++i)
			{
				if(erase_sector[2+i]== 0)
					continue;
				sFLASH_ReadBuffer((BYTE*)&auxbuff, (erase_sector[2+i]-1)*SECTOR_SIZE+LAST_BLOCK, SECTOR_SIZE);	//read the first 32 bytes
				sFLASH_WriteBuffer((BYTE*)&auxbuff, (erase_sector[2+i]-1+erase_sector[0])*SECTOR_SIZE, SECTOR_SIZE);
			}

			erase_sector[0]=LAST_BLOCK/SECTOR_SIZE;
			erase_sector[1]=LAST_BLOCK/SECTOR_SIZE+1;
			disk_ioctl (drv, CTRL_ERASE_SECTOR, erase_sector);	//erase last sector
		}
//		}
		sector *= SECTOR_SIZE;	/* Convert to byte address */
		sFLASH_WriteBuffer(buff, sector, SECTOR_SIZE);
		--count;
	}
	else {		/* READ_MULTIPLE_BLOCK */
		do {
			sFLASH_WriteBuffer(buff, sector*SECTOR_SIZE, SECTOR_SIZE);
			buff += SECTOR_SIZE;
			++sector;
		} while (--count);
	}
//	release_spi();

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

// #if (STM32_SD_DISK_IOCTRL == 1)
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = 0;
	DWORD start_sector, end_sector, sector;
	DWORD *ptr = (DWORD*)buff;
	WORD count;
	
	if (drv) return RES_PARERR;

	res = RES_ERROR;

	if (Stat & STA_NOINIT) return RES_NOTRDY;

	switch (ctrl) {
	case CTRL_SYNC :		/* Make sure that no pending write process */
		sFLASH_WaitForWriteEnd();
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
		*(DWORD*)buff = 8*512*1024/SECTOR_SIZE;
		res = RES_OK;
		break;

	case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
		*(WORD*)buff = SECTOR_SIZE;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		*(DWORD*)buff = 4*1024/SECTOR_SIZE;
		res = RES_OK;
		break;

	case CTRL_ERASE_SECTOR:				/*ojo, un sector es de 512 bytes, pero aca lo menos que puedo borrar es 4k*/

		start_sector = ptr[0]& 0xFFFFF8;		//base sector to be clear
		end_sector = ptr[1]& 0xFFFFF8;			//base sector to be clear
		count = (DWORD)(end_sector - start_sector);
		sector = start_sector;
		if (count <= 0)
			count = 1;
	//	start_sector = ptr[0];
	//	end_sector = ptr[1];
	//	count = end_sector - start_sector;
	//	sector = start_sector;
		if (count == 1) {	/* Single block read */
			sector *= SECTOR_SIZE;	/* Convert to byte address */
			sFLASH_EraseSector(sector);	//may be we have to convert the value of the direction, to a physical diretction
		}
		else {		/* ERASE_MULTIPLE_BLOCK */
	//		sector = start_sector/8;
	//		count = end_sector/8 - sector;
	//		do {
			count += 8;
			while(count)
			{		/* Convert to byte address */
			//	sFLASH_EraseSector(sector * SECTOR_SIZE*8);	//may be we have to convert the value of the direction, to a physical diretction
				sFLASH_EraseSector(sector * SECTOR_SIZE);	//may be we have to convert the value of the direction, to a physical diretction
//				++sector;
				sector += 8;
				count -= 8;
			}
	//		} while (--count);
		}
		res = RES_OK;
		break;
		
	default:
		res = RES_PARERR;
	}

	return res;
}
// #endif /* _USE_IOCTL != 0 */


// /*-----------------------------------------------------------------------*/
// /* Device Timer Interrupt Procedure  (Platform dependent)                */
// /*-----------------------------------------------------------------------*/
// /* This function must be called in period of 10ms                        */

// RAMFUNC void disk_timerproc (void)
// {
	// static DWORD pv;
	// DWORD ns;
	// BYTE n, s;


	// n = Timer1;                /* 100Hz decrement timers */
	// if (n) Timer1 = --n;
	// n = Timer2;
	// if (n) Timer2 = --n;

	// ns = pv;
	// pv = socket_is_empty() | socket_is_write_protected();	/* Sample socket switch */

	// if (ns == pv) {                         /* Have contacts stabled? */
		// s = Stat;

		// if (pv & socket_state_mask_wp)      /* WP is H (write protected) */
			// s |= STA_PROTECT;
		// else                                /* WP is L (write enabled) */
			// s &= ~STA_PROTECT;

		// if (pv & socket_state_mask_cp)      /* INS = H (Socket empty) */
			// s |= (STA_NODISK | STA_NOINIT);
		// else                                /* INS = L (Card inserted) */
			// s &= ~STA_NODISK;

		// Stat = s;
	// }
// }


/*------------------------------------------------------------------------------*/
/* Compare first 32 bytes of a buffer and check if zero	                 		*/
/*------------------------------------------------------------------------------*/
/* As parameter, expects a pointer to the buffer.                              	*/
/* Returns FR_OK if the first 32 bytes are clear, if not it returns FR_ERROR	*/

DRESULT compare(BYTE *auxbuff)
{
	BYTE i=0;
	while (*auxbuff == 0)
	{
		if(i == 32)
			return RES_OK;
		++i;
	}
	return RES_ERROR;
}
