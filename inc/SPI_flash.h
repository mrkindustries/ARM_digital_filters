#ifndef BATTERY_H
#define BATTERY_H

#include "ecg.h"
//#include "stm32f10x_conf.h"


void sFLASH_LowLevel_Init(void);
void sFLASH_LowLevel_DeInit(void);
void sFLASH_DisableWriteProtection(void);

// borrar si no sirve http://mbed.org/users/emmibed/programs/SST25VF/m3u08c/docs/SST25VF_8cpp_source.html
#endif
