#include "ff.h"		/*Fat Filesystem*/
#include "diskio.h"
#include "SPI_flash.h"
//#include "frontend.h"
#include "rtc.h"

volatile RTC_t rtc;


//VER QUE UTILIDAD TENDRIA ESTE MODULOI....
void init_storage()
{
	FIL* file = NULL;
	UINT bytes_written = 0;

	f_open(file, "ECG-20-3-2012.csv", FA_OPEN_ALWAYS | FA_WRITE);
	f_write(file, "hola", 5, &bytes_written);
	f_close(file);

}
