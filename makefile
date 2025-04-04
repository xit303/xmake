####################################################################################
# compiler settings 
####################################################################################

ifndef PREFIX
PREFIX := 
#C:/workspace/Compiler/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-
endif

CC := $(PREFIX)gcc
CPP := $(PREFIX)g++
ARCHIVE := $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy

####################################################################################
# output directory
# TODO this depends on the build configuration (debug/release)
####################################################################################

OBJDIR := .bin
FILENAME := xmake
BINARY := $(OBJDIR)/$(FILENAME).exe

####################################################################################
# create the INCLUDES variable by adding the included variabled from above
####################################################################################

INCLUDES := \
	-Iinclude \
	-Ilib/ArduinoJson/src

####################################################################################
# project specific defines
####################################################################################

DEFINES := 

####################################################################################
# get compiler settings 
# CC CPP ARCHIVE commands
# C_SRCS, CPP_SRCS, C_OBJS
# CFLAGS CPPFLAGS
####################################################################################

#include $(ESP32_FRAMEWORK_DIR)compiler.mk
# include compiler.mk

####################################################################################
# Find all source files
####################################################################################

rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

# if you want to change the source directory change src/ to the new source directory
C_SRCS := $(call rwildcard,src/,*.c)
CPP_SRCS := $(call rwildcard,src/,*.cpp)

####################################################################################
# This will generate the list of object files
####################################################################################

C_OBJS := $(patsubst %.c, $(OBJDIR)/%.o,$(C_SRCS))
CPP_OBJS := $(patsubst %.cpp, $(OBJDIR)/%.o,$(CPP_SRCS))

####################################################################################
# create linker flags
####################################################################################

LINK_FLAGS := 
# -Wl,--gc-sections -Wno-frame-address -mlongcalls
# LINK_FLAGS += -fno-rtti -fno-lto -Wl,--wrap=esp_log_write -Wl,--wrap=esp_log_writev 
# LINK_FLAGS += -Wl,--wrap=log_printf -Wl,--wrap=longjmp -Wl,--undefined=uxTopUsedPriority 
# LINK_FLAGS += -Wl,--cref -Wl,-Map=$(OBJDIR)/$(FILENAME)_FW$(VERSION_MAYOR)-$(VERSION_MINOR)-$(VERSION_PATCH).map

####################################################################################
# auto dependency generation
####################################################################################

DEPFILES := $(C_OBJS:.o=.d) $(CPP_OBJS:.o=.d)

-include $(DEPFILES)

####################################################################################
# build targets
####################################################################################

all: $(BINARY)

$(BINARY): $(C_OBJS) $(CPP_OBJS)
	@echo "linking " $(BINARY)
	$(CPP) -o $(BINARY) $(C_OBJS) $(CPP_OBJS) $(LINK_FLAGS)

$(OBJDIR)/%.o: %.c
	@test -d $(@D) || mkdir -p $(@D)
	@echo "building " $@
	$(CC) $(CFLAGS) -g -c $< -o $@ -MMD -MF $(@:.o=.d)

$(OBJDIR)/%.o: %.cpp
	@test -d $(@D) || mkdir -p $(@D)
	@echo "building " $@
	$(CPP) $(CPPFLAGS) -g -c $< -o $@ -MMD -MF $(@:.o=.d)

####################################################################################

info:
	@echo $(BINARY)
	@echo
	@echo "INCLUDES"
	@echo $(INCLUDES)
	@echo
	@echo "C_OBJS"
	@echo $(C_OBJS)
	@echo "C_SRCS"
	@echo $(C_SRCS)
	@echo
	@echo "CPP_OBJS"
	@echo $(CPP_OBJS)
	@echo "CPP_SRCS"
	@echo $(CPP_SRCS)
	@echo
	@echo "CFLAGS"
	@echo $(CFLAGS)
	@echo "CPPFLAGS"
	@echo $(CPPFLAGS)
	@echo
	@echo "LIBRARIES"
	@echo $(LIBRARIES)

####################################################################################

run:
	./$(BINARY)

####################################################################################

clean:
	rm -rf $(OBJDIR) $(DEPFILES)

####################################################################################

.PHONY: all clean debug src lib run
