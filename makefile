################################################################################
# Global Project Settings
################################################################################

PROJECT_NAME := xmake

ifeq ($(OS),Windows_NT)
	PROJECT_NAME := $(PROJECT_NAME).exe
endif

CC = g++
LD = ld
AR = ar
CP = objcopy
OD = objdump
SIZE = size

################################################################################
# Directories
################################################################################

BIN_DIR := .bin
OBJECT_DIR = $(BIN_DIR)/Debug

################################################################################
# Include Directories
################################################################################

INCLUDES := \
	-Iinclude \
	-Ilib/ArduinoJson/src

################################################################################
# Source Files
################################################################################

SRC_DIRS = 	src 

SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp) $(wildcard $(dir)/*.c) $(wildcard $(dir)/*.s))

################################################################################
# Object Files
################################################################################

OBJS = $(patsubst %.cpp, $(OBJECT_DIR)/%.o, $(SRCS)) \
       $(patsubst %.c, $(OBJECT_DIR)/%.o, $(SRCS)) \
       $(patsubst %.s, $(OBJECT_DIR)/%.o, $(SRCS))

# extract only .o files for linker
LINK_OBJS = $(filter %.o, $(OBJS))

################################################################################
# Predefined Symbols
################################################################################

PREDEFINED_SYMBOLS = \
	-DDEBUG

################################################################################
# Flags
################################################################################

ALL_FLAGS = -g -std=c++23 -Wall -Wextra -pedantic -ffunction-sections -fdata-sections -MMD -Os
LINKER_FLAGS = $(PREDEFINED_SYMBOLS)

ASM_FLAGS = $(ALL_FLAGS) $(PREDEFINED_SYMBOLS) $(INCLUDES)
C_FLAGS = $(ALL_FLAGS) $(PREDEFINED_SYMBOLS) $(INCLUDES)
CPP_FLAGS = $(ALL_FLAGS) $(GPP_MISRA_RULES) $(PREDEFINED_SYMBOLS) $(INCLUDES)

####################################################################################
# auto dependency generation
####################################################################################

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJECT_DIR)/$*.d
CPP_DEPFILES = $(filter %.d, $(patsubst %.cpp, $(OBJECT_DIR)/%.d, $(SRCS)) )

################################################################################
# Default Target
################################################################################

# all: $(BIN_DIR)/$(PROJECT_NAME) copy

################################################################################
# Target Generation
################################################################################

$(BIN_DIR)/$(PROJECT_NAME): $(OBJS)
	@mkdir -p "$(dir $@)" 
	@echo Building target: $@
	$(CC) -o $@ $(LINK_OBJS) $(LINKER_FLAGS) $(LIBS) $(INCLUDES)
	@echo Finished building target: $@


$(OBJECT_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CPP_FLAGS) $(DEPFLAGS) -c -o $@ $<
	@echo Finished building: $@

$(OBJECT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) $(DEPFLAGS) -c -o $@ $<
	@echo Finished building: $@

$(OBJECT_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(ASM_FLAGS) $(DEPFLAGS) -c -o $@ $<
	@echo Finished building: $@

####################################################################################

copy:
	@cp -f xmakefile.json $(BIN_DIR)/xmakefile.json 

####################################################################################

test:
	$(MAKE) -C test

####################################################################################

info:
	@echo "SRCS"
	@echo $(SRCS)
	@echo "OBJS"
	@echo $(OBJS)
	@echo "LINK_OBJS"
	@echo $(LINK_OBJS)
	@echo "CPP_DEPFILES"
	@echo $(CPP_DEPFILES)

####################################################################################

run:
	./$(BINARY)

install:
	@echo "Installing $(PROJECT_NAME) to /usr/local/bin"
	sudo mkdir -p /usr/local/bin
	sudo cp -f $(BIN_DIR)/$(PROJECT_NAME) /usr/local/bin/$(PROJECT_NAME)
	@echo "Installed $(PROJECT_NAME) to /usr/local/bin"

####################################################################################

clean:
	rm -rf $(BIN_DIR) $(CPP_DEPFILES)

####################################################################################

.PHONY: all clean debug src lib run test

-include $(CPP_DEPFILES)