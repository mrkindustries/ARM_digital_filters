#include "filter.h"

// http://mshook.appspot.com/z/firkernel.htm?samplerate=500&filterorder=127&filtertype=bandpass&cutoff=0.05&highcutoff=3
int16_t kernel[]={-2,-2,-2,-2,-2,-2,-3,-4,-4,-5,-6,-6,-6,-6,-5,-4,-4,-4,-4,-5,-7,-10,-13,-15,-17,-18,-17,-16,-13,
		-10,-7,-5,-5,-8,-12,-19,-26,-32,-37,-39,-37,-32,-23,-13,-4,3,6,2,-7,-23,-42,-61,-77,-86,-84,-70,-42,-2,
		48,102,155,201,236,254,254,236,201,155,102,48,-2,-42,-70,-84,-86,-77,-61,-42,-23,-7,2,6,3,-4,-13,-23,-32,
		-37,-39,-37,-32,-26,-19,-12,-8,-5,-5,-7,-10,-13,-16,-17,-18,-17,-15,-13,-10,-7,-5,-4,-4,-4,-4,-5,-6,-6,-6,
		-6,-5,-4,-4,-3,-2,-2,-2,-2,-2,-2};
/*
int16_t kernel[]={0,0,0,0,1,0,0,0,0,0,1,2,3,3,3,3,3,3,3,4,4,3,2,1,1,2,4,6,7,6,2,-2,-8,-12,-12,-7,0,6,10,6,-3,-17,
		-30,-38,-35,-24,-8,5,11,5,-11,-34,-54,-64,-58,-40,-15,4,12,4,-17,-44,-66,922,-66,-44,-17,4,12,4,
		-15,-40,-58,-64,-54,-34,-11,5,11,5,-8,-24,-35,-38,-30,-17,-3,6,10,6,0,-7,-12,-12,-8,-2,2,6,7,6,
		4,2,1,1,2,3,4,4,3,3,3,3,3,3,3,2,1,0,0,0,0,0,1,0,0,0,0,0};
*/

void moving_average(int32_t *signal, unsigned int size, int32_t *o_signal){
    unsigned int i = 0;
    int32_t sum = 0; // suma de los valores de la señal de entrada pertenecientes al rango que abarca la ventana.
    int32_t old = 0; // el valor a descartar de la ventana a medida que avanzo.
    //Dos FOR para evitar preguntar cada vez si i<FILTER_ORDER. Ver despues que conviene.
    for(i = 0; i< FILTER_ORDER_PB; i++){
        sum = sum + signal[i];
        o_signal[i]=(sum >> N); // = sum * (1/FILTER_ORDER), con FILTER_ORDER potencia de 2.
    }
    for(i = FILTER_ORDER_PB; i < size; i++){
        old= signal[i - FILTER_ORDER_PB];
        sum = sum - old + signal[i]; //descarto el valor viejo e ingreso el valor de la posicion actual.
        o_signal[i]= (sum >> N); // = sum * (1/FILTER_ORDER), con FILTER_ORDER potencia de 2.
    }
}

void next(int16_t *pos){
	*pos = (*pos + 1) % FILTER_ORDER;
}

void filter(int32_t *input, int32_t *buffer, int16_t *last, int32_t *output){
    int32_t average = 0;
    int16_t i = 0;

    //*last = (*last +1) % FILTER_ORDER; //busco la posicion a descartar, donde pueda ingresar el valor nuevo.
    buffer[*last] = *input; 
    int first = (*last + 1) % 128; // a first le asigno la siguiente posicion a *last.
    for(i = 0; i<FILTER_ORDER; i++){
        average += kernel[i]* buffer[first];
        first = (first +1) % FILTER_ORDER;
    }
    average /= 1000;//1000;//1000000;
    output[0] = average;// Valor de aplicar el filtro a la entrada n.
   
    return;
}

void init_buffers(int32_t buffer[][FILTER_ORDER]){
	uint16_t i,j;

	for(i = 0; i<8; i++){
		for(j = 0; j<FILTER_ORDER;j++){
			buffer[i][j]=0;
		}
	}
}


