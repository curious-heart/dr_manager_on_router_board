# Global target; when 'make' is run without arguments, this is what it should do
#APP_LOG_LEVEL=0 #0-DEBUG, 1-INFO, 2-WARN, 3-ERROR. refer to logger.h.

BUILD_TYPE=release
BUILD_DATE = $(shell TZ='Asia/Shanghai' date +%Y%m%d%H%M%S)
OBJ = ./obj

INC = . ./common_tools ./hv_controller ./mb_tcp_server_test ./gpio_key_processor ./op_gpio_thu_reg ./dap_calc
SRC = . ./common_tools ./hv_controller ./dap_calc

ifeq ($(CONFIG_manage_lcd_and_tof_here),y)
INC += ./lcd_display ./tof_measure ./tof_measure/core/inc ./tof_measure/platform/inc
SRC += ./lcd_display ./tof_measure ./tof_measure/core/src ./tof_measure/platform/src
endif

TARGET = dr_manager
TCP_SRVR_TEST_TARGET = mb_tcp_test_client
GPIO_KEY_PROCESSOR_TARGET = gpio_key_monitor

all: prepare $(TARGET) $(TCP_SRVR_TEST_TARGET) $(GPIO_KEY_PROCESSOR_TARGET)

prepare:
	@echo $(SOURCES)
	@mkdir -p $(OBJ)

INCLUDES = $(wildcard $(addsuffix /*.h, $(INC)))
SOURCES = $(wildcard $(addsuffix /*.c, $(SRC)))
SOURCES += ./gpio_key_processor/gpio_key_app_version_def.c
ifeq ($(CONFIG_manage_lcd_and_tof_here),n)
SOURCES := $(filter-out ./lcd_refresh_thread.c ./tof_thread.c, $(SOURCES))
INCLUDES := $(filter-out ./lcd_resource.h, $(INCLUDES))
endif

TCP_SRVR_TEST_SOURCES = ./mb_tcp_server_test/mb_tcp_server_test.c /hv_controller/hv_registers.c ./common_tools/logger.c ./common_tools/get_opt_helper.c ./common_tools/common_tools.c
GPIO_KEY_PROCESSOR_SOURCES = $(wildcard ./gpio_key_processor/*.c) ./common_tools/logger.c ./common_tools/get_opt_helper.c ./common_tools.c ./hv_controller/hv_controller.c ./hv_controller/hv_registers.c ./op_gpio_thu_reg/op_gpio_thu_reg.c

# These variables hold the name of the compilation tool, the compilation flags and the link flags
# We make use of these variables in the package manifest
CC = gcc
override CFLAGS += -Wall $(addprefix -I, $(INC)) -pthread -DBUILD_DATE_STR="\"$(BUILD_DATE)\"" \
	-DBUILD_TYPE_STR="\"$(BUILD_TYPE)\"" -DUSE_I2C_2V8
override LDLIBS += -lm -lmodbus -pthread -lsqlite3

ifeq ($(CONFIG_manage_lcd_and_tof_here),y)
override CFLAGS += -DMANAGE_LCD_AND_TOF_HERE
endif

DEPS = $(INCLUDES)
OBJECTS = $(patsubst %.c, $(OBJ)/%.o, $(notdir $(SOURCES))) 
TCP_SRVR_TEST_OBJECTS = $(patsubst %.c, $(OBJ)/%.o, $(notdir $(TCP_SRVR_TEST_SOURCES)))
GPIO_KEY_PROCESSOR_OBJECTS = $(patsubst %.c, $(OBJ)/%.o, $(notdir $(GPIO_KEY_PROCESSOR_SOURCES)))

.SECONDEXPANSION:
# This rule builds individual object files, and depends on the corresponding C source files and the header files

$(OBJ)/%.o: ./%.c $(DEPS)
	#$(CC) -E -C -o $@ $< $(CFLAGS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./common_tools/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./hv_controller/%.c $(DEPS)
	#$(CC) -E -C -o $@ $< $(CFLAGS)
	$(CC) -c -o $@ $< $(CFLAGS)
ifeq ($(CONFIG_manage_lcd_and_tof_here),y)
$(OBJ)/%.o: ./lcd_display/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./tof_measure/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./tof_measure/core/src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./tof_measure/platform/src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
endif
$(OBJ)/%.o: ./mb_tcp_server_test/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./gpio_key_processor/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./op_gpio_thu_reg/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(OBJ)/%.o: ./dap_calc/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(TCP_SRVR_TEST_TARGET): $(TCP_SRVR_TEST_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(GPIO_KEY_PROCESSOR_TARGET): $(GPIO_KEY_PROCESSOR_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# To clean build artifacts, we specify a 'clean' rule, and use PHONY to indicate that this rule never matches with a potential file in the directory
.PHONY: prepare clean

clean:
	rm -f $(TARGET) $(TCP_SRVR_TEST_TARGET) $(GPIO_KEY_PROCESSOR_TARGET)
	rm -rf $(OBJ)
