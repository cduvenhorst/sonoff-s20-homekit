#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "device_settings.h"

#include "hap_accessory_information_service.h"


#if HAS_OUTLET_SERVICES > 0
#include "hap_outlet_services.h"
#endif

// Services of the accessory without sensor services
homekit_service_t *accessoryServices[] = {
	&accessoryInformationService,
#if HAS_OUTLET_SERVICES >= 1
	&outlet_1_Service,
#endif

	NULL
};

// Accessory definition
homekit_accessory_t accessoryS20 = {
	.id = 1,
	.category = homekit_accessory_category_outlet,
	.config_number = 1,
	.services = accessoryServices
};

// Array of managed accessories
homekit_accessory_t *accessories[] = {
	&accessoryS20,
	NULL
};
