CROSS_COMPILE ?= arm-none-eabi-
BMP_BOOTLOADER ?=
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS +=                          	\
	-I../stm32/include                 \
	-I../include                 	\
	-Itarget			\
	-mcpu=cortex-m4                 \
	-mthumb                         \
	-mfloat-abi=hard                \
	-mfpu=fpv4-sp-d16               \
	-DSTM32F4                       \
	-I../libopencm3/include         \
	-I../include/stm32        \
	-I../include/$(BOARD) \
	-I../include/blackpill-f4

LDFLAGS +=                      \
	-lopencm3_stm32f4       \
	-Wl,-T,$(LINKER_SCRIPT) \
	-nostartfiles           \
	-lc                     \
	-lnosys                 \
	-Wl,-Map=mapfile        \
	-mthumb                 \
	-mcpu=cortex-m4         \
	-Wl,-gc-sections        \
	-mfloat-abi=hard        \
	-mfpu=fpv4-sp-d16       \
	-L../libopencm3/lib

CFLAGS += -DDFU_SERIAL_LENGTH=13

VPATH +=                          \
	include/stm32        \
	include/blackpill-f4

SRC +=               \
	serialno.c       \

all:	$(NAME).bin 

host_clean:
	-$(Q)$(RM) usb-in-out.bin
