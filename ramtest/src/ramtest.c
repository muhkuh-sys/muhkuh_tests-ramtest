
#include "ramtest.h"
#include "systime.h"
#include "uprintf.h"

int random_burst(unsigned long* pulStartaddress, unsigned long *pulEndAddress, unsigned long ulSeed);


typedef struct RAMTEST_PAIR_STRUCT
{
	unsigned long uiSeed;

} RAMTEST_PAIR_T;

static const RAMTEST_PAIR_T testPairs[6] =
{
	{  9004440025      }, /* 64 bit */
	{  300454440035    }, /* 64 bit */
	{  314159265       },
	{  100000000003    }, /* 64 bit */
	{  99989           },
	{  7271477         }
};



/*-----------------------------------*/

/* DATABUS TEST */
static RAMTEST_RESULT_T ram_test_databus(RAMTEST_PARAMETER_T *ptRamTestParameter)
{
	/*
	 * Description: Test the data bus wiring in a memory region by
	 *              performing a walking 1's test at a fixed address
	 *              within that region.  The address (and hence the
	 *              memory region) can be selected by the caller.
	 *
	*/

	volatile unsigned long *pulStart;
	volatile unsigned long *pulCnt;
	unsigned long ulWalkingOnes;
	unsigned char errorCounter = 0;




	pulStart = (volatile unsigned long*)(ptRamTestParameter->ulStart);

	RAMTEST_RESULT_T tResult;

	tResult = RAMTEST_RESULT_OK;
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);

	pulCnt = pulStart;

	/*Write Walking Ones */
	for(ulWalkingOnes = 1; ulWalkingOnes!=0; ulWalkingOnes<<=1)
		{
		*pulCnt++ = ulWalkingOnes;
		}

	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);
	/* Read Back walking Ones */


	pulCnt = pulStart;
	for(ulWalkingOnes = 1; ulWalkingOnes != 0; ulWalkingOnes<<=1)
	{


		if(*pulCnt != ulWalkingOnes)
		{
			if(errorCounter <= 0) uprintf("! Databus Test failed !\n");
			uprintf("                     DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD \n");
			uprintf("                     3322222222221111111111           \n");
			uprintf("                     10987654321098765432109876543210 \n");
			uprintf("! wrote     value: 0b%032b\n", ulWalkingOnes);
			uprintf("! read back value: 0b%032b\n", *pulCnt);

			errorCounter ++;
			tResult = RAMTEST_RESULT_FAILED;
		}

		pulCnt+=1;

	}

	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);
	return tResult;

}



static RAMTEST_RESULT_T ram_test_checkerboard_1pass(RAMTEST_PARAMETER_T *ptRamTestParameter, unsigned long ulPattern, unsigned long ulAntiPattern)
{
	RAMTEST_RESULT_T tResult;
	volatile unsigned long *pulStart;
	volatile unsigned long *pulEnd;
	volatile unsigned long *pulCnt;
	unsigned long ulXor;
	unsigned long ulValue;
	unsigned long ulReadBack;

	tResult =  RAMTEST_RESULT_OK;
	pulStart = (volatile unsigned long*)ptRamTestParameter->ulStart;
	pulEnd   = (volatile unsigned long*)(ptRamTestParameter->ulStart + ptRamTestParameter->ulSize);
	ulXor = ulPattern ^ ulAntiPattern;

	/* 
	 * Fill Ram with checkerboard pattern, e.g.
	 * 10101010101010101010101010101010
	 * 01010101010101010101010101010101
	 */
	pulCnt = pulStart;
	ulValue = ulPattern;
	
	while(pulCnt<pulEnd)
	{
		*pulCnt = ulValue;
		ulValue ^= ulXor;
		++pulCnt;
	}

	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);


	/* Read Back and compare */

	pulCnt     = pulStart;
	ulValue = ulPattern;

	while(pulCnt < pulEnd)
	{
		ulReadBack = *pulCnt;
		if(ulReadBack != ulValue)
		{
			uprintf("! Checkerboard test at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pulCnt, (unsigned long)(pulCnt-pulStart));
			uprintf("! wrote value:     0x%08x\n", ulValue);
			uprintf("! read back value: 0x%08x\n", ulReadBack);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}

		ulValue ^= ulXor;
		++pulCnt;
	}
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);

	return tResult;
}



static RAMTEST_RESULT_T ram_test_checkerboard(RAMTEST_PARAMETER_T *ptRamTestParameter)
{
	RAMTEST_RESULT_T tResult;
	unsigned long pattern 	  = 0xAAAAAAAA;
	unsigned long antipattern = 0x55555555;
	
	tResult = ram_test_checkerboard_1pass(ptRamTestParameter, pattern, antipattern);
	if (tResult == RAMTEST_RESULT_OK)
	{
		tResult = ram_test_checkerboard_1pass(ptRamTestParameter, antipattern, pattern);
	}
	
	return tResult;
}



static RAMTEST_RESULT_T ram_test_marching(RAMTEST_PARAMETER_T *ptRamTestParameter)
{

	RAMTEST_RESULT_T tResult;
	volatile unsigned long *pulStart;
	volatile unsigned long *pulEnd;
	volatile unsigned long *pulCnt;


	unsigned long zero  = 0x00000000;
	unsigned long ones  = 0xFFFFFFFF;

	pulStart = (volatile unsigned long*) (ptRamTestParameter->ulStart);
	pulEnd	 = (volatile unsigned long*) (ptRamTestParameter->ulStart + ptRamTestParameter->ulSize);


    tResult = RAMTEST_RESULT_OK;

	/*
	 * This test covers all:
	 *    SAF's  (Stuck at Faults)
	 *    TF's   (Transition Faults)
	 *    AF's   (Adress decoder faults)
	 *    CFin's (Coupling faults - inversion coupling)
	 *    CFid's (Coupling faults - idempotent coupling)
	 *    CFst's (State coupling faults)
	 *    SO'F   (Stuck open faults)
	 */


	/* 1 Fill Ram with zeroe's */

	pulCnt = pulStart;

	while(pulCnt < pulEnd)
	{
		*(pulCnt++) = zero;
	}
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);


	/* 2 Read zeroes, if successful fill with ones */

	pulCnt = pulStart;

	while(pulCnt < pulEnd)
	{
		if(*pulCnt == zero) *pulCnt = ones;
		else
		{

			uprintf("! Marching test failed at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pulCnt, (unsigned long)(pulCnt-pulStart));
			//uprintf("! wrote value:     0x%08x\n", zero);
			uprintf("! Wrote 0xFFFFFFFF or 0x00000000\n");
			uprintf("! read back value: 0x%08x\n", *pulCnt);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}

		++pulCnt;

	}
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);


	if (tResult == RAMTEST_RESULT_FAILED) return tResult;

	/*
	 * next code segment is not part of the marchc- test
	 * it is implemented in order to detect SAF's due to MarchC-'s incapability of detecting these
	 * it is part of mats++
	 */

	pulCnt = pulEnd;

	while(--pulCnt >= pulStart)
	{
		if(*pulCnt == ones) *pulCnt = zero;
		else
		{
			uprintf("! Marching test failed at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pulCnt, (unsigned long)(pulCnt-pulStart));
			//uprintf("! wrote value:     0x%08x\n", ones);
			uprintf("! Wrote 0xFFFFFFFF or 0x00000000\n");
			uprintf("! read back value: 0x%08x\n", *pulCnt);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}

		if(*pulCnt != zero)
		{
			uprintf("! Marching test failed at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pulCnt, (unsigned long)(pulCnt-pulStart));
			//uprintf("! wrote value:     0x%08x\n", zero);
			uprintf("! Wrote 0xFFFFFFFF or 0x00000000\n");
			uprintf("! read back value: 0x%08x\n", *pulCnt);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}
	}

	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);

	if (tResult == RAMTEST_RESULT_FAILED) return tResult;

	/* Now fill the ram with ones again in order to proceed with the MarchC- test */

	pulCnt = pulStart;

	while(pulCnt < pulEnd)
	{
		*(pulCnt++) = ones;
	}

	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);




	/* Continue with marchc- Read ones write back zeros */

	pulCnt = pulStart;

	while(pulCnt < pulEnd)
	{
			if(*pulCnt == ones) *pulCnt = zero;
			else
			{
				uprintf("! Marching test test failed at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pulCnt, (unsigned long)(pulCnt-pulStart));
				//uprintf("! wrote value:     0x%08x\n", ones);
				uprintf("! Wrote 0xFFFFFFFF or 0x00000000\n");
				uprintf("! read back value:  0x%08x\n", *pulCnt);
				tResult = RAMTEST_RESULT_FAILED;
				break;
			}

		++pulCnt;
	}
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);

	if (tResult == RAMTEST_RESULT_FAILED) return tResult;

	/* 4 Read Zeros write Ones */

	pulCnt = pulEnd;

	while(--pulCnt >= pulStart)
	{
		if(*pulCnt == zero) *pulCnt = ones; // zero ones
		else
		{
			uprintf("! Marching test test failed at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pulCnt, (unsigned long)(pulCnt-pulStart));
			//uprintf("! wrote value:     0x%08x\n", zero);
			uprintf("! Wrote 0xFFFFFFFF or 0x00000000\n");
			uprintf("! read back value: 0x%08x\n", *pulCnt);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}
	}
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);

	if (tResult == RAMTEST_RESULT_FAILED) return tResult;

	/* 5 Read ones write zeros */

	pulCnt = pulEnd;

	while(--pulCnt >= pulStart)
	{
		if(*pulCnt == ones) *pulCnt = zero;
		else
		{
			uprintf("! Marching test test failed at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pulCnt, (unsigned long)(pulCnt-pulStart));
			//uprintf("! wrote value:     0x%08x\n", ones);
			uprintf("! Wrote 0xFFFFFFFF or 0x00000000\n");
			uprintf("! read back value: 0x%08x\n", *pulCnt);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}

	}
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);

	if (tResult == RAMTEST_RESULT_FAILED) return tResult;

	/* Last Step - Read back the zeros */

	pulCnt = pulEnd;

	while(--pulCnt >= pulStart)
	{
		if(*pulCnt != zero)
		{
			uprintf("! Marching test test failed at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pulCnt, (unsigned long)(pulCnt-pulStart));
			//uprintf("! wrote value:     0x%08x\n", zero);
			uprintf("! Wrote 0xFFFFFFFF or 0x00000000\n");
			uprintf("! read back value: 0x%08x\n", *pulCnt);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}

	}
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);


	return tResult;


}


unsigned long pseudo_generator(unsigned long number)
{
	/* Works with a LFSR (linear feedback left shift register)
	 * with 32 Bits with MLS (Max Length sequence)
	 * which reproduces only after 2^32-1 generations
	 */

	unsigned long seed = number;

	seed =     ((((seed >> 31)
	            ^ (seed >> 6)
	            ^ (seed >> 4)
	            ^ (seed >> 1)
	            ^  seed)
	            &  0x00000001)
	            << 31)
	            | (seed>>1);


	return seed;


}


/* test byte access */
static RAMTEST_RESULT_T ram_test_08bit(RAMTEST_PARAMETER_T *ptRamTestParameter, const RAMTEST_PAIR_T *ptTestPair)
{
	RAMTEST_RESULT_T tResult;
	volatile unsigned char *pucStart;
	volatile unsigned char *pucEnd;
	volatile unsigned char *pucCnt;
	unsigned long ulSeed;
	unsigned long ulRandom;
	unsigned char ucRandom;
	unsigned char ucReadBack;
	
	tResult = RAMTEST_RESULT_OK;
	pucStart = (volatile unsigned char*)(ptRamTestParameter->ulStart); //Start Value
	pucEnd   = (volatile unsigned char*)(ptRamTestParameter->ulStart + ptRamTestParameter->ulSize); // End value
	ulSeed = ptTestPair->uiSeed;
	
	
	/* fill ram */
	
	ulRandom = ulSeed;
	pucCnt = pucStart;

	while( pucCnt<pucEnd )
	{
		ulRandom = pseudo_generator(ulRandom);
		ucRandom = (unsigned char)((ulRandom)&0x000000ff);
		*pucCnt = ucRandom;
		++pucCnt;
	};
	
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);
	
	
	/* read back and compare */
	
	ulRandom = ulSeed;
	pucCnt = pucStart;

	while(pucCnt<pucEnd)
	{
		ulRandom = pseudo_generator(ulRandom);
		ucRandom = (unsigned char)((ulRandom)&0x000000ff);
		ucReadBack = *pucCnt;
		if (ucReadBack!=ucRandom)
		{
			uprintf("! 8 bit access at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pucCnt, (unsigned long)(pucCnt-pucStart));
			uprintf("! wrote value:     0x%02x\n", ulRandom);
			uprintf("! read back value: 0x%02x\n", ucReadBack);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}
		++pucCnt;
	}

	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);
	return tResult;
}



/* test word access */
static RAMTEST_RESULT_T ram_test_16bit(RAMTEST_PARAMETER_T *ptRamTestParameter, const RAMTEST_PAIR_T *ptTestPair)
{
	RAMTEST_RESULT_T tResult;
	volatile unsigned short *pusStart;
	volatile unsigned short *pusEnd;
	volatile unsigned short *pusCnt;
	unsigned long ulSeed;
	unsigned long ulRandom;
	unsigned short usRandom;
	unsigned short usReadBack;
	
	tResult = RAMTEST_RESULT_OK;
	pusStart = (volatile unsigned short*)(ptRamTestParameter->ulStart); //Start Value
	pusEnd   = (volatile unsigned short*)(ptRamTestParameter->ulStart + ptRamTestParameter->ulSize); // End value
	ulSeed = ptTestPair->uiSeed;


	/* fill ram */
	
	ulRandom = ulSeed;
	pusCnt = pusStart;

	while( pusCnt<pusEnd )
	{
		ulRandom = pseudo_generator(ulRandom);
		usRandom = (unsigned short)(ulRandom&0x0000ffff);
		*pusCnt = usRandom;
		++pusCnt;
	};
	
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);

	
	/* read back and compare */
	
	ulRandom = ulSeed;
	pusCnt = pusStart;
	
	while( pusCnt<pusEnd )
	{
		ulRandom = pseudo_generator(ulRandom);
		usRandom = (unsigned short)(ulRandom&0x0000ffff);
		usReadBack = *pusCnt;
		if( usReadBack != usRandom )
		{
			uprintf("! 16 bit access at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pusCnt, (unsigned long)(pusCnt-pusStart));
			uprintf("! wrote value:     0x%04x\n", usRandom);
			uprintf("! read back value: 0x%04x\n", usReadBack);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}
		++pusCnt;
	}

	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);
	return tResult;


}


/* Test DWORD access. */
static RAMTEST_RESULT_T ram_test_32bit(RAMTEST_PARAMETER_T *ptRamTestParameter, const RAMTEST_PAIR_T *ptTestPair)
{
	RAMTEST_RESULT_T tResult;
	volatile unsigned long *pulStart;
	volatile unsigned long *pulEnd;
	volatile unsigned long *pulCnt;
	unsigned long ulSeed;
	unsigned long ulRandom;
	unsigned long ulReadBack;
	
	tResult = RAMTEST_RESULT_OK;
	pulStart = (volatile unsigned long*)(ptRamTestParameter->ulStart);
	pulEnd   = (volatile unsigned long*)(ptRamTestParameter->ulStart + ptRamTestParameter->ulSize);
	ulSeed   = ptTestPair->uiSeed;
	
	
	/* fill ram */
	
	ulRandom = ulSeed;
	pulCnt = pulStart;
	
	while(pulCnt<pulEnd)
	{
		ulRandom = pseudo_generator(ulRandom);	//Generate a pseudorandom nr
		*(pulCnt) = ulRandom;
		++pulCnt;
	}
	
	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);
	
	
	/* Read back and compare */
	
	ulRandom = ulSeed;
	pulCnt = pulStart;
	
	while (pulCnt<pulEnd)
	{
		ulRandom = pseudo_generator(ulRandom);
		ulReadBack = *pulCnt;
	
		if (ulReadBack != ulRandom)
		{
			uprintf("! 32 bit access at address 0x%08x failed (offset 0x%08x)\n", (unsigned long)pulCnt, (unsigned long)(pulCnt-pulStart));
			uprintf("! wrote value:     0x%08x\n", ulRandom);
			uprintf("! read back value: 0x%08x\n", ulReadBack);
			tResult = RAMTEST_RESULT_FAILED;
			break;
		}
		++pulCnt;
	}

	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);

	return tResult;
}


static RAMTEST_RESULT_T ram_test_burst(RAMTEST_PARAMETER_T *ptRamTestParameter, const RAMTEST_PAIR_T *ptTestPair)
{
	unsigned long *pulCnt;
	unsigned long *pulEnd;
	int iResult;
	RAMTEST_RESULT_T tResult;


	pulCnt = (unsigned long*)(ptRamTestParameter->ulStart);
	pulEnd = (unsigned long*)(ptRamTestParameter->ulStart + ptRamTestParameter->ulSize);

	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);
	iResult = random_burst(pulCnt, pulEnd, ptTestPair->uiSeed);



	ptRamTestParameter->pfnProgress(ptRamTestParameter, RAMTEST_RESULT_OK);


	if( iResult==0 )
	{
		tResult = RAMTEST_RESULT_OK;
	}

	else
	{
		tResult = RAMTEST_RESULT_FAILED;
	}

	return tResult;
}







/*-----------------------------------*/

static RAMTEST_RESULT_T ramtest_pseudorandom(RAMTEST_PARAMETER_T *ptParameter, const RAMTEST_PAIR_T *ptTestPair)
{


	/* This test contains
	 *     8  Bit
	 *     16 Bit
	 *     32 Bit
	 *     and 32 Bit Burst access tests
	 */

	RAMTEST_RESULT_T tResult;
	unsigned long ulCases;


	ulCases = ptParameter->ulCases;
	tResult = RAMTEST_RESULT_OK;


	/* test 8 bit access */
	if( tResult==RAMTEST_RESULT_OK && (ulCases&RAMTESTCASE_08BIT)!=0 )
	{
		uprintf(". Testing 8 bit access...\n");
		tResult = ram_test_08bit(ptParameter, ptTestPair);
		if( tResult==RAMTEST_RESULT_OK )
		{
			uprintf(". 8 bit access OK.\n");
		}
		else
		{
			uprintf("! 8 bit access failed.\n");
		}
	}

	/* test 16 bit access */
	if( tResult==RAMTEST_RESULT_OK && (ulCases&RAMTESTCASE_16BIT)!=0 )
	{
		uprintf(". Testing 16 bit access...\n");
		tResult = ram_test_16bit(ptParameter, ptTestPair);
		if( tResult==RAMTEST_RESULT_OK )
		{
			uprintf(". 16 bit access OK.\n");
		}
		else
		{
			uprintf("! 16 bit access failed.\n");
		}
	}

	/* test 32 bit access */
	if( tResult==RAMTEST_RESULT_OK && (ulCases&RAMTESTCASE_32BIT)!=0 )
	{
		uprintf(". Testing 32 bit access...\n");
		tResult = ram_test_32bit(ptParameter, ptTestPair);
		if( tResult==RAMTEST_RESULT_OK )
		{
			uprintf(". 32 Bit access OK.\n");
		}
		else
		{
			uprintf("! 32 Bit access failed.\n");
		}
	}



	/* test 32 bit burst access */
	if( tResult==RAMTEST_RESULT_OK && (ulCases&RAMTESTCASE_BURST)!=0 )
	{
		uprintf(". Testing 32 bit burst access...\n");
		tResult = ram_test_burst(ptParameter, ptTestPair);
		if( tResult==RAMTEST_RESULT_OK )
		{
			uprintf(". 32 bit burst access OK.\n");
		}
		else
		{
			uprintf("! 32 bit burst access failed.\n");
		}

	}


	return tResult;
}



/*
 * This test contains
 *     databus
 *     checkerboard
 *     marching C- tests
 *
 * This test can be run only one time, because it finds all mistakes it's looking for with 100% probability
 * --> 100% probability for mistakes it's looking for, doesn't find anything it's not looking for
 *
 *     databus test:
 *         looks for faulty databus lines and returns the D0..31 number of faulty line
 *
 *     checkerboard test:
 *         looks for detention faults (leakage faults)
 *
 *     marching C- test:
 *         check description above
 */

RAMTEST_RESULT_T ramtest_deterministic(RAMTEST_PARAMETER_T *ptParameter)
{
	RAMTEST_RESULT_T tResult;
	unsigned long ulCases;
	ulCases =  ptParameter->ulCases;

	tResult = RAMTEST_RESULT_OK;


	/* Test all Datalines independently from each other */
	if( tResult==RAMTEST_RESULT_OK && (ulCases&RAMTESTCASE_DATABUS)!=0 )
	{
		uprintf(". Testing Databus...\n");
		tResult = ram_test_databus(ptParameter);
		if( tResult==RAMTEST_RESULT_OK )
		{
			uprintf(". Databus test OK.\n");
		}
		else
		{
			uprintf("! Databus test failed.\n");
		}
	}


	/* Test checkerboard pattern for retention + produce hard cluster for burst */
	if( tResult==RAMTEST_RESULT_OK && (ulCases&RAMTESTCASE_CHECKERBOARD)!=0 )
	{
		uprintf(". Testing checkerboard pattern...\n");
		tResult = ram_test_checkerboard(ptParameter);
		if( tResult==RAMTEST_RESULT_OK )
		{
			uprintf(". Checkerboard test OK.\n");
		}
		else
		{
			uprintf("! Checkerboard test OK.\n");
		}
	}

	/* test Marching pattern */
	if( tResult==RAMTEST_RESULT_OK && (ulCases&RAMTESTCASE_MARCHC)!=0 )
	{
		uprintf(". Testing Marching Pattern...\n");
		tResult = ram_test_marching(ptParameter);
		if( tResult==RAMTEST_RESULT_OK )
		{
			uprintf(". Marching test OK\n");
		}
		else
		{
			uprintf("! Marching test failed.\n");
		}
	}


	return tResult;
}




RAMTEST_RESULT_T ramtest_run(RAMTEST_PARAMETER_T *ptParameter)
{
	RAMTEST_RESULT_T tRamTestResult;
	unsigned long ulCases;
	unsigned long ulLoopCnt;
	unsigned long ulLoopMax;
	unsigned int  uiTestCnt;


	/* Show welcome message. */
	uprintf(". Ram start address:      0x%08x\n", ptParameter->ulStart);
	uprintf(". Ram size in bytes:      0x%08x\n", ptParameter->ulSize);
	uprintf(". Test cases:\n");
	ulCases = ptParameter->ulCases;

	if( (ulCases&RAMTESTCASE_DATABUS)!=0 )
	{
		uprintf("     Databus\n");
	}
	if( (ulCases&RAMTESTCASE_MARCHC)!=0 )
	{
		uprintf("     Marching C-\n");
	}
	if( (ulCases&RAMTESTCASE_CHECKERBOARD)!=0 )
	{
		uprintf("     Checkerboard\n");
	}
	if( ulCases==0 )
	{
		uprintf("    none\n");
	}
	if( (ulCases&RAMTESTCASE_08BIT)!=0 )
	{
		uprintf("     8 bit\n");
	}
	if( (ulCases&RAMTESTCASE_16BIT)!=0 )
	{
		uprintf("     16 bit\n");
	}
	if( (ulCases&RAMTESTCASE_32BIT)!=0 )
	{
		uprintf("     32 bit\n");
	}
	if( (ulCases&RAMTESTCASE_BURST)!=0 )
	{
		uprintf("     Burst\n");
	}

	ulLoopMax = ptParameter->ulLoops;
	if( ulLoopMax==0 )
	{
		uprintf(". Loops:                  endless\n");
	}
	else
	{
		uprintf(". Loops:                  0x%08x\n", ulLoopMax);
	}
	uprintf("\n\n");

	/* Run test cases. */
	ulLoopCnt = 0;





	do
	{

		++ulLoopCnt;

		uprintf("****************************************\n");
		uprintf("*                                      *\n");
		uprintf("*  Deterministic test - Loop %08d  *\n", ulLoopCnt);
		uprintf("*                                      *\n");
		uprintf("****************************************\n");
		tRamTestResult = ramtest_deterministic(ptParameter);

		if(tRamTestResult == RAMTEST_RESULT_FAILED) break;

		uprintf("****************************************\n");
		uprintf("*                                      *\n");
		uprintf("*  Random number test - Loop %08d  *\n", ulLoopCnt);
		uprintf("*                                      *\n");
		uprintf("****************************************\n");



		for(uiTestCnt=0; uiTestCnt<(sizeof(testPairs)/sizeof(RAMTEST_PAIR_T)); ++uiTestCnt )
		{
			uprintf(". Start test case %d of %d\n", uiTestCnt+1, sizeof(testPairs)/sizeof(RAMTEST_PAIR_T));
			tRamTestResult = ramtest_pseudorandom(ptParameter, testPairs+uiTestCnt);
			if( tRamTestResult!=RAMTEST_RESULT_OK )
			{
				uprintf("! Test case %d failed.\n", uiTestCnt+1);
				break;
			}
			uprintf(". Test case %d OK.\n", uiTestCnt+1);
		}

		if( tRamTestResult==RAMTEST_RESULT_OK )
		{
			/* Test loop OK. */
			uprintf("* OK *\n");
		}

		if( ulLoopMax!=0 && ulLoopCnt>=ulLoopMax )
		{
			break;
		}
	} while(tRamTestResult==RAMTEST_RESULT_OK);

	return tRamTestResult;
}
