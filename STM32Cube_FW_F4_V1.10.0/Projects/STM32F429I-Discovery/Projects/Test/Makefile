
# Put your stlink folder here so make burn will work.
#STLINK=/home/thekwan/Work/STM32/stlink

# Put your source files here (or *.c, etc)
SRCS=main.c                      \
	 system_stm32f4xx.c          \
	 stm32f4xx_it.c              \
	 stm32f4xx_hal_msp.c 

# Library code
SRCS += stm32f4xx_hal.c              \
		stm32f4xx_hal_rcc.c          \
		stm32f4xx_hal_pwr_ex.c       \
		stm32f4xx_hal_spi.c          \
		stm32f4xx_hal_gpio.c         \
		stm32f4xx_hal_uart.c         \
	   	stm32f4xx_hal_cortex.c       \
		stm32f4xx_hal_dma.c          \
		stm32f4xx_hal_i2c.c          \
		stm32f4xx_hal_ltdc.c         \
		stm32f4xx_hal_sdram.c        \
		stm32f4xx_hal_dma.c          \
		stm32f4xx_hal_dma2d.c        \
		stm32f4xx_hal_dma_ex.c       \
		stm32f4xx_hal_rcc_ex.c       \
		stm32f4xx_ll_fmc.c           \
		ili9341.c

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=Test

# Put your STM32F4 library code directory here
STM_COMMON=/home/deokhwan/Workspace/Stm32/Stm32f4Cube/STM32Cube_FW_F4_V1.10.0
#STM_COMMON=../../../../..

# Normally you shouldn't need to change anything below this line!
#######################################################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

#CFLAGS  = -g -O2 -Wall -TTrueSTUDIO/STM32F429I-Discovery/STM32F429ZI_FLASH.ld -specs=nosys.specs
CFLAGS  = -g -Wall -TTrueSTUDIO/STM32F429I_DISCO/STM32F429ZI_FLASH.ld -specs=nosys.specs
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -DSTM32F429xx
CFLAGS += -DUSE_HAL_DRIVER
CFLAGS += -DUSE_STM32F429I_DISCO
#CFLAGS += -DLCD_IF_MCU_ONLY

# Include files from STM libraries
CFLAGS += -IInc
CFLAGS += -I$(STM_COMMON)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I$(STM_COMMON)/Drivers/STM32F4xx_HAL_Driver/Inc
CFLAGS += -I$(STM_COMMON)/Drivers/CMSIS/Include
CFLAGS += -I$(STM_COMMON)/Drivers/BSP/STM32F429I-Discovery
CFLAGS += -I$(STM_COMMON)/Drivers/BSP/Components/ili9341

###################################################

VPATH += Src
VPATH += $(STM_COMMON)/Drivers/STM32F4xx_HAL_Driver/Src
VPATH += $(STM_COMMON)/Drivers/BSP/STM32F429I-Discovery
VPATH += $(STM_COMMON)/Drivers/BSP/Components/ili9341

# Includes
# ..
###################################################


ODIR = obj
# add startup file to build
ASMS += ./TrueSTUDIO/startup_stm32f429xx.s
SRCS += $(STM_COMMON)/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery.c 
SRCS += $(STM_COMMON)/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_lcd.c 
SRCS += $(STM_COMMON)/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_sdram.c 
_OBJ = $(notdir $(SRCS:.c=.o))
OBJS = $(patsubst %, $(ODIR)/%,$(_OBJ))

.PHONY: proj

all: proj

proj: $(PROJ_NAME).elf

$(ODIR)/%.o: %.c
	@mkdir -p $(ODIR)
	@echo "CC $(notdir $<)"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(PROJ_NAME).elf: $(OBJS) $(ASMS)
	@echo "---------------------------------------------"
	@echo "- BUILD PROGRAM                             -"
	@echo "---------------------------------------------"
	@$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	@$(OBJCOPY) -O ihex   $(PROJ_NAME).elf $(PROJ_NAME).hex
	@$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	@rm -rf *.o $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin obj

# Flash the STM32F4
burn: proj
	@echo "---------------------------------------------"
	@echo "- FLASH WRITING...                          -"
	@echo "---------------------------------------------"
	st-flash write $(PROJ_NAME).bin 0x8000000
