BOARD ?= blackpill-f411ce
BOARD_DIR = ../common/boards/$(BOARD)
VPATH += ../common $(BOARD_DIR) target

# CFLAGS += -Wall -Wextra -Werror -Wreturn-type \
CFLAGS += -Wall -Wextra -Wreturn-type \
	-I../include \
	-I../include/$(BOARD) \
	-Wno-char-subscripts \
	-std=c11 -ggdb3 -MD -I./target \
	-I. -Iinclude -I../include -I$(BOARD_DIR) \

SRC =                     \
	main.c            \
	usbwrap.c         \
	setup.c           \
	util.c            \
	max31865.c        \

ifndef TARGET
NAME = $(notdir $(CURDIR))
TARGET = $(NAME).elf
endif


$(info We are in $(notdir $(CURDIR)))
include $(BOARD_DIR)/Makefile.inc
# include ../rules.mk
OPT_FLAGS ?= -Os
CFLAGS += $(OPT_FLAGS)
LDFLAGS += $(OPT_FLAGS) -lm -ggdb3
# Output memory usage information
LDFLAGS += -Wl,--print-memory-usage

include ../common/boards/common/Makefile.inc

OBJ = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SRC)))
$(info OBJ is $(OBJ))
$(info LDFLAGS is $(LDFLAGS))

$(TARGET): ../include/version.h $(OBJ)
	@echo "  LD      $@"
	$(Q)$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o:	%.c
	@echo "  CC      $<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.o:	%.S
	@echo "  AS      $<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.bin:	%.elf
	@echo "  OBJCOPY $@"
	$(Q)$(OBJCOPY) -O binary $^ $@

%.hex:	%.elf
	@echo "  OBJCOPY $@"
	$(Q)$(OBJCOPY) -O ihex $^ $@

.PHONY:	clean host_clean all_boards clang-format FORCE

clean:	host_clean
	$(Q)echo "  CLEAN"
	-$(Q)$(RM) *.o *.d *.elf *.bin *~ $(TARGET) $(HOSTFILES)
	-$(Q)$(RM) boards/*/*.o boards/*/*.d mapfile ftd2xx.dll

clang-format:
	$(Q)clang-format -i *.c */*.c */*/*.c *.h */*.h */*/*.h

