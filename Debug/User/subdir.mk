################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/M95Task.c \
../User/MainTask.c \
../User/ModbusPacketTask.c \
../User/ModbusTask.c \
../User/ReadRegistersTask.c \
../User/fm25v02.c \
../User/gpio.c \
../User/m95.c \
../User/modbus.c 

OBJS += \
./User/M95Task.o \
./User/MainTask.o \
./User/ModbusPacketTask.o \
./User/ModbusTask.o \
./User/ReadRegistersTask.o \
./User/fm25v02.o \
./User/gpio.o \
./User/m95.o \
./User/modbus.o 

C_DEPS += \
./User/M95Task.d \
./User/MainTask.d \
./User/ModbusPacketTask.d \
./User/ModbusTask.d \
./User/ReadRegistersTask.d \
./User/fm25v02.d \
./User/gpio.d \
./User/m95.d \
./User/modbus.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/Gorseti_work/Projects/Bootloader/Bootloader/User" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User

clean-User:
	-$(RM) ./User/M95Task.d ./User/M95Task.o ./User/MainTask.d ./User/MainTask.o ./User/ModbusPacketTask.d ./User/ModbusPacketTask.o ./User/ModbusTask.d ./User/ModbusTask.o ./User/ReadRegistersTask.d ./User/ReadRegistersTask.o ./User/fm25v02.d ./User/fm25v02.o ./User/gpio.d ./User/gpio.o ./User/m95.d ./User/m95.o ./User/modbus.d ./User/modbus.o

.PHONY: clean-User

