/*

 Sonoff S20 HomeKit
 Copyright (C) 2018  Carsten Duvenhorst

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>
#include <sysparam.h>

#include "http_client_ota.h"

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

#include "led_status.h"
#include "button.h"
#include "debug.h"

// Generate new settings (and a new qrcode.png) with "make homekitSettings"
#include "homekit_settings.h"

#include "hap_device.h"

char setupId[5] = SETUP_ID;
char setupCode[11] = SETUP_CODE;

#define vTaskDelayMs(ms) vTaskDelay((ms) / portTICK_PERIOD_MS)

const int buttonGPIO = 0;
const int relayGPIO = 12;
const int ledGPIO = 13;

led_status_t *greenLedStatus;

// MARK: Homekit server configuration
homekit_server_config_t config = {
	.accessories = accessories,
	.setupCode = SETUP_CODE,
	.setupIdentifier = SETUP_ID
};

void setRelay(bool on)
{
	DEBUG("Switching relay %d", on);
	gpio_write(relayGPIO, on ? 1 : 0);
}

// set the startOTA system start parameter to true and restart.
void ota_prepare_update_task()
{
	sysparam_set_bool("startOTA", true);

	sdk_system_restart();
	vTaskDelete(NULL);
}

void ota_prepare_update()
{
	DEBUG("Preparing OTA-Update");
	xTaskCreate(ota_prepare_update_task, "OTA Update", 256, NULL, 2, NULL);
}

void button_callback(uint8_t gpio, button_event_t event)
{
	switch (event) {

	case button_event_single_press:
		DEBUG("Send HAP notification to outlet on characteristic.");

		outlet_1_CharacteristicOn.value.bool_value =
		    !outlet_1_CharacteristicOn.value.bool_value;

		homekit_characteristic_notify(&outlet_1_CharacteristicOn,
					      outlet_1_CharacteristicOn.value);

		break;

	case button_event_long_press:
		ota_prepare_update();
		break;

	default:
		DEBUG("Unknown button event: %d", event);
	}

}

void ota_button_callback(uint8_t gpio, button_event_t event)
{

	static bool on = false;

	switch (event) {

	case button_event_single_press:
		DEBUG("Toggling relay");
		on = !on;
		setRelay(on);
		break;

	case button_event_long_press:
		sdk_system_restart();
		break;

	default:
		DEBUG("Unknown button event: %d", event);
	}

}

void initGPIO()
{
	//Initialize GPIOs
	gpio_enable(ledGPIO, GPIO_OUTPUT);
	gpio_write(ledGPIO, 0);

	gpio_enable(relayGPIO, GPIO_OUTPUT);
	setRelay(0);
}

void identify_task(void *_args)
{

	greenLedStatus = led_status_init(ledGPIO);
	led_status_set(greenLedStatus, &identifyPattern);

	vTaskDelayMs(5000);

	led_status_done(greenLedStatus);

	vTaskDelete(NULL);

}

void identify(homekit_value_t _value)
{

	DEBUG("Identify");
	xTaskCreate(identify_task, "Sonoff S20 identify", 128, NULL, 2, NULL);

}

void outlet_1_Characteristic_On_Callback(homekit_characteristic_t * _ch,
					 homekit_value_t on, void *context)
{

	DEBUG("ON callback; received bool value = %d", on.bool_value);
	setRelay(on.bool_value);

	homekit_characteristic_notify(&outlet_1_CharacteristicOutletInUse, on);
}

void setupPairingInfo(homekit_server_config_t * configuration)
{

	char *storedSetupCode;
	char *storedSetupId;

	sysparam_status_t status =
	    sysparam_get_string("setupCode", &storedSetupCode);

	if (status == SYSPARAM_OK) {
		snprintf(setupCode, 11, "%s", storedSetupCode);
		free(storedSetupCode);
	} else {
		sysparam_set_string("setupCode", SETUP_CODE);
		snprintf(setupCode, 11, "%s", SETUP_CODE);
	}
	setupCode[10] = 0;

	configuration->setupCode = setupCode;
	DEBUG("SetupCode == %s\n", setupCode);

	status = sysparam_get_string("setupId", &storedSetupId);

	if (status == SYSPARAM_OK) {
		snprintf(setupId, 5, "%s", storedSetupId);
		free(storedSetupId);
	} else {
		sysparam_set_string("setupId", SETUP_ID);
		snprintf(setupId, 5, "%s", SETUP_ID);
	}
	setupId[4] = 0;

	configuration->setupIdentifier = setupId;
	DEBUG("SetupId == %s\n", setupId);

	return;
}

void createHAPAccessory(homekit_server_config_t * configuration)
{

	setupPairingInfo(configuration);

	uint8_t macaddr[6];
	sdk_wifi_get_macaddr(STATION_IF, macaddr);

	int name_len = snprintf(NULL, 0, "%s-%s-%02X%02X%02X",
				DEVICE_NAME,
				DEVICE_MODEL,
				macaddr[3],
				macaddr[4],
				macaddr[5]);

	if (name_len > 63) {
		name_len = 63;
	}

	char *name_value = malloc(name_len + 1);

	snprintf(name_value, name_len + 1, "%s-%s-%02X%02X%02X",
		 DEVICE_NAME, DEVICE_MODEL, macaddr[3], macaddr[4], macaddr[5]);

	accessoryInformationNameCharacteristic.value =
	    HOMEKIT_STRING(name_value);

	int serialLength = snprintf(NULL, 0, "%d", sdk_system_get_chip_id());

	char *serialNumberValue = malloc(serialLength + 1);
	snprintf(serialNumberValue, serialLength + 1, "%d",
		 sdk_system_get_chip_id());

	accessoryInformationSerialCharacteristic.value =
	    HOMEKIT_STRING(serialNumberValue);
	identifyCharacteristic.setter = identify;

	// Set callback on outlet on characteristic.
	homekit_characteristic_add_notify_callback(&outlet_1_CharacteristicOn,
						   outlet_1_Characteristic_On_Callback,
						   NULL);
}

void setupAccessory()
{
	createHAPAccessory(&config);
	homekit_server_init(&config);
}

static ota_info ota_update_info = {
	.server = OTA_UPDATE_SERVER,
	.port = OTA_UPDATE_PORT,
	.path = OTA_UPDATE_PATH,
	.basename = OTA_UPDATE_FIRMWARE_NAME,
	.checkSHA256 = true,
};

static void firmwareDownloadTask(void *PvParameter)
{

	greenLedStatus = led_status_init(ledGPIO);
	led_status_set(greenLedStatus, &otaUpdatePattern);

	while (1) {
		OTA_err err;
		// Remake this task until ota work
		err = ota_update((ota_info *) PvParameter);

		if (err != OTA_UPDATE_DONE) {
			vTaskDelayMs(1000);
			continue;
		}

		vTaskDelayMs(3000);
		led_status_done(greenLedStatus);
		DEBUG("Booting updated firmware.\n");
		sdk_system_restart();
	}
}

void wifiReadyCallback()
{

	led_status_done(greenLedStatus);

	bool ota = false;

	sysparam_status_t status = sysparam_get_bool("startOTA", &ota);
	if (status == SYSPARAM_OK) {
		if (ota) {
			sysparam_set_bool("startOTA", false);

			button_delete(buttonGPIO);

			if (button_create
			    (buttonGPIO, 0, 4000, ota_button_callback)) {
				DEBUG("Failed to initialize button.");
			}

			DEBUG("Looking for updates.\n");
			xTaskCreate(firmwareDownloadTask, "download_task", 4096,
				    &ota_update_info, 2, NULL);
			return;
		}
	}

	setupAccessory();

}

void user_init(void)
{

	uart_set_baud(0, 115200);

	initGPIO();

	greenLedStatus = led_status_init(ledGPIO);
	led_status_set(greenLedStatus, &waitingWifiPattern);

	wifi_config_init("sonoff-s20", NULL, wifiReadyCallback);

	if (button_create(buttonGPIO, 0, 4000, button_callback)) {
		DEBUG("Failed to initialize button.");
	}

}
