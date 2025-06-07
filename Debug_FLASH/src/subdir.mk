################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/FlexCAN.c \
../src/main.c \
../src/pwm.c 

OBJS += \
./src/FlexCAN.o \
./src/main.o \
./src/pwm.o 

C_DEPS += \
./src/FlexCAN.d \
./src/main.d \
./src/pwm.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/FlexCAN.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


