#include "timersystem.h"

#define TIMER_FREQ  100 // 10ms
static uint32_t msecs;



void  init_timer_system()
{
    msecs= 0;

    // Obtener frecuencia
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    uint32_t sysClock= clocks.SYSCLK_Frequency;
  //  NVIC_SetPriority(SysTick_IRQn, 0);

    // Iniciar systimer
    SysTick_Config(sysClock/TIMER_FREQ);
}

// Interrupcion
void SysTick_Handler(void)
{
    // Actualizar el timer
    msecs += 1000/TIMER_FREQ;

}

uint32_t getMsecs()
{
    return msecs;
}

void sleep(uint32_t msecsTime)
{
    uint32_t stopTime= msecs + msecsTime;
    while(msecs < stopTime);
}





