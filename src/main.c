#include "ecg.h"

#define FILTER_ORDER 128

int32_t input = 0;
int32_t buffer[FILTER_ORDER] = {0};	/// Buffer entero inicializado en cero

/// http://mshook.appspot.com/z/firkernel.htm?samplerate=500&filterorder=127&filtertype=bandpass&cutoff=0.05&highcutoff=3
int16_t kernel[]={-2,-2,-2,-2,-2,-2,-3,-4,-4,-5,-6,-6,-6,-6,-5,-4,-4,-4,-4,-5,-7,-10,-13,-15,-17,-18,-17,-16,-13,
		-10,-7,-5,-5,-8,-12,-19,-26,-32,-37,-39,-37,-32,-23,-13,-4,3,6,2,-7,-23,-42,-61,-77,-86,-84,-70,-42,-2,
		48,102,155,201,236,254,254,236,201,155,102,48,-2,-42,-70,-84,-86,-77,-61,-42,-23,-7,2,6,3,-4,-13,-23,-32,
		-37,-39,-37,-32,-26,-19,-12,-8,-5,-5,-7,-10,-13,-16,-17,-18,-17,-15,-13,-10,-7,-5,-4,-4,-4,-4,-5,-6,-6,-6,
		-6,-5,-4,-4,-3,-2,-2,-2,-2,-2,-2};

uint16_t sample(void)
{
	/* return calibrated current measurement */
	uint16_t ADC_value = ADC_GetConversionValue(ADC1);
	return ADC_value;

}
uint32_t FIR_filter(int32_t input, int32_t *buffer, int16_t ppos)
{
    int32_t average = 0;

    buffer[ppos - 1] = input; //busco la posicion a descartar, donde pueda ingresar el valor nuevo.


    for(int16_t i = i; i<FILTER_ORDER; i++)
    {
    	average += kernel[i]* buffer[ppos];
    	ppos++;

    	/** es un buffer circular */
    	if(ppos > (FILTER_ORDER-1))
    		ppos = 0;
    }

    return average / 1000;
}

void drive_DAC(uint16_t DAC_value)
{
	DAC_SetChannel1Data(DAC_Align_12b_R, DAC_value);
}

/**
 * @brief Initialize T1 timer to sample at 1khz
 */
void timer_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 100 - 1; // 1 MHz down to 1 KHz (0.1 ms)
	TIM_TimeBaseStructure.TIM_Prescaler = 72; // 72MHz / 72 = 1 MHz (adjust per your clock)
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);
}

/**
 * @brief Lock the processor until is time to sample
 */
void sync(void)
{
	while (TIM_GetFlagStatus(TIM2, TIM_FLAG_Update) == RESET)
		;

  TIM_ClearFlag(TIM2, TIM_IT_Update);
	//  GPIO_ToggleBits(GPIOD, GPIO_Pin_13);

}


/**
 * @brief Initialize the Digital to Analog Converter
 */
void DAC_init(void)
{
	DAC_InitTypeDef DAC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;

	/**printf("\nInitializing current loop drivers");*/

	/* Once the DAC channel is enabled, the corresponding GPIO pin is automatically
	connected to the DAC converter. In order to avoid parasitic consumption,
	the GPIO pin should be configured in analog */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* DAC channel 1 (PA.4) Configuration */
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	/* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is
	  automatically connected to the DAC converter. */
	DAC_Cmd(DAC_Channel_1, ENABLE);
}


/**
 * @brief Initialize current receiver
 */
void ADC_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	// input of ADC (it doesn't seem to be needed, as default GPIO state is floating input)
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 ;		// that's ADC5 (PA5 on STM32)
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//clock for ADC (max 14MHz --> 72/4=4MHz)
	RCC_ADCCLKConfig (RCC_PCLK2_Div6);
	// enable ADC system clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	// define ADC config
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	// we work in continuous sampling mode
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;

	ADC_RegularChannelConfig(ADC1,ADC_Channel_5, 1,ADC_SampleTime_28Cycles5); // define regular conversion config
	ADC_Init ( ADC1, &ADC_InitStructure);	//set config of ADC1

	// enable ADC
	ADC_Cmd (ADC1,ENABLE);	//enable ADC1

	//	ADC calibration (optional, but recommended at power on)
	ADC_ResetCalibration(ADC1);	// Reset previous calibration
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);	// Start new calibration (ADC must be off at that time)
	while(ADC_GetCalibrationStatus(ADC1));

	// start conversion
	ADC_Cmd (ADC1,ENABLE);	//enable ADC1
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	// start conversion (will be endless as we are in continuous mode)
}

/**
 * @brief Main loop
 */
int main(void)
{
	int16_t filtered_value = 0;

	///Initialize the system
	SystemInit();
	ADC_init();
	DAC_init();
	timer_init();

	/** lleno el buffer asi no hay entradas en 0 (sino va a haber escalones de altísima frecuencia) */
	for(int16_t i=1; i<FILTER_ORDER; i++)
	{
		sync();
		buffer[i] = sample();
	}
    while (1)
    {
    	for(int16_t i=1; i<FILTER_ORDER;i++)
    	{
			sync();				/** Bloqueo hasta que sea el momento exacto para leer la señal */
			input = sample();	/** Leo el voltaje de PA5 */
			drive_DAC(filtered_value/4 + 2000);	/**el 0 está a mitad de escala, asi puede representar nros negativos*/

			/** Testbench para probar el filtro generando internamente una señal cuadrada */
			if (i>50)
				input = 3000;
			else
				input = 0;

			/** Filtrado */
			filtered_value = FIR_filter(input, buffer, i);
    	}
    }
}

