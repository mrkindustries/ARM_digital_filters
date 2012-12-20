#include "bluetooth.h"
#include "frontend.h"


void start()
{
	//configure_LEDs_GPIO();
	init_bluetooth();


	//Inicializa el módulo SPI que se utiliza para comunicar con el ADS1298 y la memoria flash
	init_spi();
	init_frontend();

//	makeEcg();

	if( btConnect() != false)
		run();
}
