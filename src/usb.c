#include "usb.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "ff.h"
#include "diskio.h"
#include "timersystem.h"
#include "ECG_board.h"
#include "usb_pwr.h"

extern uint16_t MAL_Init (uint8_t lun);
double signal1[1024];

extern struct
{
  __IO RESUME_STATE eState;
  __IO uint8_t bESOFcnt;
}
ResumeS;


void init_usb()
{
	sleep(400);
	//CoTickDelay(400);
	uint8_t a=1, i;
	char s[12];
	FATFS fs;
//	uint8_t Buffer[50];
	FIL file;
	UINT bytes_written = 0;
	UINT bytes_readen = 0;
/*
	f_mount(0, &fs);
	disk_initialize(0);

	disk_read (0, (BYTE*)&fs.win, 0, 1);	//leo el MBR de la memoria para saber si hay sistema de archivos

	//esto va, probando usb...
	if(fs.win[510]!=0x55 || fs.win[511]!=0xAA)
	{
		sFLASH_EraseBulk();
		f_mkfs(0, 1, 512);
	}

*/

/*
	strcpy((char*)&s,"test_XX.txt" );
	f_open(&file, (const char*)&s, FA_OPEN_ALWAYS | FA_WRITE);
	f_write(&file, "mira como me autoescribo\n", 25, &bytes_written);
	f_write(&file, "http://www.youtube.com/watch?v=LCHy0NuPugY", 35, &bytes_written);
	f_close(&file);*/

//	Set_PullUp();
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();
////	CoTickDelay(10);
	while (bDeviceState != CONFIGURED)
		//CoTickDelay(100);
		sleep(100);




    while(1)
	{
	//	bDeviceState;	// == CONFIGURED
	//	ResumeS.eState;
/*		if (i < 100)
		{
			sprintf((char *)&Buffer, "bDeviceState = %d, ResumeS.eState = %d \n",bDeviceState,ResumeS.eState );
			a = strlen((char *)&Buffer);
			if(f_write(&file, (unsigned char*)&Buffer, a, &bytes_written) == FR_OK)
				GPIO_SetBits(GPIOB, GPIO_Pin_8);
			++i;
		}
		if(i == 100)
		{
			i = 200;
			f_close(&file);
		}
		CoTickDelay (50);
	    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	*/
    	sleep(50);
    	//CoTickDelay (50);
	}
}
