/*
 * config.h
 *
 *  Created on: 26 mai 2016
 *      Author: XIATAN
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define DEMO_UART		0

#define IS_KERNEL_FUNCTION_MEASURE 0
#define IS_KERNEL_CRITICAL_MEASURE 0
#define IS_PRRMONITOR_MEASURE 0

#define IS_PM_MEASURE IS_KERNEL_FUNCTION_MEASURE | IS_KERNEL_CRITICAL_MEASURE

#endif /* CONFIG_H_ */
