PROGRAM = sonoff-s20
PROGRAM_SRC_DIR = ./src

EXTRA_COMPONENTS = \
	extras/http-parser \
	extras/dhcpserver \
	extras/dht \
	extras/rboot-ota \
	extras/mbedtls \
	$(abspath components/http-client-ota) \
	$(abspath components/wifi-config) \
	$(abspath components/wolfssl) \
	$(abspath components/cJSON) \
	$(abspath components/homekit)

FLASH_SIZE = 8
FLASH_MODE = dout
FLASH_SPEED = 40

HOMEKIT_SMALL = 0
HOMEKIT_MAX_CLIENTS = 14
HOMEKIT_OVERCLOCK = 1
HOMEKIT_OVERCLOCK_PAIR_SETUP = 1
HOMEKIT_OVERCLOCK_PAIR_VERIFY = 1

HOMEKIT_SPI_FLASH_BASE_ADDR = 0x7A000

EXTRA_CFLAGS += -DHOMEKIT_SHORT_APPLE_UUIDS

OTA_UPDATE_SERVER ?= 192.168.2.2
OTA_UPDATE_PORT ?= 8080
OTA_UPDATE_PATH ?= "/ota/sonoff-s20"
OTA_UPDATE_FIRMWARE_NAME ?= latest

EXTRA_CFLAGS += \
-DOTA_UPDATE_SERVER=\"$(OTA_UPDATE_SERVER)\" \
-DOTA_UPDATE_PORT=\"$(OTA_UPDATE_PORT)\" \
-DOTA_UPDATE_PATH=\"$(OTA_UPDATE_PATH)\" \
-DOTA_UPDATE_FIRMWARE_NAME=\"$(OTA_UPDATE_FIRMWARE_NAME)\"

ESPTOOL2 := $(shell which esptool2)
OPENSSL := $(shell which openssl)

HOMEKITSETTINGS = utils/generateHAPSettings.py

OTA_DIR = $(FIRMWARE_DIR)ota/

sonoff_s20_slot_0 : LINKER_SCRIPTS = $(PROGRAM_DIR)ld/slot_0.ld $(ROOT)ld/rom.ld
sonoff_s20_slot_0 : override PROGRAM = sonoff_s20_slot_0

sonoff_s20_slot_1 : LINKER_SCRIPTS = $(PROGRAM_DIR)ld/slot_1.ld $(ROOT)ld/rom.ld
sonoff_s20_slot_1 : override PROGRAM = sonoff_s20_slot_1

all : ota-images

include $(SDK_PATH)/common.mk

homekitSettings:
	$(Q) $(HOMEKITSETTINGS) $(PROGRAM_SRC_DIR)/homekit_settings.h $(PROGRAM_DIR)qrcode.png

$(OTA_DIR):
	mkdir -p $@

sonoff_s20_slot_0: $(WHOLE_ARCHIVES) $(COMPONENT_ARS) $(BUILD_DIR)libgcc.a $(BUILD_DIR)libc.a $(SDK_PROCESSED_LIBS) $(LINKER_SCRIPTS)
	$(vecho) "Linking OTA firmware for slot 0"
	$(Q) $(LD) $(LDFLAGS) -Wl,--whole-archive $(WHOLE_ARCHIVES) -Wl,--no-whole-archive -Wl,--start-group $(COMPONENT_ARS) $(BUILD_DIR)libgcc.a $(BUILD_DIR)libc.a $(LIB_ARGS) $(SDK_LIB_ARGS) -Wl,--end-group -o $(BUILD_DIR)$@.out

sonoff_s20_slot_1: $(WHOLE_ARCHIVES) $(COMPONENT_ARS) $(BUILD_DIR)libgcc.a $(BUILD_DIR)libc.a $(SDK_PROCESSED_LIBS) $(LINKER_SCRIPTS)
	$(vecho) "Linking OTA firmware for slot 1"
	$(Q) $(LD) $(LDFLAGS) -Wl,--whole-archive $(WHOLE_ARCHIVES) -Wl,--no-whole-archive -Wl,--start-group $(COMPONENT_ARS) $(BUILD_DIR)libgcc.a $(BUILD_DIR)libc.a $(LIB_ARGS) $(SDK_LIB_ARGS) -Wl,--end-group -o $(BUILD_DIR)$@.out

ota-images: $(OTA_DIR) sonoff_s20_slot_0 sonoff_s20_slot_1
	$(vecho) "Building OTA firmware images"
	$(Q) $(ESPTOOL2) -quiet -bin -boot2 $(BUILD_DIR)sonoff_s20_slot_0.out $(OTA_DIR)$(OTA_UPDATE_FIRMWARE_NAME)_0.bin .text .data .rodata
	$(Q) $(OPENSSL) sha256 < $(OTA_DIR)$(OTA_UPDATE_FIRMWARE_NAME)_0.bin > $(OTA_DIR)$(OTA_UPDATE_FIRMWARE_NAME)_0.sha256
	$(Q) $(ESPTOOL2) -quiet -bin -boot2 $(BUILD_DIR)sonoff_s20_slot_1.out $(OTA_DIR)$(OTA_UPDATE_FIRMWARE_NAME)_1.bin .text .data .rodata
	$(Q) $(OPENSSL) sha256 < $(OTA_DIR)$(OTA_UPDATE_FIRMWARE_NAME)_1.bin > $(OTA_DIR)$(OTA_UPDATE_FIRMWARE_NAME)_1.sha256

monitor:
	$(FILTEROUTPUT) --port $(ESPPORT) --baud 115200 --elf $(PROGRAM_OUT)
