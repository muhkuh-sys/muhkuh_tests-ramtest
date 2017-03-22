
#ifndef __RAMTEST_H__
#define __RAMTEST_H__


typedef enum RAMTEST_RESULT_ENUM
{
	RAMTEST_RESULT_OK     = 0,
	RAMTEST_RESULT_FAILED = 1
} RAMTEST_RESULT_T;


typedef enum
{
	RAMTESTCASE_DATABUS            = 0x00000001,
	RAMTESTCASE_08BIT              = 0x00000002,
	RAMTESTCASE_16BIT              = 0x00000004,
	RAMTESTCASE_32BIT              = 0x00000008,
	RAMTESTCASE_MARCHC             = 0x00000010,
	RAMTESTCASE_CHECKERBOARD       = 0x00000020,
	RAMTESTCASE_BURST              = 0x00000040,
	RAMTESTCASE_SEQUENCE           = 0x00000080,
	RAMTESTCASE_MEMCPY             = 0x00000100,
	RAMTESTCASE_CA9SMP_ALTERNATE   = 0x00010000,
	RAMTESTCASE_CA9SMP_BLOCK       = 0x00020000,
} RAMTESTCASE_T;

#define RAMTESTCASE_PSEUDORANDOM_MASK  (RAMTESTCASE_08BIT | RAMTESTCASE_16BIT | RAMTESTCASE_32BIT | RAMTESTCASE_BURST)
#define RAMTESTCASE_DETERMINISTIC_MASK (RAMTESTCASE_DATABUS | RAMTESTCASE_MARCHC | RAMTESTCASE_CHECKERBOARD | RAMTESTCASE_SEQUENCE | RAMTESTCASE_MEMCPY)
#define RAMTESTCASE_SMP_MASK           (RAMTESTCASE_CA9SMP_ALTERNATE | RAMTESTCASE_CA9SMP_BLOCK)


typedef enum
{
	RAMPERFTESTCASE_SEQ_R8       = 0x00000001,
	RAMPERFTESTCASE_SEQ_R16      = 0x00000002,
	RAMPERFTESTCASE_SEQ_R32      = 0x00000004,
	RAMPERFTESTCASE_SEQ_R256     = 0x00000008,
	RAMPERFTESTCASE_SEQ_W8       = 0x00000010,
	RAMPERFTESTCASE_SEQ_W16      = 0x00000020,
	RAMPERFTESTCASE_SEQ_W32      = 0x00000040,
	RAMPERFTESTCASE_SEQ_W256     = 0x00000080,
	RAMPERFTESTCASE_SEQ_RW8      = 0x00000100,
	RAMPERFTESTCASE_SEQ_RW16     = 0x00000200,
	RAMPERFTESTCASE_SEQ_RW32     = 0x00000400,
	RAMPERFTESTCASE_SEQ_RW256    = 0x00000800,
	RAMPERFTESTCASE_SEQ_NOP      = 0x00001000,
	RAMPERFTESTCASE_ROW_R8       = 0x00010000,
	RAMPERFTESTCASE_ROW_R16      = 0x00020000,
	RAMPERFTESTCASE_ROW_R32      = 0x00040000,
	RAMPERFTESTCASE_ROW_R256     = 0x00080000,
	RAMPERFTESTCASE_ROW_W8       = 0x00100000,
	RAMPERFTESTCASE_ROW_W16      = 0x00200000,
	RAMPERFTESTCASE_ROW_W32      = 0x00400000,
	RAMPERFTESTCASE_ROW_W256     = 0x00800000,
	RAMPERFTESTCASE_ROW_RW8      = 0x01000000,
	RAMPERFTESTCASE_ROW_RW16     = 0x02000000,
	RAMPERFTESTCASE_ROW_RW32     = 0x04000000,
	RAMPERFTESTCASE_ROW_RW256    = 0x08000000,
	RAMPERFTESTCASE_ROW_JUMP     = 0x10000000
} RAMPERFTESTCASE_T;


struct RAMTEST_PARAMETER_STRUCT;
typedef void (*PFN_RAMTEST_PROGRESS_T) (struct RAMTEST_PARAMETER_STRUCT *ptRamTestParameter, RAMTEST_RESULT_T tResult);

typedef void (*PFN_RAMTEST_LOOP_FINISHED_T) (struct RAMTEST_PARAMETER_STRUCT *ptRamTestParameter);

typedef struct RAMTEST_PARAMETER_STRUCT
{
	unsigned long ulStart;
	unsigned long ulSize;
	unsigned long ulCases;
	unsigned long ulLoops;
	
	unsigned long ulPerfTestCases;
	unsigned long ulRowSize;
	unsigned long ulRefreshTime_clk;
	
	PFN_RAMTEST_PROGRESS_T pfnProgress;
	unsigned long ulProgress;
	unsigned long ulTimes[32];
	PFN_RAMTEST_LOOP_FINISHED_T pfnLoopFinished; /* called when a loop is complete if non-NULL */
	
	unsigned long ulTagMask;
	unsigned long ulTagValue;
	unsigned long ulLoopCnt;
} RAMTEST_PARAMETER_T;



RAMTEST_RESULT_T ramtest_run(RAMTEST_PARAMETER_T *ptParameter);
RAMTEST_RESULT_T ramtest_deterministic(RAMTEST_PARAMETER_T *ptParameter);
RAMTEST_RESULT_T ramtest_run_performance_tests(RAMTEST_PARAMETER_T *ptParameter);
void ramtest_print_performance_tests(RAMTEST_PARAMETER_T *ptParameter);
void ram_perftest_init_netx(void);

unsigned long pseudo_generator(unsigned long number);


#endif  /* __RAMTEST_H__ */
