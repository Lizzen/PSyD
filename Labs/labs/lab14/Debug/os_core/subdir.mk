################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../os_core/os_core.c \
../os_core/os_dbg_r.c \
../os_core/os_flag.c \
../os_core/os_mbox.c \
../os_core/os_mem.c \
../os_core/os_mutex.c \
../os_core/os_q.c \
../os_core/os_sem.c \
../os_core/os_task.c \
../os_core/os_time.c \
../os_core/ucos_ii.c 

OBJS += \
./os_core/os_core.o \
./os_core/os_dbg_r.o \
./os_core/os_flag.o \
./os_core/os_mbox.o \
./os_core/os_mem.o \
./os_core/os_mutex.o \
./os_core/os_q.o \
./os_core/os_sem.o \
./os_core/os_task.o \
./os_core/os_time.o \
./os_core/ucos_ii.o 

C_DEPS += \
./os_core/os_core.d \
./os_core/os_dbg_r.d \
./os_core/os_flag.d \
./os_core/os_mbox.d \
./os_core/os_mem.d \
./os_core/os_mutex.d \
./os_core/os_q.d \
./os_core/os_sem.d \
./os_core/os_task.d \
./os_core/os_time.d \
./os_core/ucos_ii.d 


# Each subdirectory must supply rules for building sources it contributes
os_core/%.o: ../os_core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -I.. -I../../BSP/include -I../os_core -I../os_port -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


