#ifndef TIMER_H
#define TIMER_H
#include "ecg.h"

//
// System timer
// Provee funciones de sleep, timing, etc.
// Tambien maneja los LEDs de la placa.
//

// Busywait de una condicion, con timeout en milisegundos
#define waitForCondTimout(cond, timeout) { \
    uint32_t mt=getMsecs()+(timeout); \
    while(!(cond) && getMsecs()<mt); }

void init_timer_system();

// Cantidad de milisegundos desde que inicio el programa
uint32_t getMsecs();

// Sleep con busy-wait
void sleep(uint32_t msecs);

// Interupcion del systimer
void SysTick_Handler(void);



#endif
