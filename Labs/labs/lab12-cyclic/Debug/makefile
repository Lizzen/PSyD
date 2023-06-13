################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := cs-rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
lab12-cyclic.hex \

SECONDARY_LIST += \
lab12-cyclic.lst \

SECONDARY_SIZE += \
lab12-cyclic.siz \


# All Target
all: lab12-cyclic.elf secondary-outputs

# Tool invocations
lab12-cyclic.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Sourcery Windows GCC C Linker'
	arm-none-eabi-gcc -T"../lab12-cyclic.ld" -nostartfiles -L../../BSP/Debug -Wl,-Map,lab12-cyclic.map -mcpu=arm7tdmi -g3 -gdwarf-2 -o "lab12-cyclic.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

lab12-cyclic.hex: lab12-cyclic.elf
	@echo 'Invoking: ARM Sourcery Windows GNU Create Flash Image'
	arm-none-eabi-objcopy -O binary lab12-cyclic.elf  "lab12-cyclic.hex"
	@echo 'Finished building: $@'
	@echo ' '

lab12-cyclic.lst: lab12-cyclic.elf
	@echo 'Invoking: ARM Sourcery Windows GNU Create Listing'
	arm-none-eabi-objdump -h -S lab12-cyclic.elf > "lab12-cyclic.lst"
	@echo 'Finished building: $@'
	@echo ' '

lab12-cyclic.siz: lab12-cyclic.elf
	@echo 'Invoking: ARM Sourcery Windows GNU Print Size'
	arm-none-eabi-size  --format=berkeley lab12-cyclic.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(SECONDARY_SIZE)$(OBJS)$(C_DEPS)$(ASM_DEPS)$(SECONDARY_FLASH)$(EXECUTABLES)$(SECONDARY_LIST)$(S_UPPER_DEPS) lab12-cyclic.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
