
# Put your stlink folder here so make burn will work.
STLINK=/home/thekwan/Work/STM32/stlink

# Put your source files here (or *.c, etc)
SRCS=Src/main.c Src/system_stm32f4xx.c Src/stm32f4xx_it.c Src/stm32f4xx_hal_msp.c Src/usart.c Src/shell.c Src/sccb.c

# Library code
SRCS +=stm32f4xx_hal.c stm32f4xx_hal_rcc.c stm32f4xx_hal_pwr_ex.c stm32f4xx_hal_spi.c stm32f4xx_hal_gpio.c stm32f4xx_hal_uart.c \
	   stm32f4xx_hal_cortex.c stm32f4xx_hal_dma.c stm32f4xx_hal_i2c.c stm32f4xx_hal_ltdc.c stm32f4xx_hal_rcc_ex.c ili9341.c

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=sccb

# Put your STM32F4 library code directory here
#STM_COMMON=/home/thekwan/Work/STM32/Projects/STM32Cube_FW_F4_V1.10.0
STM_COMMON=../../../../..

# Normally you shouldn't need to change anything below this line!
#######################################################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

#CFLAGS  = -g -O2 -Wall -TTrueSTUDIO/STM32F429I-Discovery/STM32F429ZI_FLASH.ld -specs=nosys.specs
CFLAGS  = -g -Wall -TTrueSTUDIO/STM32F429I-Discovery/STM32F429ZI_FLASH.ld -specs=nosys.specs
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -DSTM32F429xx
CFLAGS += -DUSE_HAL_DRIVER
CFLAGS += -DUSE_STM32F429I_DISCO

# Include files from STM libraries
CFLAGS += -IInc
CFLAGS += -I$(STM_COMMON)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I$(STM_COMMON)/Drivers/STM32F4xx_HAL_Driver/Inc
CFLAGS += -I$(STM_COMMON)/Drivers/CMSIS/Include
CFLAGS += -I$(STM_COMMON)/Drivers/BSP/STM32F429I-Discovery

###################################################

vpath %.c \
$(STM_COMMON)/Drivers/STM32F4xx_HAL_Driver/Src \
$(STM_COMMON)/Drivers/BSP/Components/ili9341

# Includes
# ..
###################################################


# add startup file to build
SRCS += ./TrueSTUDIO/startup_stm32f429xx.s
SRCS += $(STM_COMMON)/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery.c 
OBJS = $(SRCS:.c=.o)

.PHONY: proj

all: proj

proj: $(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f *.o $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin

# Flash the STM32F4
burn: proj
	$(STLINK)/st-flash write $(PROJ_NAME).bin 0x8000000
