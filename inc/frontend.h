#ifndef FRONTEND_H
#define FRONTEND_H

//#include "stdint.h"
//#include <stdbool.h>
#include "ecg.h"
/*
 * COMANDOS PARA USAR CON LA FUNCION ads_comando();
 */
#define COMANDO_WAKEUP 	0x02
#define COMANDO_STANDBY 0x04
#define COMANDO_RESET	0x06
#define COMANDO_START	0x08
#define COMANDO_STOP	0x0A
#define COMANDO_RDATAC 	0x10 //read data continuosly
#define COMANDO_SDATAC	0x11//stop read data continuosly
#define COMANDO_RDATA	0x12
#define COMANDO_RREG	0x20//read n registers
#define COMANDO_WREG	0x40//write n registers

/*
 * configuraciones posibles a ADS1298_CONFIG1
 */
#define HR_MODE 		0x80
#define LP_MODE 		0x00
#define DATA_RATE_500  	0x06
#define DATA_RATE_1000 	0x05
#define DATA_RATE_2000 	0x04
#define DATA_RATE_4000 	0x03

/* Configuraciones de los canales
 *
 */
//on/off
#define CHn_POWER_DOWN		0x80
#define CHn_POWER_UP		0x00
//ganancias
#define CHn_GAIN_1			0x10
#define CHn_GAIN_2			0x20
#define CHn_GAIN_3			0x30
#define CHn_GAIN_4			0x40
#define CHn_GAIN_6			0x00
#define CHn_GAIN_8			0x50
#define CHn_GAIN_12			0x60
//tipo de entrada
#define CHn_NORMAL_ELECTRODE	0x00
#define CHn_INPUT_SHORTED		0x01
#define CHn_SUPPLY				0x03
#define CHn_TEMPERATURE			0x04
#define CHn_TEST_SIGNAL			0x05


//direcciones de los registros
#define ADS1298_ID			0x00
#define ADS1298_CONFIG1		0x01
#define ADS1298_CONFIG2		0x02
#define ADS1298_CONFIG3		0x03
#define ADS1298_LOFF		0x04
#define ADS1298_CH1SET		0x05
#define ADS1298_CH2SET		0x06
#define ADS1298_CH3SET		0x07
#define ADS1298_CH4SET		0x08
#define ADS1298_CH5SET		0x09
#define ADS1298_CH6SET		0x0a
#define ADS1298_CH7SET		0x0b
#define ADS1298_CH8SET		0x0c
#define ADS1298_RLD_SENSP	0x0d
#define ADS1298_RLD_SENSN	0x0e
#define ADS1298_LOFF_SENSP	0x0f
#define ADS1298_LOFF_SENSN	0x10
#define ADS1298_LOFF_FLIP	0x11
#define ADS1298_LOFF_STATP	0x12
#define ADS1298_LOFF_STATN	0x13
#define ADS1298_GPIO		0x14
#define ADS1298_PACE		0x15
#define ADS1298_RESP		0x16
#define ADS1298_CONFIG4		0x17
#define ADS1298_WCT1		0x18
#define ADS1298_WCT2		0x19


#define CS_FRONTEND_LOW GPIO_ResetBits(GPIOC,GPIO_Pin_6)
#define CS_FRONTEND_HIGH GPIO_SetBits(GPIOC,GPIO_Pin_6)

#define RESET_FRONTEND_LOW GPIO_ResetBits(GPIOC,GPIO_Pin_2)
#define RESET_FRONTEND_HIGH GPIO_SetBits(GPIOC,GPIO_Pin_2)

#define PWDN_FRONTEND_LOW GPIO_ResetBits(GPIOC,GPIO_Pin_0)
#define PWDN_FRONTEND_HIGH GPIO_SetBits(GPIOC,GPIO_Pin_0)

#define START_CONVERTING GPIO_SetBits(GPIOC,GPIO_Pin_8)
#define STOP_CONVERTING GPIO_ResetBits(GPIOC,GPIO_Pin_8)

#define LED_ON GPIO_SetBits(GPIOB,GPIO_Pin_6);
#define LED_OFF GPIO_ResetBits(GPIOB,GPIO_Pin_6);



//bool making_ecg;
//--------------------------------------


void configuro_dac(void);
void init_spi();
void init_frontend();
void startEcg( uint32_t seconds );
void power_up_sequence(void);
void delay_count (uint16_t delay);
uint8_t ads_comando(uint8_t comando);
uint8_t prueba_comunicacion_ads(void);
void ads_escribir_registro(uint8_t direccion,uint8_t dato);
void ini_ads(void);
void EXTI_Configuration(void);
void NVIC_Configuration(void);
signed long int leo_canales(void);



#endif
