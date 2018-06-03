
// Accessory Information Service Characteristics
homekit_characteristic_t accessoryInformationNameCharacteristic = {
	HOMEKIT_DECLARE_CHARACTERISTIC_NAME(DEVICE_NAME " " DEVICE_MODEL)
};

homekit_characteristic_t accessoryInformationSerialCharacteristic = {
	HOMEKIT_DECLARE_CHARACTERISTIC_SERIAL_NUMBER(DEVICE_SERIAL)
};

homekit_characteristic_t identifyCharacteristic = {
	HOMEKIT_DECLARE_CHARACTERISTIC_IDENTIFY(NULL)
};

/*
Required Characteristics for accessories:
 - NAME
 - MANUFACTURER
 - MODEL
 - SERIAL_NUMBER
 - FIRMWARE_REVISION
 - IDENTIFY
*/
homekit_characteristic_t *accessoryInformationCharacterisrics[] = {
	&accessoryInformationNameCharacteristic,
	HOMEKIT_CHARACTERISTIC(MANUFACTURER, DEVICE_MANUFACTURER),
	HOMEKIT_CHARACTERISTIC(MODEL, DEVICE_MODEL),
	&accessoryInformationSerialCharacteristic,
	HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, FW_VERSION),
	&identifyCharacteristic,
	NULL
};

// Accessory Information Service
homekit_service_t accessoryInformationService = {
	.type = HOMEKIT_SERVICE_ACCESSORY_INFORMATION,
	.characteristics = accessoryInformationCharacterisrics
};
