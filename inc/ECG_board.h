#ifndef BLUETOOTH_H
#define BLUETOOTH_H
#define SECTOR_SIZE _MAX_SS		// in ffconf.h the MAX_SS (max sector size) is set to 4096


#include "ffconf.h"
#include "ff.h"
#include "diskio.h"
#include "ecg.h"


//Se cambio OS_FlagID por unsigned char
volatile unsigned char flagID, flag1, flagdato;

/** @addtogroup STM32_EVAL_SPI_FLASH
  * @{
  */

/** @defgroup STM32_EVAL_SPI_FLASH_Exported_Types
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32_EVAL_SPI_FLASH_Exported_Constants
  * @{
  */
/**
  * @brief  M25P SPI Flash supported commands
  */
#define sFLASH_CMD_WRITE          0x02  /*!< Write to Memory instruction */
#define sFLASH_CMD_AAI			  0xAD	/*!< Auto Address Increment instruction */
#define sFLASH_CMD_WRSR           0x01  /*!< Write Status Register instruction */
#define sFLASH_CMD_WRDI			  0x04  /*!< Write Disable instruction */
#define sFLASH_CMD_WREN           0x06  /*!< Write enable instruction */
#define sFLASH_CMD_READ           0x03  /*!< Read from Memory instruction */
#define sFLASH_CMD_RDSR           0x05  /*!< Read Status Register instruction  */
#define sFLASH_CMD_RDID           0x9F  /*!< Read identification */
#define sFLASH_CMD_SE             0x20  /*!< Sector Erase instruction */
#define sFLASH_CMD_BE             0xC7  /*!< Bulk Erase instruction */

#define sFLASH_WIP_FLAG           0x01  /*!< Write In Progress (WIP) flag */

#define sFLASH_DUMMY_BYTE         0xA5
#define sFLASH_SPI_PAGESIZE       0x1000

#define sFLASH_M25P128_ID         0x202018
#define sFLASH_M25P64_ID          0x202017
#define sFALSH_SST25W40_ID		  0xBF2504


/**
  * @}
  */

/** @addtogroup STM32100B_EVAL_LOW_LEVEL_SD_SPI
  * @{
  */
/**
  * @brief  SD SPI Interface pins
  */
#define SD_SPI                           SPI1
#define SD_SPI_CLK                       RCC_APB2Periph_SPI1
#define SD_SPI_SCK_PIN                   GPIO_Pin_5                  /* PA.05 */
#define SD_SPI_SCK_GPIO_PORT             GPIOA                       /* GPIOA */
#define SD_SPI_SCK_GPIO_CLK              RCC_APB2Periph_GPIOA
#define SD_SPI_MISO_PIN                  GPIO_Pin_6                  /* PA.06 */
#define SD_SPI_MISO_GPIO_PORT            GPIOA                       /* GPIOA */
#define SD_SPI_MISO_GPIO_CLK             RCC_APB2Periph_GPIOA
#define SD_SPI_MOSI_PIN                  GPIO_Pin_7                  /* PA.07 */
#define SD_SPI_MOSI_GPIO_PORT            GPIOA                       /* GPIOA */
#define SD_SPI_MOSI_GPIO_CLK             RCC_APB2Periph_GPIOA
#define SD_CS_PIN                        GPIO_Pin_12                 /* PC.12 */
#define SD_CS_GPIO_PORT                  GPIOC                       /* GPIOC */
#define SD_CS_GPIO_CLK                   RCC_APB2Periph_GPIOC
#define SD_DETECT_PIN                    GPIO_Pin_7                  /* PE.07 */
#define SD_DETECT_GPIO_PORT              GPIOE                       /* GPIOE */
#define SD_DETECT_GPIO_CLK               RCC_APB2Periph_GPIOE

/**
  * @}
  */

/** @addtogroup STM32100B_EVAL_LOW_LEVEL_M25P_FLASH_SPI
  * @{
  */
/**
  * @brief  M25P FLASH SPI Interface pins
  */
#define sFLASH_SPI                       SPI2
#define sFLASH_SPI_CLK                   RCC_APB1Periph_SPI2
#define sFLASH_SPI_SCK_PIN               GPIO_Pin_13                  /* PB.13 */
#define sFLASH_SPI_SCK_GPIO_PORT         GPIOB
#define sFLASH_SPI_SCK_GPIO_CLK          RCC_APB2Periph_GPIOB
#define sFLASH_SPI_MISO_PIN              GPIO_Pin_14                  /* PB.14 */
#define sFLASH_SPI_MISO_GPIO_PORT        GPIOB
#define sFLASH_SPI_MISO_GPIO_CLK         RCC_APB2Periph_GPIOB
#define sFLASH_SPI_MOSI_PIN              GPIO_Pin_15                  /* PB.15 */
#define sFLASH_SPI_MOSI_GPIO_PORT        GPIOB
#define sFLASH_SPI_MOSI_GPIO_CLK         RCC_APB2Periph_GPIOB
#define sFLASH_CS_PIN                    GPIO_Pin_7                  /* PC.07 */
#define sFLASH_CS_GPIO_PORT              GPIOC
#define sFLASH_CS_GPIO_CLK               RCC_APB2Periph_GPIOC


/**
  * @}
  */

/** @defgroup STM32_EVAL_SPI_FLASH_Exported_Macros
  * @{
  */
/**
  * @brief  Select sFLASH: Chip Select pin low
  */
#define sFLASH_CS_LOW()       GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)
/**
  * @brief  Deselect sFLASH: Chip Select pin high
  */
#define sFLASH_CS_HIGH()      GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)
/**
  * @}
  */



/** @defgroup STM32_EVAL_SPI_FLASH_Exported_Functions
  * @{
  */
/**
  * @brief  High layer functions
  */
void sFLASH_DeInit(void);
void sFLASH_Init(void);
void sFLASH_EraseSector(uint32_t SectorAddr);
void sFLASH_EraseBulk(void);
void sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_WriteBuffer(const uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint32_t sFLASH_ReadID(void);
void sFLASH_StartReadSequence(uint32_t ReadAddr);

/**
  * @brief  Low layer functions
  */
uint8_t sFLASH_ReadByte(void);
void sFLASH_SendByte(uint8_t byte);
uint16_t sFLASH_SendHalfWord(uint16_t HalfWord);
void sFLASH_WriteEnable(void);
void sFLASH_WaitForWriteEnd(void);



#endif
