###############################################################################
# @file main.c
#
# @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
# @date   03.01.2026
#
# @copyright Copyright (c) 2026
#
###############################################################################
#
# @brief Main file for the VP Template project
#
#
###############################################################################

#
# Define the commands which should be used to build the files
# Cross-compilation commands
CC      = arm-none-eabi-gcc
AR      = arm-none-eabi-ar
AS		= arm-none-eabi-as
LD		= arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size


###############################################################################
# Project specific options
###############################################################################

# Directory Layout
SRC_DIR	  = src
OBJ_DIR   = obj
LIB_DIR	  = lib
BLD_DIR   = build

# Locate the main libraries
HAL = $(LIB_DIR)/HAL
HAL_SRC = $(HAL)/Src

# Set the linker file which should be used
#
APP_LD_FILE = linker/App.ld
AUTH_LD_FILE = linker/Auth.ld

# Pre-Processor defines to configure the HAL library
DEF	= -DSTM32G4xx -DSTM32G474xx -DUSE_HAL_DRIVER -DF_CPU=170000000L -DDEBUG_BUILD


###############################################################################
# Flags for the Assembler, Compiler and Linker
###############################################################################

# Assembler Flags
ASFLAGS = -g -mcpu=cortex-m4 -mthumb

# Compiler Flags
CFLAGS = -c -O0 -g -mcpu=cortex-m4 -mthumb
CFLAGS += -Wall -ffunction-sections -fdata-sections -fstack-usage -fdump-rtl-expand
CFLAGS += -Wno-unused-function -nostdlib

# Linker Flags
LDFLAGS = -nostdlib -mcpu=cortex-m4 -mthumb --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections -L$(OBJ_DIR) -Wl,--start-group -lc -lgcc -lm -lnosys -lstm32 -Wl,--end-group

# Set the include search directoties
CFLAGS += -I$(SRC_DIR)
# Include files for CMSIS
CFLAGS += -I$(LIB_DIR)/Core/Include
# Include files for HAL
CFLAGS += -I$(LIB_DIR)/HAL/Inc
# Include files for general STM32G4xx
CFLAGS += -I$(LIB_DIR)/STM32G4xx/Include

# Include files for App
CFLAGS += -I$(SRC_DIR)/App
# Include files for HAL
CFLAGS += -I$(SRC_DIR)/HAL
# Include files for OS
CFLAGS += -I$(SRC_DIR)/OS
# Include files for Auth  ##I dont know if this is needed, may ask later (oleaso dont forget to ask)
CFLAGS += -I$(SRC_DIR)/Auth
# Include files for Service
CFLAGS += -I$(SRC_DIR)/Service
# Include files for Utils
CFLAGS += -I$(SRC_DIR)/Util

###############################################################################
# Source files for the HAL library
###############################################################################
STM32LIB_SRC_C = $(wildcard $(HAL_SRC)/*.c)
STM32LIB_FILENAMES_C = $(notdir $(STM32LIB_SRC_C))
OBJS_STM32LIB = $(addprefix $(OBJ_DIR)/, $(STM32LIB_FILENAMES_C:.c=.o))
vpath %.c $(dir $(STM32LIB_SRC_C))

###############################################################################
# Project specific Auth Assembler source files
###############################################################################
SRC_ASM_AUTH += $(SRC_DIR)/vectors_auth.s
SRC_ASM_AUTH += $(SRC_DIR)/startup_auth.s
FILENAMES_ASM_AUTH = $(notdir $(SRC_ASM_AUTH))
OBJFILENAMES_ASM_AUTH = $(FILENAMES_ASM_AUTH:.s=.o)
OBJS_ASM_AUTH = $(addprefix $(OBJ_DIR)/, $(OBJFILENAMES_ASM_AUTH:.s=.o))
vpath %.s $(dir $(SRC_ASM_AUTH))
vpath %.S $(dir $(SRC_ASM_AUTH))

###############################################################################
# Project specific Application Assembler source files
###############################################################################
SRC_ASM_APP += $(SRC_DIR)/vectors_app.s
SRC_ASM_APP += $(SRC_DIR)/startup_app.s
FILENAMES_ASM_APP = $(notdir $(SRC_ASM_APP))
OBJFILENAMES_ASM_APP = $(FILENAMES_ASM_APP:.s=.o)
OBJS_ASM_APP = $(addprefix $(OBJ_DIR)/, $(OBJFILENAMES_ASM_APP:.s=.o))
vpath %.s $(dir $(SRC_ASM_APP))
vpath %.S $(dir $(SRC_ASM_APP))

###############################################################################
# Project specific Application C-Source files
###############################################################################
APP_SRC_C += $(SRC_DIR)/main_app.c
APP_SRC_C += $(SRC_DIR)/System.c
APP_SRC_C += $(wildcard $(SRC_DIR)/App/*.c)
APP_SRC_C += $(wildcard $(SRC_DIR)/HAL/*.c)
APP_SRC_C += $(wildcard $(SRC_DIR)/OS/*.c)
APP_SRC_C += $(wildcard $(SRC_DIR)/Service/*.c)
APP_SRC_C += $(wildcard $(SRC_DIR)/Util/*.c)
APP_SRC_C += $(wildcard $(SRC_DIR)/Util/Log/*.c)
APP_SRC_C += $(wildcard $(SRC_DIR)/Util/Filter/*.c)
APP_SRC_C += $(wildcard $(SRC_DIR)/Util/StateTable/*.c)
APP_FILENAMES_S	= $(notdir $(APP_SRC_C))
APP_OBJS_C = $(addprefix $(OBJ_DIR)/, $(APP_FILENAMES_S:.c=.o))
vpath %.c $(dir $(APP_SRC_C))

###############################################################################
# Project specific Authenticator C-Source files
###############################################################################
AUTH_SRC_C += $(SRC_DIR)/main_auth.c
AUTH_SRC_C += $(SRC_DIR)/System.c
AUTH_SRC_C += $(wildcard $(SRC_DIR)/Auth/*.c)
AUTH_SRC_C += $(wildcard $(SRC_DIR)/HAL/*.c)
AUTH_SRC_C += $(wildcard $(SRC_DIR)/OS/*.c)
AUTH_SRC_C += $(wildcard $(SRC_DIR)/Service/*.c)
AUTH_SRC_C += $(wildcard $(SRC_DIR)/Util/*.c)
AUTH_SRC_C += $(wildcard $(SRC_DIR)/Util/Log/*.c)
AUTH_SRC_C += $(wildcard $(SRC_DIR)/Util/Filter/*.c)
AUTH_SRC_C += $(wildcard $(SRC_DIR)/Util/StateTable/*.c)
AUTH_FILENAMES_S	= $(notdir $(AUTH_SRC_C))
AUTH_OBJS_C = $(addprefix $(OBJ_DIR)/, $(AUTH_FILENAMES_S:.c=.o))
vpath %.c $(dir $(AUTH_SRC_C))

DEPS := $(APP_OBJS_C:.o=.d)

all: $(BLD_DIR) $(OBJ_DIR) $(BLD_DIR)/app.bin $(BLD_DIR)/auth.bin

###############################################################################
# Rules
###############################################################################
$(BLD_DIR):
	@echo "Creating Build Directory"
	@mkdir -p $(BLD_DIR)

$(OBJ_DIR):
	@echo "Creating Object Directory"
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.s
	@echo "  AS      $(notdir $@)"
	@$(AS) $(ASFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.S
	@echo "  AS      $(notdir $@)"
	@$(AS) $(ASFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.c
	@echo "  CC      $(notdir $@)"
	@$(CC) -c $(CFLAGS) $(DEF) -MP -MMD $< -o $@

$(OBJ_DIR)/libstm32.a: $(OBJS_STM32LIB)
	@echo "  AR      $(notdir $@)"
	@ar rcs $@ $(OBJS_STM32LIB)

$(BLD_DIR)/app.elf: $(OBJ_DIR)/libstm32.a $(OBJS_ASM_APP) $(APP_OBJS_C) | $(APP_LD_FILE)
	@echo "  LD      $(notdir $@)"
	@$(LD) $^ $(LDFLAGS) $(INC) -T$(APP_LD_FILE) -Wl,-Map $(BLD_DIR)/app.map -o $@

$(BLD_DIR)/auth.elf: $(OBJ_DIR)/libstm32.a $(OBJS_ASM_AUTH) $(AUTH_OBJS_C) | $(AUTH_LD_FILE)
	@echo "  LD      $(notdir $@)"
	@$(LD) $^ $(LDFLAGS) $(INC) -T$(AUTH_LD_FILE) -Wl,-Map $(BLD_DIR)/auth.map -o $@

$(BLD_DIR)/%.bin: $(BLD_DIR)/%.elf
	@echo "  OBJCOPY $(notdir $@)"
	@arm-none-eabi-objcopy $< -O binary $@

clean:
	rm -f $(BLD_DIR)/*.elf
	rm -f $(BLD_DIR)/*.bin
	rm -f $(OBJ_DIR)/*.o
	rm -f $(OBJ_DIR)/*.a
	rm -f $(OBJ_DIR)/*.su
	rm -f $(OBJ_DIR)/*.d

.PHONY: all clean

-include $(DEPS)