################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../os_port/os_cpu_c.c 

ASM_SRCS += \
../os_port/os_cpu_a.asm \
../os_port/os_cpu_isr_wrapper.asm 

OBJS += \
./os_port/os_cpu_a.o \
./os_port/os_cpu_c.o \
./os_port/os_cpu_isr_wrapper.o 

C_DEPS += \
./os_port/os_cpu_c.d 

ASM_DEPS += \
./os_port/os_cpu_a.d \
./os_port/os_cpu_isr_wrapper.d 


# Each subdirectory must supply rules for building sources it contributes
os_port/%.o: ../os_port/%.asm
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -I../../BSP/include -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

os_port/%.o: ../os_port/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -I.. -I../../BSP/include -I../os_core -I../os_port -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


