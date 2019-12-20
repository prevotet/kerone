################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_gpio.c \
../src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_int.c \
../src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_pm.c \
../src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_tmr.c 

OBJS += \
./src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_gpio.o \
./src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_int.o \
./src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_pm.o \
./src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_tmr.o 

C_DEPS += \
./src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_gpio.d \
./src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_int.d \
./src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_pm.d \
./src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/csp_tmr.d 


# Each subdirectory must supply rules for building sources it contributes
src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/%.o: ../src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM gcc compiler'
	arm-xilinx-eabi-gcc -Wall -O2 -DNDEBUG -I../src/App -I../src/BSP -I../src/Init -I../src/uC-CPU -I../src/uC-LIB -I../src/uC-CPU/ARM-Cortex-A9/GNU -I../src/uC-CSP/MCU -I../src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore -I../src/uC-CSP/MCU/Xilinx/Cortex-A9_MPCore/Zynq-7000 -I../src/uCOS-II/Source -I../src/uCOS-II/Ports/ARM-Cortex-A9/Generic/GNU -I../src/uCOS-II/Ports/ARM-Cortex-A9/CSP -I../src/uCOS-II/Ports/ARM-Cortex-A9/CSP/Xilinx/Cortex-A9_MPCore -I../../ucos-ii_demo_0_bsp/ps7_cortexa9_0/include -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


