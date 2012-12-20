/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : mass_mal.c
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Medium Access Layer interface
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"

// #include "stm32_eval_spi_sd.h"


#include "mass_mal.h"
//#include "stm32_eval.h"
#include "ECG_board.h"
#include "diskio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Mass_Memory_Size[2];
uint32_t Mass_Block_Size[2];
uint32_t Mass_Block_Count[2];
__IO uint32_t Status = 0;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Init(uint8_t lun)
{
  uint16_t status = MAL_OK;

  switch (lun)
  {
    case 0:
//      Status = SD_Init();
    	Status = disk_initialize (lun);
      break;

    default:
      return MAL_FAIL;
  }
  return status;
}
/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{
	DWORD sector;
	BYTE count;

  switch (lun)
  {
    case 0:
      //Status = SD_WriteBlock((uint8_t*)Writebuff, Memory_Offset, Transfer_Length);
    	sector = (DWORD) (Memory_Offset / 512);
    	count = (uint8_t)(Transfer_Length / 512);

    	Status = disk_write (lun, (uint8_t*)Writebuff, sector , count );

		//sFLASH_WriteBuffer((uint8_t*)Writebuff, Memory_Offset, Transfer_Length);
      break;

    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{
//	DWORD sector;
//	BYTE count;

  switch (lun)
  {
    case 0:
  //    Status = SD_ReadBlock((uint8_t*)Readbuff, Memory_Offset, Transfer_Length);

//    	sector = (DWORD) (Memory_Offset / 512);
//    	count = (uint8_t)(Transfer_Length / 512);

      //Status = disk_read (lun, Readbuff, sector , count );
      sFLASH_ReadBuffer((uint8_t*)Readbuff, Memory_Offset, Transfer_Length);

      break;
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_GetStatus (uint8_t lun)
{

//  uint32_t temp_block_mul = 0;

//  uint32_t DeviceSizeMul = 0;

  if (lun == 0)
  {

     
//    SD_GetCSDRegister(&SD_csd);
//    DeviceSizeMul = SD_csd.DeviceSizeMul + 2;
//    temp_block_mul = (1 << SD_csd.RdBlockLen)/ 512;
//    Mass_Block_Count[0] = ((SD_csd.DeviceSize + 1) * (1 << (DeviceSizeMul))) * temp_block_mul;
      Mass_Block_Size[0] = SECTOR_SIZE;
//    Mass_Memory_Size[0] = (Mass_Block_Count[0] * Mass_Block_Size[0]);

      Mass_Block_Count[0] = 8*512*1024/SECTOR_SIZE;
      Mass_Memory_Size[0] = 8*512*1024;


      Mass_Memory_Size[0] = Mass_Block_Count[0] * Mass_Block_Size[0];
      return MAL_OK;

  }
  return MAL_FAIL;
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
