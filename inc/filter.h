#ifndef FILTER_H
#define FILTER_H
#include "ecg.h"




#define N 2
#define FILTER_ORDER_PB (int16_t)pow(2,N)
#define FILTER_ORDER 128

extern int16_t kernel[FILTER_ORDER];

void moving_average(int32_t *signal, unsigned int size, int32_t *o_signal);
void filter(int32_t *input, int32_t *buffer, int16_t *last,int32_t *output);
void next(int16_t *pos);
void init_buffers(int32_t  buffer[][FILTER_ORDER]);
void save_signal(int16_t *signal, int size, const char *name);

#endif // FILTER_H
