#include "stm32_eval_spi_sd.h"

/**
  * @brief  Reads a block of data from the SD.
  * @param  pBuffer: pointer to the buffer that receives the data read from the
  *                  SD.
  * @param  ReadAddr: SD's internal address to read from.
  * @param  BlockSize: the SD card Data block size.
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_ReadBlock(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t BlockSize)
{
	return SD_RESPONSE_FAILURE;
}


/**
 * @brief  Writes a block on the SD
 * @param  pBuffer: pointer to the buffer containing the data to be written on
 *                  the SD.
 * @param  WriteAddr: address to write on.
 * @param  BlockSize: the SD card Data block size.
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
SD_Error SD_WriteBlock(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t BlockSize)
{
	return SD_RESPONSE_FAILURE;
}

/**
  * @brief  Read the CSD card register.
  *         Reading the contents of the CSD register in SPI mode is a simple
  *         read-block transaction.
  * @param  SD_csd: pointer on an SCD register structure
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetCSDRegister(SD_CSD* SD_csd)
{
	return SD_RESPONSE_FAILURE;
}
