################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ThirdParty/SEGGER/RTT/SEGGER_RTT.c 

S_UPPER_SRCS += \
../ThirdParty/SEGGER/RTT/SEGGER_RTT_ASM_ARMv7M.S 

OBJS += \
./ThirdParty/SEGGER/RTT/SEGGER_RTT.o \
./ThirdParty/SEGGER/RTT/SEGGER_RTT_ASM_ARMv7M.o 

S_UPPER_DEPS += \
./ThirdParty/SEGGER/RTT/SEGGER_RTT_ASM_ARMv7M.d 

C_DEPS += \
./ThirdParty/SEGGER/RTT/SEGGER_RTT.d 


# Each subdirectory must supply rules for building sources it contributes
ThirdParty/SEGGER/RTT/%.o ThirdParty/SEGGER/RTT/%.su ThirdParty/SEGGER/RTT/%.cyclo: ../ThirdParty/SEGGER/RTT/%.c ThirdParty/SEGGER/RTT/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../ThirdParty/SEGGER/Config -I../ThirdParty/SEGGER/RTT -I../ThirdParty/SEGGER/SystemView -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/STM32F4xx-Nucleo -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/FloraNet/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
ThirdParty/SEGGER/RTT/%.o: ../ThirdParty/SEGGER/RTT/%.S ThirdParty/SEGGER/RTT/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -I../Core/Inc -I../Drivers/FloraNet/Inc -I../ThirdParty/SEGGER/Config -I../ThirdParty/SEGGER/RTT -I../ThirdParty/SEGGER/SystemView -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/STM32F4xx-Nucleo -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-ThirdParty-2f-SEGGER-2f-RTT

clean-ThirdParty-2f-SEGGER-2f-RTT:
	-$(RM) ./ThirdParty/SEGGER/RTT/SEGGER_RTT.cyclo ./ThirdParty/SEGGER/RTT/SEGGER_RTT.d ./ThirdParty/SEGGER/RTT/SEGGER_RTT.o ./ThirdParty/SEGGER/RTT/SEGGER_RTT.su ./ThirdParty/SEGGER/RTT/SEGGER_RTT_ASM_ARMv7M.d ./ThirdParty/SEGGER/RTT/SEGGER_RTT_ASM_ARMv7M.o

.PHONY: clean-ThirdParty-2f-SEGGER-2f-RTT

