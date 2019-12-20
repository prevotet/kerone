/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2012; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               uC/OS-II
*                                          MASTER INCLUDE FILE
*
*                             (c) Copyright 2012, Micrium, Inc.; Weston, FL
*                                          All Rights Reserved
*
*
* File : INCLUDES.H
* By   : JPB
*
*********************************************************************************************************
*/

#ifndef INCLUDES_H_
#define INCLUDES_H_


/*
*********************************************************************************************************
*                                         STANDARD LIBRARIES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>


/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>


/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/

#include  <app_cfg.h>
//#include  <os_app_hooks.h>
//#include  <os_cfg_app.h>
#include  <bsp.h>
//#include  <bsp_os.h>


/*
*********************************************************************************************************
*                                                 OS
*********************************************************************************************************
*/

#include  <cpu_core.h>
#include  <ucos_ii.h>
#include  <os_csp.h>


/*
*********************************************************************************************************
*                                               XILINX
*********************************************************************************************************
*/

#include <platform.h>

#endif
