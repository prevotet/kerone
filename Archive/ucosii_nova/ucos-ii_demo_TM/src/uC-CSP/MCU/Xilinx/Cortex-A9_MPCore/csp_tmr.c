/*
*********************************************************************************************************
*                                              uC/CSP
*                                        Chip Support Package
*
*                            (c) Copyright 2012; Micrium, Inc.; Weston, FL
*
*               All rights reserved. Protected by international copyright laws.
*
*               uC/CSP is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                          TIMER MANAGEMENT
*
*                                          Xilinx Zynq-7000
* 
* Filename      : csp_tmr.h
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define  CSP_TMR_MODULE
#include <csp.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

                                                                /* Private Timer Base.                                  */     
#define  CSP_INT_ADDR_PVT_TMR                                 (CPU_ADDR   )0xF8F00600

#define  CSP_INT_REG_PVT_TMR_LOAD                           (*(CPU_REG32 *)(CSP_INT_ADDR_PVT_TMR + 0x00))
#define  CSP_INT_REG_PVT_TMR_COUNTER                        (*(CPU_REG32 *)(CSP_INT_ADDR_PVT_TMR + 0x04))
#define  CSP_INT_REG_PVT_TMR_CONTROL                        (*(CPU_REG32 *)(CSP_INT_ADDR_PVT_TMR + 0x08))
#define  CSP_INT_REG_PVT_TMR_INT_STATUS                     (*(CPU_REG32 *)(CSP_INT_ADDR_PVT_TMR + 0x0C))


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL MACRO's
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            CSP_TmrCfg()
*
* Description : Configure periodic or free running timer. 
*                   
* Argument(s) : tmr_nbr   Timer number identifier (see note #1).
*
*               freq      Periodic timer frequency. (see note #2)
*                          
* Return(s)   : DEF_OK,   If the timer was configured correctly,
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Timer number identifier are defined in 'csp.h'
*
*                       CSP_TMR_NBR_00     
*                       CSP_TMR_NBR_01
*                              .
*                              .
*                              .
*                       CSP_TMR_NBR_xx
*
*                        (a) 'CSP_TMR_NBR_SYS_TICK' is used for microcontrollers with system tick timers.
*               
*               (2) If 'freq' == 0 Timer is configured as free-running timer.
*                   If 'freq' > 0  Timer will be configured to generate a interrupt event every
*                   (1/freq) seconds.
*
*               (3) If the timer is configured in periodic mode, the interrupt handler needs
*                   to be installed first by the application in the interrupt controller.
*
*               (4) TMR 0 corresponds to the private systick timer. TMR 1 corresponsd to the shared timer. 
*
*               (5) (((PRESCALER_value + 1) * (Load_value + 1)) / PERIPHCLK) = (1 / freq)
*********************************************************************************************************
*/

CPU_BOOLEAN  CSP_TmrCfg (CSP_DEV_NBR  tmr_nbr,
                         CPU_INT32U   freq)
{
    CPU_INT32U  per_clk;  
	CPU_INT32U  load_val;
    CPU_SR_ALLOC();
  
#if (CSP_CFG_ARG_CHK_EN == DEF_ENABLED)
    if (tmr_nbr > CSP_TMR_NBR_01) {
	    return (DEF_FAIL);  
	}
	
	if ((tmr_nbr == CSP_TMR_NBR_00) && (freq == 0u)) {          /* See Note #4.                                         */
	    return (DEF_FAIL);
	}
#endif
	
	per_clk  = CSP_PM_SysClkFreqGet(CSP_PM_SYS_CLK_NBR_PERIPH_PLL);
    load_val = (per_clk / (freq)) - 1u;                           /* See Note #5.                                         */
	//load_val = 333332;
	
	switch (tmr_nbr) {
	    case CSP_TMR_NBR_00:
		    CPU_CRITICAL_ENTER();		/* IRQ is already disabled during init stage  --Tian  */
			
		    CSP_INT_REG_PVT_TMR_LOAD       = 0;                 /* Disable the timer.                                   */
		    CSP_INT_REG_PVT_TMR_INT_STATUS = DEF_BIT_00;        /* Clear any pending interrupts.                        */

			CSP_INT_REG_PVT_TMR_LOAD    = load_val;             /* Set the value to reload the counter register with.   */
                                                                /* Enable Timer IRQ, and Auto Reload.                   */
			//DEF_BIT_SET(CSP_INT_REG_PVT_TMR_CONTROL, (DEF_BIT_01 | DEF_BIT_02));
			CSP_INT_REG_PVT_TMR_CONTROL = 6;
			
			CPU_CRITICAL_EXIT();		/* IRQ is already disabled during init stage  --Tian  */
            break;
			
	    case CSP_TMR_NBR_01:
	    default:
		    return (DEF_FAIL);  
	}
	
    return (DEF_OK);
}



/*
*********************************************************************************************************
*                                            CSP_TmrOutCmpCfg()
*
* Description : Configure a timer for compare mode. 
*                   
* Argument(s) : tmr_nbr    Timer number identifier (see 'CSP_TmrCfg' note #1).
*
*               pin_nbr    Pin number.
*
*               pin_action  Output compare pin action
*
*                              CSP_TMR_OPT_PIN_OUT_NONE      Do nothing.
*                              CSP_TMR_OPT_PIN_OUT_CLR       Clear  the correspoding  external pin for output compare.
*                              CSP_TMR_OPT_PIN_OUT_SET       Set    the correspondin  external pin for output compare.                                                       
*                              CSP_TMR_OPT_PIN_OUT_TOGGLE    Toggle the corresponding external pin for output compare.
*
* Return(s)   : DEF_OK,   If the timer was configured correctly in compare mode,
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : None.
*********************************************************************************************************
*/

CPU_BOOLEAN  CSP_TmrOutCmpCfg (CSP_DEV_NBR  tmr_nbr,
                               CSP_DEV_NBR  pin,
                               CSP_OPT      pin_action,
                               CPU_INT32U   freq)
{
    (void)tmr_nbr;
    (void)pin;
    (void)pin_action;
    (void)freq;
        
    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                           CSP_TmrIntClr()
*
* Description : Clear a periodic timer interrupt.
*                   
* Argument(s) : tmr_nbr    Timer number identifier (see 'CSP_TmrCfg' note #1).
*
* Return(s)   : None.
*
* Caller(s)   : Application.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  CSP_TmrIntClr (CSP_DEV_NBR tmr_nbr)
{
#if (CSP_CFG_ARG_CHK_EN == DEF_ENABLED)
    if (tmr_nbr > CSP_TMR_NBR_01) {
	    return;  
	}
#endif
	
	switch (tmr_nbr) {
	    case CSP_TMR_NBR_00:
 	        CSP_INT_REG_PVT_TMR_INT_STATUS = DEF_BIT_00;        /* Clear the event flag by setting it to 1.             */
			break;
			
	    case CSP_TMR_NBR_01:
	    default:    
		    return;  
	}
}


/*
*********************************************************************************************************
*                                            CSP_TmrRst()
*
* Description : Resets a timer.
*                   
* Argument(s) : tmr_nbr    tmr_nbr    Timer number identifier (see 'CSP_TmrCfg' note #1).
*
* Return(s)   : None.
*
* Caller(s)   : Application.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  CSP_TmrRst (CSP_DEV_NBR  tmr_nbr)
{
#if (CSP_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* Do not modify the systick timer once initialized.    */
    if (tmr_nbr == CSP_TMR_NBR_00) {
	    return;  
	}
#endif
}


/*
*********************************************************************************************************
*                                             CSP_TmrRd()
*
* Description : Read the current value of a timer.
*                   
* Argument(s) : tmr_nbr    tmr_nbr    Timer number identifier (see 'CSP_TmrCfg' note #1).
*
* Return(s)   : The current value of the timer.
*
* Caller(s)   : Application.
*
* Note(s)     : None.
*********************************************************************************************************
*/

CSP_TMR_VAL  CSP_TmrRd  (CSP_DEV_NBR  tmr_nbr)
{
#if (CSP_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* Do not modify the systick timer once started.        */
    if (tmr_nbr == CSP_TMR_NBR_00) {
	    return (DEF_FAIL);  
	}
#endif

    return (CSP_TMR_VAL)0;
}


/*
*********************************************************************************************************
*                                     CSP_TmrStart()
*
* Description : Start a timer.
*                   
* Argument(s) : tmr_nbr    tmr_nbr    Timer number identifier (see 'CSP_TmrCfg' note #1).
*
* Return(s)   : None.
*
* Caller(s)   : Application.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  CSP_TmrStart (CSP_DEV_NBR  tmr_nbr)
{
#if (CSP_CFG_ARG_CHK_EN == DEF_ENABLED)
    if (tmr_nbr > CSP_TMR_NBR_01) {
	    return;  
	}
#endif

	switch (tmr_nbr) {
	    case CSP_TMR_NBR_00:
	        DEF_BIT_SET(CSP_INT_REG_PVT_TMR_CONTROL, DEF_BIT_00);
			break;
			
	    case CSP_TMR_NBR_01:
	    default:
		    return;
	}
}


/*
*********************************************************************************************************
*                                            CSP_TmrStop()
*
* Description : Stop a timer.
*                   
* Argument(s) : tmr_nbr    tmr_nbr    Timer number identifier (see 'CSP_TmrCfg' note #1).
*
* Return(s)   : None.
*
* Caller(s)   : Application.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  CSP_TmrStop  (CSP_DEV_NBR  tmr_nbr)
{
#if (CSP_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* Do not modify the systick timer once started.        */
    if (tmr_nbr == CSP_TMR_NBR_00) {
	    return;  
	}
#endif
}


/*
*********************************************************************************************************
*                                             CSP_TmrWr()
*
* Description : Write a value to a timer.
*                   
* Argument(s) : tmr_nbr    tmr_nbr    Timer number identifier (see 'CSP_TmrCfg' note #1).
*
*               tmr_val    value to write.
*
* Return(s)   : None.
*
* Caller(s)   : Application.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  CSP_TmrWr  (CSP_DEV_NBR  tmr_nbr,
                  CSP_TMR_VAL  tmr_val)
{
#if (CSP_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* Do not modify the systick timer once started.        */
    if (tmr_nbr == CSP_TMR_NBR_00) {
	    return;  
	}
#endif
}
