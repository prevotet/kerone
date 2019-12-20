/*
*********************************************************************************************************
*                                               uC/OS-II
*                                         The Real-Time Kernel
*
*                             (c) Copyright 2012, Micrium, Inc.; Weston, FL
*                                          All Rights Reserved
*
*
* File : APP.C
* By   : JPB
*
* LICENSING TERMS:
* ---------------
*           uC/OS-II is provided in source form for FREE short-term evaluation, for educational use or
*           for peaceful research.  If you plan or intend to use uC/OS-II in a commercial application/
*           product then, you need to contact Micrium to properly license uC/OS-II for its use in your
*           application/product.   We provide ALL the source code for your convenience and to help you
*           experience uC/OS-II.  The fact that the source is provided does NOT mean that you can use
*           it commercially without paying a licensing fee.
*
*           Knowledge of the source code may NOT be used to develop a similar product.
*
*           Please help us continue to provide the embedded community with the finest software available.
*           Your honesty is greatly appreciated.
*
*           You can contact us at www.micrium.com, or by phone at +1 (954) 217-2036.
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>
#include "port/guest_os_parameters.h"
#include "port/guest_os_fctns.h"
#include "port/guest_os_vpsr.h"

#if IS_TASK_RELEASE_TEST
#include "port/sd/sd.h"
#include "port/sd/ff.h"
#endif
/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK       AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE]; /* Startup Task Stack                                */
static  OS_STK       AppTask1Stk[APP_CFG_TASK_1_STK_SIZE];         /* Task #1      Stack                                */
static  OS_STK       AppTask2Stk[APP_CFG_TASK_2_STK_SIZE];         /* Task #2      Stack                                */
#if IS_TASK_RELEASE_TEST
#else
static  OS_STK       AppTask3Stk[APP_CFG_TASK_3_STK_SIZE];         /* Task #3      Stack                                */
#endif
        OS_EVENT    *AppMutexPrint;                                 /* UART mutex.                                       */


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskCreate      (void);

static  void  AppMutexCreate     (void);

static  void  AppTaskStart       (void *p_arg);
static  void  AppTask2           (void *p_arg);
static  void  AppTask1           (void *p_arg);

#if IS_TASK_RELEASE_TEST
#else
static  void  AppTask3           (void *p_arg);
#endif

#if IS_PRR_MANAGER_TEST
static  void  AppTask4           (void *p_arg);
static  void  AppTask5           (void *p_arg);
#endif

static  void  AppPrint           (char *str);
static  void  AppPrintWelcomeMsg (void);
        void  print              (char *str);


#if IS_TASK_RELEASE_TEST

int int_flag = 0;
int data_counter = 0;
#define file_size   512*1024
#define data_num    file_size >> 2

static unsigned int TestResult [data_num];
unsigned int * pm_data = & TestResult;

//void GlobalTimerReset(){
//	*(unsigned int *)(0xf8f00208) = 0;
//	*(unsigned int *)(0xf8f00200) = 0;
//	*(unsigned int *)(0xf8f00208) = 1;
//}

void SaveFile(){
	char * datfile = "vmpm";
	int cnt;

#if OS_CRITICAL_METHOD == 3u                 /* Allocate storage for CPU status register               */
    OS_CPU_SR   cpu_sr = 0u;
#endif

	InitSD();
	SDOpen(datfile, &fil1);
	SDAccess_write(0, (unsigned int)pm_data, file_size , (unsigned int *)&cnt, &fil1);
	SDClose(&fil1);

	xil_printf(" %d Bytes are saved from 0x%x. \n\r", cnt, (unsigned int)pm_data);

    OS_ENTER_CRITICAL();
	while(1);
	OS_EXIT_CRITICAL();
}

void Record(unsigned int result){
	if(int_flag){
		pm_data[data_counter++] = result;
		int_flag = 0;
	}
	if(data_counter == data_num)
		SaveFile();
}


#endif

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Startup Code.
*
* Note(s)     : none.
*********************************************************************************************************
*/

VM_PSR vpsr;

int  main (void)
{
#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR  cpu_err;
#endif

    print("\r\n****** uC/OS-II start ******\r\n");

    guest_IRQ_Register(OS_CPU_ARM_ExceptIrqHndlr, (unsigned int)&vpsr);
    guest_CPU_IntDis();

    BSP_Init();

    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_NameSet((CPU_CHAR *)CSP_DEV_NAME,
                (CPU_ERR  *)&cpu_err);
#endif

    AppPrintWelcomeMsg();

    OSInit();                                                   /* Initialize uC/OS-II.                                 */

    OSTaskCreateExt(AppTaskStart,                               /* Create the start task                                */
                   (void       *)0,
                   (OS_STK     *)&AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE - 1],
                   (OS_PRIO     )APP_CFG_TASK_START_PRIO,
                   (OS_PRIO     )APP_CFG_TASK_START_PRIO,
                   (OS_STK     *)&AppTaskStartStk[0],
                   (CPU_INT32U  )APP_CFG_TASK_START_STK_SIZE,
                   (void       *)0,
                   (CPU_INT16U  )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

    guest_CPU_IntEn();  /* Enable interrupts so that clock tick is received */

    OSStart();          /* Start multitasking (i.e. give control to uC/OS-II).  */

    return 0;
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    (void)p_arg;


    print("Task Start Created\r\n");

    //OS_CSP_TickInit();                                          /* Initialize the Tick interrupt                        */
    guest_OS_CSP_TickInit();

    Mem_Init();                                                 /* Initialize memory management module                  */
    Math_Init();                                                /* Initialize mathematical module                       */

#if (OS_TASK_STAT_EN > 0u)
    OSStatInit();                                               /* Determine CPU capacity                               */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    AppMutexCreate();                                           /* Create Mutual Exclusion Semaphores                   */

    AppTaskCreate();                                            /* Create Application tasks                             */

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */

#if IS_TASK_RELEASE_TEST
    	OSTimeDlyHMSM(0, 0, 0, 50);
    	Record(*(unsigned int*)(0xf8f00200));

#elif IS_TASK_GENERAL_TEST
    	sys_void();//Performance measurement
#endif

#if	!IS_TASK_RELEASE_TEST
    	OSTimeDlyHMSM(0, 0, 0, 100);                            /* Waits 100 milliseconds.                              */
#endif

    	AppPrint(".");                                          /* Prints a dot every 100 milliseconds.                 */

    }
}


/*
*********************************************************************************************************
*                                       CREATE APPLICATION TASKS
*
* Description : Creates the application tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
#if !IS_PRR_MANAGER_TEST
	OSTaskCreateExt(AppTask1,                                   /* Create the Task #1.                                  */
                   (void       *)0,
                   (OS_STK     *)&AppTask1Stk[APP_CFG_TASK_1_STK_SIZE - 1],
                   (OS_PRIO     )APP_CFG_TASK_1_PRIO,
                   (OS_PRIO     )APP_CFG_TASK_1_PRIO,
                   (OS_STK     *)&AppTask1Stk[0],
                   (CPU_STK_SIZE)APP_CFG_TASK_1_STK_SIZE,
                   (void       *)0,
                   (CPU_INT16U  )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

    OSTaskCreateExt(AppTask2,                                   /* Create the Task #2.                                  */
                   (void       *)0,
                   (OS_STK     *)&AppTask2Stk[APP_CFG_TASK_2_STK_SIZE - 1],
                   (OS_PRIO     )APP_CFG_TASK_2_PRIO,
                   (OS_PRIO     )APP_CFG_TASK_2_PRIO,
                   (OS_STK     *)&AppTask2Stk[0],
                   (CPU_STK_SIZE)APP_CFG_TASK_2_STK_SIZE,
                   (void       *)0,
                   (CPU_INT16U  )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
#endif

#if IS_TASK_RELEASE_TEST
#else
//    OSTaskCreateExt(AppTask3,                                   /* Create the Task #3.                                  */
//                       (void       *)0,
//                       (OS_STK     *)&AppTask3Stk[APP_CFG_TASK_3_STK_SIZE - 1],
//                       (OS_PRIO     )APP_CFG_TASK_3_PRIO,
//                       (OS_PRIO     )APP_CFG_TASK_3_PRIO,
//                       (OS_STK     *)&AppTask3Stk[0],
//                       (CPU_STK_SIZE)APP_CFG_TASK_3_STK_SIZE,
//                       (void       *)0,
//                       (CPU_INT16U  )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
#endif

#if IS_PRR_MANAGER_TEST
	OSTaskCreateExt(AppTask4,                                   /* Create the Task #1.                                  */
                   (void       *)0,
                   (OS_STK     *)&AppTask1Stk[APP_CFG_TASK_1_STK_SIZE - 1],
                   (OS_PRIO     )APP_CFG_TASK_1_PRIO,
                   (OS_PRIO     )APP_CFG_TASK_1_PRIO,
                   (OS_STK     *)&AppTask1Stk[0],
                   (CPU_STK_SIZE)APP_CFG_TASK_1_STK_SIZE,
                   (void       *)0,
                   (CPU_INT16U  )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

    OSTaskCreateExt(AppTask5,                                   /* Create the Task #2.                                  */
                   (void       *)0,
                   (OS_STK     *)&AppTask2Stk[APP_CFG_TASK_2_STK_SIZE - 1],
                   (OS_PRIO     )APP_CFG_TASK_2_PRIO,
                   (OS_PRIO     )APP_CFG_TASK_2_PRIO,
                   (OS_STK     *)&AppTask2Stk[0],
                   (CPU_STK_SIZE)APP_CFG_TASK_2_STK_SIZE,
                   (void       *)0,
                   (CPU_INT16U  )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
#endif

}


/*
*********************************************************************************************************
*                                       CREATE APPLICATION MUTEXES
*
* Description : Creates the application mutexes.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppMutexCreate (void)
{
	CPU_INT08U  err;


    AppMutexPrint = OSMutexCreate(20, &err);                    /* Creates the UART mutex.                              */
}


/*
*********************************************************************************************************
*                                              TASK #1
*
* Description : This is an example of an application task that prints "1" every second to the UART.
*
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTask1 (void *p_arg)
{
    (void)p_arg;

    AppPrint("Task #1 Started\r\n");

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */

    	OSTimeDlyHMSM(0, 0, 1, 000);                              /* Waits for 1-second.                                  */

#if IS_TASK_RELEASE_TEST
    	Record(*(unsigned int*)(0xf8f00200));

#elif IS_TASK_GENERAL_TEST
    	sys_void(); //Performance measurement
#else
#endif

    	AppPrint("1");                                          /* Prints 1 to the UART.                                */

    }
}


/*
*********************************************************************************************************
*                                               TASK #2
*
* Description : This is an example of an application task that prints "2" every 2 seconds to the UART.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTask2 (void *p_arg)
{
    (void)p_arg;

    AppPrint("Task #2 Started\r\n");

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */

    	OSTimeDlyHMSM(0, 0, 2, 000);                              /* Waits for 2-seconds.                                 */

#if IS_TASK_RELEASE_TEST
    	Record(*(unsigned int*)(0xf8f00200));
#endif

    	AppPrint("2");                                     /* Prints 2 to the UART.                                */

    }
}

#if IS_TASK_RELEASE_TEST
#else
static  void  AppTask3 (void *p_arg)
{
    (void)p_arg;

    float x = 2.5;
    float y = 4.5;

    AppPrint("Task #3 Started\r\n");

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */

    	OSTimeDlyHMSM(0, 0, 2, 500);                              /* Waits for 2-seconds.                                 */

    	AppPrint("C");                                     /* Prints 2 to the UART.                                */

    	if(x<y)
    		x = x * y;
    	else
    		x = x / y;

    	xil_printf("[%d]", (unsigned long int)x);
    }
}
#endif

/* Tasks to test PRR managements */
#if IS_PRR_MANAGER_ASSIGN_TEST
#define PRCTRL			0x10000000
#define HW_DEV0 		0x10001000
#define HW_DEV1 		0x10002000

static  void  AppTask4 (void *p_arg)
{
    (void)p_arg;

    AppPrint("Task #4 Started\r\n");

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
    	OSTimeDlyHMSM(0, 0, 0, 50);                              /* Waits for 2-seconds.                                 */
    	*(unsigned long int*)(HW_DEV0 + 4) = 0x1;
    }
}

static  void  AppTask5 (void *p_arg)
{
    (void)p_arg;

    AppPrint("Task #5 Started\r\n");

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
    	OSTimeDlyHMSM(0, 0, 0, 60);                              /* Waits for 2-seconds.                                 */
    	*(unsigned long int*)(HW_DEV1 + 4) = 0x1;
    }
}

#endif

#if IS_PRR_MANAGER_PREEMT_TEST
#define PRCTRL			0x10000000
#define HW_DEV0 		0x10001000
#define HW_DEV1 		0x10002000

//void IVC_handler(){
//	*(unsigned long int*)(PRCTRL) = 0x1;
//}

static  void  AppTask4 (void *p_arg)
{
    (void)p_arg;

    AppPrint("Task #4 Started\r\n");

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
    	OSTimeDlyHMSM(0, 0, 0, 50);                              /* Waits for 2-seconds.                                 */
    	*(unsigned long int*)(HW_DEV0 + 4) = 0x1;
    }
}

static  void  AppTask5 (void *p_arg)
{
    (void)p_arg;

    AppPrint("Task #5 Started\r\n");

//    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
//    	OSTimeDlyHMSM(0, 0, 0, 60);                              /* Waits for 2-seconds.                                 */
//    	//*(unsigned long int*)(HW_DEV1 + 4) = 0x1;
//    }
}

#endif
/*
*********************************************************************************************************
*                                            PRINT THROUGH UART
*
* Description : Prints a string through the UART. It makes use of a mutex to
*               access this shared resource.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : application functions.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppPrint (char *str)
{
	CPU_INT08U  err;


	OSMutexPend(AppMutexPrint, 0, &err);                        /* Wait for the shared resource to be released.         */

	print(str);                                                 /* Access the shared resource.                          */

    OSMutexPost(AppMutexPrint);                                 /* Releases the shared resource.                        */
}


/*
*********************************************************************************************************
*                                        PRINT WELCOME THROUGH UART
*
* Description : Prints a welcome message through the UART.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : application functions.
*
* Note(s)     : Because the welcome message gets displayed before
*               the multi-tasking has started, it is safe to access
*               the shared resource directly without any mutexes.
*********************************************************************************************************
*/




static  void  AppPrintWelcomeMsg (void)
{
	print("\r\n");
    print("Micrium\r\n");
    print("Guest uCOS-II\r\n\r\n");
    print("This application runs three different tasks:\r\n\r\n");
    print("1. Task Start: Initializes the OS and creates tasks and\r\n");
    print("               other kernel objects such as semaphores.\r\n");
    print("               This task remains running and printing a\r\n");
    print("               dot '.' every 100 milliseconds.\r\n");
    print("2. Task #1   : Prints '1' every 1-second.\r\n");
    print("3. Task #2   : Prints '2' every 2-seconds.\r\n\r\n");
}
