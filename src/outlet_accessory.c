/*
 * simple_led_accessory.c
 * Define the accessory in pure C language using the Macro in characteristics.h
 *
 *  Created on: 2020-02-08
 *      Author: Mixiaoxiao (Wang Bin)
 */

#include <Arduino.h>
#include <settings.h>
#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <stdio.h>
#include <port.h>

//const char * buildTime = __DATE__ " " __TIME__ " GMT";

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, ACCESSORY_NAME);
homekit_characteristic_t serial_number = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, ACCESSORY_SN);
homekit_characteristic_t cha_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_bright = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 50);
homekit_characteristic_t cha_sat = HOMEKIT_CHARACTERISTIC_(SATURATION, (float) 0);
homekit_characteristic_t cha_hue = HOMEKIT_CHARACTERISTIC_(HUE, (float) 180);

void accessory_identify(homekit_value_t _value) {
	printf("accessory identify\n");
	for (int j = 0; j < 10; j++) {
		// digitalWrite(PIN_LED, LOW);
		delay(50);
		// digitalWrite(PIN_LED, HIGH);
		delay(50);
	}
}

homekit_accessory_t *accessories[] = {
	HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
		HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
			&name,
			HOMEKIT_CHARACTERISTIC(MANUFACTURER, ACCESSORY_MANUFACTURER),
			&serial_number,
			HOMEKIT_CHARACTERISTIC(MODEL, ACCESSORY_MODEL),
			HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, VERSION),
			HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
			NULL
		}),
		HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
			HOMEKIT_CHARACTERISTIC(NAME, ACCESSORY_CHARACTERISTIC_NAME),
			&cha_switch_on,
			&cha_bright,
			&cha_sat,
			&cha_hue,
			//HOMEKIT_CHARACTERISTIC(OUTLET_IN_USE, true),
			NULL
		}),
		NULL
	}),
	NULL
};

homekit_server_config_t config = {
		.accessories = accessories,
		.password = ACCESSORY_PASSWORD,
		//.on_event = on_homekit_event,
		.setupId = ACCESSORY_SETUP_ID
};
