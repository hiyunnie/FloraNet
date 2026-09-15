################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW.c \
../ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_Config_FreeRTOS.c \
../ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_FreeRTOS.c 

OBJS += \
./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW.o \
./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_Config_FreeRTOS.o \
./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_FreeRTOS.o 

C_DEPS += \
./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW.d \
./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_Config_FreeRTOS.d \
./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_FreeRTOS.d 


# Each subdirectory must supply rules for building sources it contributes
ThirdParty/SEGGER/SystemView/%.o ThirdParty/SEGGER/SystemView/%.su ThirdParty/SEGGER/SystemView/%.cyclo: ../ThirdParty/SEGGER/SystemView/%.c ThirdParty/SEGGER/SystemView/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../ThirdParty/SEGGER/Config -I../ThirdParty/SEGGER/RTT -I../ThirdParty/SEGGER/SystemView -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/STM32F4xx-Nucleo -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/FloraNet/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-ThirdParty-2f-SEGGER-2f-SystemView

clean-ThirdParty-2f-SEGGER-2f-SystemView:
	-$(RM) ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW.cyclo ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW.d ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW.o ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW.su ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_Config_FreeRTOS.cyclo ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_Config_FreeRTOS.d ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_Config_FreeRTOS.o ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_Config_FreeRTOS.su ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_FreeRTOS.cyclo ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_FreeRTOS.d ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_FreeRTOS.o ./ThirdParty/SEGGER/SystemView/SEGGER_SYSVIEW_FreeRTOS.su

.PHONY: clean-ThirdParty-2f-SEGGER-2f-SystemView

