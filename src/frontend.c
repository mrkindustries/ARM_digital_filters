//#include <CoOS.h>
#include "frontend.h"
#include "semihosting.h"
#include "bluetooth.h"
#include "filter.h"
//#include "utils.h"
#include "timersystem.h"
#include "global.h"
/*#include "stm32f10x_conf.h"
#include "stm32f10x_gpio.h"
//#include "stm32f10x_dac.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
//#include "misc.h"
#include "stm32f10x_exti.h"
*/

#define CHANNELS 8

int32_t canal[CHANNELS][512];
int32_t status[512];

//Se cambio OS_FlagID por unsigned char
static unsigned char data_ready_256=0; // listo para filtrar.
static unsigned char data_ready_512=0; // listo para filtrar.
static unsigned char end_filtered_256=1;
static unsigned char end_filtered_512=1;
uint8_t make_ecg=0;
static uint32_t count=0;
//static bool making_ecg= false;
uint8_t lectura[27];
uint16_t contador_lecturas;
static uint16_t lecturas = 0; //cantidad de lecturas que hizo el frontend en total.

int32_t o_signal[CHANNELS];
int32_t buffer[CHANNELS][FILTER_ORDER];


/**
 * @brief SPI peripheral initializer for the sensors drivers
 *
 * This function sets the GPIO and SPI in master mode to drive the 24bit ADC
 */
void init_spi(void) {

	SPI_InitTypeDef spi;
	GPIO_InitTypeDef gpio;

	RCC_PCLK2Config(RCC_HCLK_Div2);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO |RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);// in main()

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;//6 for ADS1298, 7 for external flash
	GPIO_Init(GPIOC, &gpio);

	CS_FRONTEND_HIGH;

	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;//spi pins for ADS1298
	GPIO_Init(GPIOA, &gpio);

	//gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	//gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;//spi pins for flash
	//GPIO_Init(GPIOB, &gpio);

	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_DataSize = SPI_DataSize_8b;
	spi.SPI_CPOL = SPI_CPOL_Low;
	spi.SPI_CPHA = SPI_CPHA_2Edge;
	spi.SPI_NSS = SPI_NSS_Soft;
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;//may test higher baudrates
	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	spi.SPI_CRCPolynomial = 0;
	SPI_Init(SPI1, &spi);//spi for ads is number 1

	SPI_Cmd(SPI1,ENABLE); //enable spi for ADS1298
}

/**
 * @brief ADS1298 Initalizer
 *
 * This function configures the MCU GPIO pins and then launch the rest
 * of the frontend initial configuration.
 */
void init_frontend(void)
{
	GPIO_InitTypeDef gpio;

	NVIC_Configuration();

	/* como era complicado soldar de a un pin en el micro
	 * solde dos pines juntos, y voy a declarar uno como salida open drain
	 * y el otro como entrada flotante de modo que no hayan conflictos eléctricos
	 * uso los pines PC0 (salida) y PC1(entrada) para el pin de /PWDN
	 * uso los pines PC2 (salida) y PC3(entrada) para el pin de /RESET
	 * */
	gpio.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpio);

	gpio.GPIO_Mode=GPIO_Mode_Out_OD;
	gpio.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_2;//RESET y PWDN
	GPIO_Init(GPIOC, &gpio);

	gpio.GPIO_Mode=GPIO_Mode_Out_PP;
	gpio.GPIO_Pin=GPIO_Pin_8;//START
	GPIO_Init(GPIOC, &gpio);

	gpio.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Pin=GPIO_Pin_9;//Data ready
	GPIO_Init(GPIOC, &gpio);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource9);

	EXTI_Configuration();

	//configuro_dac();// el dac se puede utilizar para ver las señales en tiempo real
	// si se lo utiliza se debe tener en cuenta que usa el mismo pin que el medidor
	// de batería, por lo cual no se pueden utilizar simultaneamente

	PWDN_FRONTEND_HIGH;
	RESET_FRONTEND_HIGH;

	power_up_sequence();

	////-------------------------->CAMBIAR!
	//data_ready_256 = CoCreateFlag(1,0);
	//data_ready_512 = CoCreateFlag(1,0);

	sleep(30);
	//CoTickDelay(30);
	//making_ecg = 1; 	// se tiene que setear cuando se empieza a realizar un ecg.
							// y unset cuando se termina de realizar el ecg.
	//-------------------------------
	contador_lecturas=0; //debería ser seteado a 0 cuando se comienza a realizar el ecg.
	ini_ads();
}

/**
 * @brief Send a command to the ADS1298
 *
 *	Before calling check that CS_FRONTEND is low
 *
 */
uint8_t ads_comando(uint8_t comando)
{
	while (SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET)
		;
	SPI_I2S_SendData(SPI1,comando);
	while (SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY))
		;
	return SPI_I2S_ReceiveData(SPI1);
}


/**
 * @brief Drives RESET and PDWN lines for ADS1298 initialization
 *
 * Esta función sigue las instrucciones brindadas
 * por texas en el datasheet del ads1298
 * página 78
 */
void power_up_sequence(void)
{

	//Follow power up sequencing recommended by datasheet
	CS_FRONTEND_HIGH;
	RESET_FRONTEND_HIGH;

	sleep(4);
	//CoTickDelay(4); //delay de 32ms minimo
	RESET_FRONTEND_LOW;

	delay_count(35); //delay 1us minimo
	RESET_FRONTEND_HIGH;
	delay_count(350);//delay 10us minimo
	//end Follow power up sequencing

	//CLKSEL Pin = 1 by hardware

	PWDN_FRONTEND_HIGH;
	sleep(100);
	//CoTickDelay(100);//Delay for Power-On Reset and Oscillator Start-Up 1 seg

	//Issue Reset Pulse and wait for 18 tclks
	RESET_FRONTEND_LOW;
	delay_count(35); //delay 1us minimo
	RESET_FRONTEND_HIGH;
	delay_count(350);//delay 10us minimo

	//Mando el comando SDATAC, detiene las conversiones para poder escribir registros
	CS_FRONTEND_LOW;

	ads_comando(COMANDO_SDATAC);
	//Delay min 4tclks (2useg)
	delay_count(70);
	CS_FRONTEND_HIGH;
}

/**
 * @brief Spinlock
 *
 * It hangs the processor for a (determined) while.
 */
void delay_count(uint16_t count)
{
	while(count > 0)
	{
		count--;
	}
}

/**
 * @brief Read ADS1298 internal register
 *
 * ads_leo_registro recibe la direccion del registro y devuelve
 * su contenido.
 */
uint8_t ads_leo_registro(uint8_t direccion)
{
	uint8_t dato;

	CS_FRONTEND_LOW;

	direccion=direccion&0x1f;
	direccion|=COMANDO_RREG;

	ads_comando(direccion);

	ads_comando(0);

	dato=ads_comando(0);
	delay_count(70);
	CS_FRONTEND_HIGH;

	return dato;
}

/**
 * @brief Write ADS1298 internal register
 *
 * ads_escribir_registro recibe la direccion del registro y el dato
 * y escribe el dato en el registro
 */
void ads_escribir_registro(uint8_t direccion,uint8_t dato)
{
	CS_FRONTEND_LOW;

	direccion = direccion & 0x1f;
	//formo comando en la misma variable direccion
	direccion|=COMANDO_WREG;

	ads_comando(direccion);
	ads_comando(0);
	ads_comando(dato);

	CS_FRONTEND_HIGH;
}

/**
 * @brief Test communications
 *
 * Esta funcion lee el ID number y devuelve 1 si es el AD1298 y 0 en caso contrario
 * con esto puedo verificar el estado de la comunicacion.
 * Genial para verificar por JTAG que el ADS funciona apenas salido del horno
 */
uint8_t prueba_comunicacion_ads(void)
{
	uint8_t id;

	id=ads_leo_registro(ADS1298_ID);

	if(id==0x92)
		return 1;
	else
		return 0;
}

/**
 * @brief Configure channels
 *
 * ini_ads configura los 8 canales para adquirir una señal
 * desde el puerto, con una ganacia del amplificador de entrada unitaria
 *
 * además configura el circuito de generación de la realimentación RLD
 *
 * y por último configura la generacion del terminal de wilson WCT
 */
void ini_ads(void)
{
	//SET PDB_REFBUF=1
	ads_escribir_registro(ADS1298_CONFIG3,0xC0);

	sleep(15);
	//CoTickDelay(15);//internal reference settle time 150ms
	//SET HR MODE y 500 SPS
	ads_escribir_registro(ADS1298_CONFIG1, HR_MODE | DATA_RATE_500 );

	//activo señales de testeo
	//ads_escribir_registro(ADS1298_CONFIG2,0x10);
	//CHn_NORMAL_ELECTRODE
	//configuro canales

	ads_escribir_registro(ADS1298_CH1SET,CHn_POWER_UP|CHn_GAIN_12|CHn_NORMAL_ELECTRODE);
	ads_escribir_registro(ADS1298_CH2SET,CHn_POWER_UP|CHn_GAIN_12|CHn_NORMAL_ELECTRODE);
	ads_escribir_registro(ADS1298_CH3SET,CHn_POWER_UP|CHn_GAIN_12|CHn_NORMAL_ELECTRODE);
	ads_escribir_registro(ADS1298_CH4SET,CHn_POWER_UP|CHn_GAIN_12|CHn_NORMAL_ELECTRODE);
	ads_escribir_registro(ADS1298_CH5SET,CHn_POWER_UP|CHn_GAIN_12|CHn_NORMAL_ELECTRODE);
	ads_escribir_registro(ADS1298_CH6SET,CHn_POWER_UP|CHn_GAIN_12|CHn_NORMAL_ELECTRODE);
	ads_escribir_registro(ADS1298_CH7SET,CHn_POWER_UP|CHn_GAIN_12|CHn_NORMAL_ELECTRODE);
	ads_escribir_registro(ADS1298_CH8SET,CHn_POWER_UP|CHn_GAIN_12|CHn_NORMAL_ELECTRODE);


	//configuro RLD

		//ads_escribir_registro(ADS1298_RLD_SENSP,0x06);//selecciono los canales positivos 2 y 3 (LL, LA)
		//ads_escribir_registro(ADS1298_RLD_SENSN,0x02);//selecciono el canal negativo 2 (RA) ,este electrodo también está en el 3
														//por lo cual habría que probar si disminuye el ruido conectando los dos simultaneamente
		//ads_escribir_registro(ADS1298_CONFIG3,0xCC);//PDB_REFBUF=1, RLD_REF_INT=1, /PD_RLD=1, enciendo buffer RLD y configuro referencia interna

	//configuro WCT

//	ads_escribir_registro(ADS1298_WCT1,0x0B);//enciendo WCTA amplifier y lo conecto a RA
//	ads_escribir_registro(ADS1298_WCT2,0xD4);//enciendo WCTB amplifier y lo conecto a LA, enciendo WCTC amplifier y lo conecto a LL


	//habilito el inicio de las conversiones
	START_CONVERTING;

	//mando comando RDATAC
	CS_FRONTEND_LOW;
	ads_comando(COMANDO_RDATAC);
	delay_count(70);		//Delay min 4tclks (2useg)
	CS_FRONTEND_HIGH;

}



/*
 * configuro la interrupcion externa cuando llegan nuevas muestras
 *
 */
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	// Set the Vector Table base location at 0x08000000
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

	// Configure one bit for preemption priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	// Enable the EXTI9_5 Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void EXTI_Configuration(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    EXTI_InitStructure.EXTI_Line = EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

/*
void configuro_dac(void)
{
	GPIO_InitTypeDef gpio;
	DAC_InitTypeDef DAC_InitStructure;

	// DAC Periph clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	gpio.GPIO_Mode = GPIO_Mode_AIN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &gpio);

	// DAC channel1 Configuration
	  DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software;
	  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	  DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits8_0;
	  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	  // Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is
	     //automatically connected to the DAC converter.
	  DAC_Cmd(DAC_Channel_1, ENABLE);

	  // Set DAC Channel1 DHR12L register
	  DAC_SetChannel1Data(DAC_Align_12b_R, 0x07FF);
}
*/


void startEcg ( uint32_t seconds )
{
	uint16_t register i,j = 0;
	int16_t pos = 0;		//posicion del buffer a insertar el proximo elemento a filtrar.
	uint16_t size = 250;	//porcion de la señal a filtrar.
	int count = 0;
//	int16_t cantidad_lecturas = 0;


	data_ready_256=0;
	data_ready_512=0;


	// Inicializo los buffers en 0.
	//init_buffers(buffer);

	uint32_t start_time = getMsecs() + seconds*1000;
	lecturas=0;
	contador_lecturas=0;
//	send_number(lecturas);
	make_ecg=1;
	bool send;
	while(getMsecs() < start_time){
		//if(count==2) break;

		//esperar por  los primeros 256 datos.
		while(!data_ready_256);
		data_ready_256 =0;
		end_filtered_256=0;
		send=false;
		for(i = 0; i < size; i++){
			if(send)
				send_Message_Header(DATA_MESSAGE);
			for(j=0;j<CHANNELS;j++){
				band_reject(&(canal[j][i]),&(buffer[j][0]), &pos, kernel, &(o_signal[j]));
				if(send){
					count++;
					send_number(o_signal[j]);
				}
			}
			send=!send;
			next(&pos);
		}
		end_filtered_256=1;
		send=false;
		//esperar por los segundos 256 datos.
		while(!data_ready_512);
		data_ready_512 =0;
		end_filtered_512=0;

		for(i = size; i < (size*2-1); i++){
			if(send)
				send_Message_Header(DATA_MESSAGE);
			for(j=0;j<CHANNELS;j++)
			{
				band_reject(&(canal[j][i]),&(buffer[j][0]), &pos, kernel, &(o_signal[j]));
				if(send){
					count++;
					send_number(o_signal[j]);
				}
			}
			send=!send;
			next(&pos);
		 }
		end_filtered_512=1;
	//	cantidad_lecturas = lecturas;
		//count++;
	}
	make_ecg=0;
	//send_number(count);

	char buffer[50];
	sprintf(buffer, "%d", count);
	SH_SendString(buffer);
	SH_SendChar('\n');
	send_Message_Header(FINISHED_MESSAGE);
	//finish();
}
/**
 * @brief Esto supongo q va a ser reemplazado por DMA, asi que ni me gasto.
 *
 */
void EXTI9_5_IRQHandler(void)
{
	int32_t medicion;
	uint8_t register i;
	uint8_t j;

	if ((EXTI_GetITStatus(EXTI_Line9) != RESET) && make_ecg)
	{
		count++;

		CS_FRONTEND_LOW;

		/** Fuerzo la entrega de datos. Por alguna razón es la unica forma en que le anduvo a Lucas */
		ads_comando(COMANDO_RDATA);

		/**leo el status y todos los canales */
		for(i=0;i<27;++i)
		{
			lectura[i]=ads_comando(0);
		}

		status[contador_lecturas]=(lectura[0]<<16)|(lectura[1]<<8)|(lectura[2]);

		for(i=0;i<8;++i)
		{
			j=i*3+3;
			medicion=(lectura[j]<<16)|(lectura[j+1]<<8)|(lectura[j+2]);
			if(medicion&0x00800000)
				medicion|=0xff000000;

			canal[i][contador_lecturas]=(medicion); //aca hay que normalizar los valores.


		}

		/*medicion=(lectura[6]<<16)|(lectura[7]<<8)|(lectura[8]);
		if(medicion&0x00800000)
			medicion|=0xff000000;

		canal2[contador_lecturas]=medicion;


		medicion=(lectura[9]<<16)|(lectura[10]<<8)|(lectura[11]);
		if(medicion&0x00800000)
			medicion|=0xff000000;

		canal3[contador_lecturas]=medicion;

		medicion=(lectura[24]<<16)|(lectura[25]<<8)|(lectura[26]);
		if(medicion&0x00800000)
			medicion|=0xff000000;

		canal8[contador_lecturas]=medicion;
*/
		/*
		//Muestro Valor en el DAC
		medicion<<=8;//amplifico por 1024
		medicion+=0x00800000;//sumo un offset para evitar señales negativas
		medicion>>=12;//adapto el valor a la precision del DAC
		DAC_SetChannel1Data(DAC_Align_12b_R,medicion);//cargo el valor al DAC
		DAC_SoftwareTriggerCmd(DAC_Channel_1,ENABLE);//convierto
		 */

		contador_lecturas+=1;
		lecturas++;


		if(contador_lecturas == 249){
			if(end_filtered_512==0){
				SH_SendString("Error!\n");
			}
			data_ready_256 = 1;
		}
		if (contador_lecturas == 499){
			if(end_filtered_256==0){
				SH_SendString("Error!\n");
			}
			data_ready_512 = 1;
			contador_lecturas = 0;
		}

		//EXTI_ClearITPendingBit(EXTI_Line9);
		//delay_count(70);
        CS_FRONTEND_HIGH;
	}

	EXTI_ClearITPendingBit(EXTI_Line9);
	delay_count(70); //probar sacar y ver si anda.

}
