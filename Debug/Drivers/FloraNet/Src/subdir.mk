################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/FloraNet/Src/bh1750.c \
../Drivers/FloraNet/Src/sht3x.c \
../Drivers/FloraNet/Src/soil_moisture.c \
../Drivers/FloraNet/Src/w25qxx.c 

OBJS += \
./Drivers/FloraNet/Src/bh1750.o \
./Drivers/FloraNet/Src/sht3x.o \
./Drivers/FloraNet/Src/soil_moisture.o \
./Drivers/FloraNet/Src/w25qxx.o 

C_DEPS += \
./Drivers/FloraNet/Src/bh1750.d \
./Drivers/FloraNet/Src/sht3x.d \
./Drivers/FloraNet/Src/soil_moisture.d \
./Drivers/FloraNet/Src/w25qxx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/FloraNet/Src/%.o Drivers/FloraNet/Src/%.su Drivers/FloraNet/Src/%.cyclo: ../Drivers/FloraNet/Src/%.c Drivers/FloraNet/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../ThirdParty/SEGGER/Config -I../ThirdParty/SEGGER/RTT -I../ThirdParty/SEGGER/SystemView -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/STM32F4xx-Nucleo -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/FloraNet/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-FloraNet-2f-Src

clean-Drivers-2f-FloraNet-2f-Src:
	-$(RM) ./Drivers/FloraNet/Src/bh1750.cyclo ./Drivers/FloraNet/Src/bh1750.d ./Drivers/FloraNet/Src/bh1750.o ./Drivers/FloraNet/Src/bh1750.su ./Drivers/FloraNet/Src/sht3x.cyclo ./Drivers/FloraNet/Src/sht3x.d ./Drivers/FloraNet/Src/sht3x.o ./Drivers/FloraNet/Src/sht3x.su ./Drivers/FloraNet/Src/soil_moisture.cyclo ./Drivers/FloraNet/Src/soil_moisture.d ./Drivers/FloraNet/Src/soil_moisture.o ./Drivers/FloraNet/Src/soil_moisture.su ./Drivers/FloraNet/Src/w25qxx.cyclo ./Drivers/FloraNet/Src/w25qxx.d ./Drivers/FloraNet/Src/w25qxx.o ./Drivers/FloraNet/Src/w25qxx.su

.PHONY: clean-Drivers-2f-FloraNet-2f-Src

