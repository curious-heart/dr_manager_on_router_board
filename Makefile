# Global target; when 'make' is run without arguments, this is what it should do
APP_LOG_LEVEL=0 #0-DEBUG, 1-INFO, 2-WARN, 3-ERROR. refer to logger.h.

OBJ = ./obj

INC = . ./common_tools ./hv_controller ./lcd_display ./tof_measure ./tof_measure/core/inc ./tof_measure/platform/inc
SRC = . ./common_tools ./hv_controller ./lcd_display ./tof_measure ./tof_measure/core/src ./tof_measure/platform/src

TARGET = dr_manager
all: prepare $(TARGET)

prepare:
	@mkdir -p $(OBJ)
 

INCLUDES = $(wildcard $(addsuffix /*.h, $(INC)))
SOURCES = $(wildcard $(addsuffix /*.c, $(SRC)))

# These variables hold the name of the compilation tool, the compilation flags and the link flags
# We make use of these variables in the package manifest
CC = gcc
override CFLAGS += -Wall $(addprefix -I, $(INC)) -DAPP_LOG_LEVEL=$(APP_LOG_LEVEL)
override LDFLAGS += -lm -lmodbus
 
DEPS = $(INCLUDES)
OBJECTS = $(patsubst %.c, $(OBJ)/%.o, $(notdir $(SOURCES)))
 
.SECONDEXPANSION:
# This rule builds individual object files, and depends on the corresponding C source files and the header files

$(OBJ)/%.o: ./%.c $(DEPS)
	#$(CC) -E -C -o $@ $< $(CFLAGS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./common_tools/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./hv_controller/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./lcd_display/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./tof_measure/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./tof_measure/core/src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./tof_measure/platform/src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
 
# To clean build artifacts, we specify a 'clean' rule, and use PHONY to indicate that this rule never matches with a potential file in the directory
.PHONY: prepare clean
 
clean:
	rm -f $(TARGET) $(OBJ)/*.o  
