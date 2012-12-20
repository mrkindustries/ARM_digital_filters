//#include <CoOS.h>
//#include "stm32f10x_conf.h"
#include "battery.h"
#include "timersystem.h"
/**
	 *******************************************************************************
	 * @brief       "task5_USB" task code
	 * @param[in]   None
	 * @param[out]  None
	 * @retval      None
	 * @par Description
	 * @details    Check and log battery status
	 *
	 *******************************************************************************
	 */


void init_battery()
{
	/* ADC init */

	while(1)
	{
		/* read ADC PORT?.? to measure the battery voltage */
		/* perform moving average filtering */
		sleep(1000);
		//CoTickDelay (1000);
	}
}
