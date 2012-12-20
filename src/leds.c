#include "leds.h"

void configure_LEDs_GPIO()
{
	/* LED1 está en PB11 */
	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_6;
	GPIO_Init(GPIOB, &gpio);

}

void init_leds()
{
	configure_LEDs_GPIO();

	/*
	while(1)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_11);	//Led ON
		sleep(100);
		//CoTickDelay (100);
		GPIO_ResetBits(GPIOB, GPIO_Pin_11);	//Led OFF
		sleep(100);
		//CoTickDelay (100);
	}*/
}


