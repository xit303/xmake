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

ALL_FLAGS = -g -std=c++23 -Wall -Wextra 
LINKER_FLAGS = $(PREDEFINED_SYMBOLS)
COMPILER_OPTIONS = $(ALL_FLAGS) $(PREDEFINED_SYMBOLS) $(INCLUDES)
ASSEMBLER_FLAGS = $(ALL_FLAGS) $(PREDEFINED_SYMBOLS) $(INCLUDES)
CPP_FLAGS = $(ALL_FLAGS) $(GPP_MISRA_RULES) $(PREDEFINED_SYMBOLS) $(INCLUDES)

####################################################################################
# auto dependency generation
####################################################################################

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJECT_DIR)/$*.d
CPP_DEPFILES = $(filter %.d, $(patsubst %.cpp, $(OBJECT_DIR)/%.d, $(SRCS)) )

-include $(CPP_DEPFILES)

################################################################################
# Default Target
################################################################################

all: $(BIN_DIR)/$(PROJECT_NAME)

################################################################################
# Target Generation
################################################################################

$(BIN_DIR)/$(PROJECT_NAME): $(OBJS)
	@mkdir -p "$(dir $@)" 
	@echo Building target: $@
	$(CC) -o $@ $(LINK_OBJS) $(LINKER_FLAGS) $(LIBS) $(INCLUDES)
	@echo Finished building target: $@
	@cp -f xmakefile.json $(BIN_DIR)/xmakefile.json 

$(OBJECT_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(COMPILER_OPTIONS) $(CPP_FLAGS) -c -o $@ $<
	@echo Finished building: $@

$(OBJECT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(COMPILER_OPTIONS) -c -o $@ $<
	@echo Finished building: $@

$(OBJECT_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(ASSEMBLER_FLAGS) -c -o $@ $<
	@echo Finished building: $@

####################################################################################

info:
	@echo "SRCS"
	@echo $(SRCS)
	@echo "OBJS"
	@echo $(OBJS)
	@echo "CPP_DEPFILES"
	@echo $(CPP_DEPFILES)

####################################################################################

run:
	./$(BINARY)

####################################################################################

clean:
	rm -rf $(BIN_DIR) $(DEPFILES)

####################################################################################

.PHONY: all clean debug src lib run
