#if HAS_OUTLET_SERVICES >= 1

/*
Outlet 1 service
Outlet Service Characteristics for outlet one.
*/

homekit_characteristic_t outlet_1_CharacteristicName =
    { HOMEKIT_DECLARE_CHARACTERISTIC_NAME("Outlet 1") };

homekit_characteristic_t outlet_1_CharacteristicOn =
    { HOMEKIT_DECLARE_CHARACTERISTIC_ON(false) };

homekit_characteristic_t outlet_1_CharacteristicOutletInUse =
    { HOMEKIT_DECLARE_CHARACTERISTIC_OUTLET_IN_USE(false) };

// Array of all characteristics
homekit_characteristic_t *outlet_1_ServiceCharacterisrics[] = {
	&outlet_1_CharacteristicName,
	&outlet_1_CharacteristicOn,
	&outlet_1_CharacteristicOutletInUse,
	NULL
};

// Outlet service definition
homekit_service_t outlet_1_Service = {
	.type = HOMEKIT_SERVICE_OUTLET,
	.primary = true,
	.characteristics = outlet_1_ServiceCharacterisrics
};

// HAS_OUTLET_SERVICES
#endif
