PORT ?= /dev/cu.usbmodem1101
BAUD ?= 115200
BUILD_DIR ?= build
DEVICE_SOURCE ?= device.c

.PHONY: all flash erase clean FORCE

all: $(BUILD_DIR)/main/device.c
	cd $(BUILD_DIR) && "$(IDF_PATH)/tools/idf.py" build

$(BUILD_DIR)/.prepared: Makefile
	mkdir -p $(BUILD_DIR)/main
	rm -f $(BUILD_DIR)/sdkconfig $(BUILD_DIR)/sdkconfig.old
	printf '%s\n' \
	  'cmake_minimum_required(VERSION 3.16)' \
	  'set(IDF_TARGET "esp32s3")' \
	  'include($$ENV{IDF_PATH}/tools/cmake/project.cmake)' \
	  'project(display)' > $(BUILD_DIR)/CMakeLists.txt
	printf '%s\n' \
	  'idf_component_register(SRCS "device.c" INCLUDE_DIRS "." REQUIRES esp_wifi esp_event esp_netif nvs_flash esp_timer esp_hw_support esp_system freertos lwip soc esp_driver_gpio)' \
	  'target_compile_options($${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)' > $(BUILD_DIR)/main/CMakeLists.txt
	printf '%s\n' \
	  'CONFIG_IDF_TARGET="esp32s3"' \
	  'CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y' \
	  'CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ=240' \
	  'CONFIG_FREERTOS_HZ=1000' \
	  'CONFIG_COMPILER_OPTIMIZATION_SIZE=y' \
	  'CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y' \
	  'CONFIG_ESPTOOLPY_FLASHSIZE="16MB"' \
	  'CONFIG_ESPTOOLPY_FLASHMODE_DIO=y' \
	  'CONFIG_ESPTOOLPY_FLASHFREQ_80M=y' \
	  'CONFIG_FREERTOS_SUPPORT_STATIC_ALLOCATION=y' > $(BUILD_DIR)/sdkconfig.defaults
	touch $(BUILD_DIR)/.prepared

$(BUILD_DIR)/main/device.c: $(DEVICE_SOURCE) $(BUILD_DIR)/.prepared FORCE
	cmp -s $(DEVICE_SOURCE) $(BUILD_DIR)/main/device.c || cp $(DEVICE_SOURCE) $(BUILD_DIR)/main/device.c

FORCE:

flash: all
	cd $(BUILD_DIR) && "$(IDF_PATH)/tools/idf.py" -p $(PORT) -b $(BAUD) flash

erase: $(BUILD_DIR)/main/device.c
	cd $(BUILD_DIR) && "$(IDF_PATH)/tools/idf.py" -p $(PORT) -b $(BAUD) erase-flash

clean:
	rm -rf $(BUILD_DIR)

HOST_CC ?= gcc
HOST_CFLAGS ?= -O3 -march=native -Wall -Wextra -Werror -std=gnu89
VIRTPANEL_CFLAGS ?= -O3 -Wall -Wextra -Werror -std=gnu89
SDL2_CONFIG ?= sdl2-config
VIRTUAL_SCALE ?= 5
VIRTUAL_CODE ?= 01
FREETYPE_CFLAGS ?= $(shell pkg-config --cflags freetype2)
FREETYPE_LIBS ?= $(shell pkg-config --libs freetype2)

.PHONY: server virtual virtual-run clean-host

server: displayd

displayd: displayd.c
	$(HOST_CC) $(HOST_CFLAGS) $(FREETYPE_CFLAGS) -pthread -o $@ displayd.c -lm $(FREETYPE_LIBS)

virtual: virtpanel

virtual-run: virtpanel
	./virtpanel $(VIRTUAL_SCALE) $(VIRTUAL_CODE)

virtpanel: virtpanel.c
	$(HOST_CC) $(VIRTPANEL_CFLAGS) $(shell $(SDL2_CONFIG) --cflags) -o $@ virtpanel.c $(shell $(SDL2_CONFIG) --libs)

clean-host:
	rm -f displayd virtpanel displayd.sock displayd.pid displayd.log
