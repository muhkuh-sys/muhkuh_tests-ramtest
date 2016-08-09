#include <string.h>
#include "ramtest.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"
#include "main_standalone_common.h"

#include "netx_io_areas.h"

/*-------------------------------------------------------------------------*/

/* 
	NXHX 4000:
	MMIO 51 CH0 LNK LED (green)  -> CA9 core 0 OK
	MMIO 52 CH0 ACT LED (yellow) -> CA9 core 0 Error
	MMIO 53 CH1 LNK LED (green)  -> CA9 core 1 OK
	MMIO 54 CH1 ACT LED (yellow) -> CA9 core 1 Error
	
	Note: these MMIOs must be pre-configured for PIO function, which is the default.
*/

const unsigned long MMIO_LED_ON  = 0x000100ffUL;
const unsigned long MMIO_LED_OFF = 0x000300ffUL;

#define LED_OFF 0
#define LED_GREEN 1
#define LED_YELLOW 2

void ramtest_mmio_led_progress(struct RAMTEST_PARAMETER_STRUCT *ptRamTestParameter, RAMTEST_RESULT_T tResult);
void ramtest_mmio_led_progress(struct RAMTEST_PARAMETER_STRUCT *ptRamTestParameter, RAMTEST_RESULT_T tResult)
{

	unsigned long ulProgressRaw     =  ptRamTestParameter->ulProgress;
	unsigned long ulProgressLedMmioNr  = (ulProgressRaw >> 16) & 0xff;
	unsigned long ulErrorLedMmioNr = (ulProgressRaw >> 24) & 0xff;
	unsigned long ulProgress        =  ulProgressRaw & 1;
	
	HOSTDEF(ptMmioCtrlArea);
	int iLedStates;
	unsigned long ulProgressLedState;
	unsigned long ulErrorLedState;
	
	if( tResult==RAMTEST_RESULT_OK )
	{
		if( ulProgress==0 )
		{
			iLedStates = LED_GREEN;
		}
		else
		{
			iLedStates = LED_OFF;
		}

		ptRamTestParameter->ulProgress ^= 1;
	}
	else
	{
		iLedStates = LED_YELLOW;
	}
	
	/* set leds */
	switch(iLedStates) 
	{
		case LED_OFF:
			ulProgressLedState = MMIO_LED_OFF;
			ulErrorLedState = MMIO_LED_OFF;
			break;
		case LED_GREEN:
			ulProgressLedState = MMIO_LED_ON;
			ulErrorLedState = MMIO_LED_OFF;
			break;
		case LED_YELLOW:
			ulProgressLedState = MMIO_LED_OFF;
			ulErrorLedState = MMIO_LED_ON;
			break;
	}
	
	ptMmioCtrlArea->aulMmio_cfg[ulProgressLedMmioNr] = ulProgressLedState;
	ptMmioCtrlArea->aulMmio_cfg[ulErrorLedMmioNr] = ulErrorLedState;
	
}


typedef struct RAMTEST_STANDALONE_NETX4000_PARAMETER_STRUCT
{
	unsigned long ulStart;
	unsigned long ulSize;
	unsigned long ulCases;
	unsigned long ulLoops;
	unsigned long ulPerfTestCases;
	unsigned long ulStatusLedMmioNr;
	unsigned long ulUseUart;
} RAMTEST_STANDALONE_NETX4000_PARAMETER_T; 


void ramtest_main(const RAMTEST_STANDALONE_NETX4000_PARAMETER_T* ptParam) __attribute__ ((noreturn));
void ramtest_main(const RAMTEST_STANDALONE_NETX4000_PARAMETER_T* ptParam)
{
	RAMTEST_PARAMETER_T tTestParams;
	RAMTEST_RESULT_T tRes;

#ifdef CPU_CR7
	systime_init();
#endif

	if (ptParam->ulUseUart == 1)
	{
		ramtest_init_uart();
	}
	else
	{
		ramtest_clear_serial_vectors();
	}
	
	
	uprintf("\f. *** RAM test by cthelen@hilscher.com ***\n");
	uprintf("V" VERSION_ALL "\n\n");
	
	/*
	 * Set the RAM test configuration.
	 */
	memset(&tTestParams, 0, sizeof(tTestParams));
	tTestParams.ulStart         = ptParam->ulStart;
	tTestParams.ulSize          = ptParam->ulSize;
	tTestParams.ulCases         = ptParam->ulCases;
	tTestParams.ulLoops         = ptParam->ulLoops;
	tTestParams.ulPerfTestCases = ptParam->ulPerfTestCases;
	
		
	/* Set the progress callback. */
	if (ptParam->ulStatusLedMmioNr == 0)
	{
		tTestParams.pfnProgress = ramtest_rdyrun_progress;
		tTestParams.ulProgress = 0;
	}
	else
	{
		tTestParams.pfnProgress = ramtest_mmio_led_progress;
		tTestParams.ulProgress = ptParam->ulStatusLedMmioNr;
	}
	
	/*
	 * Run the RAM test.
	 * With tTestParams.ulLoops=0 this function will only return if an error occurs.
	 */
	tRes = ramtest_run(&tTestParams);

	tTestParams.pfnProgress(&tTestParams, tRes);
	while(1);
}
