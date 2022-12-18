#pragma once
#ifndef SETTINGS_H_
#define SETTINGS_H_

#define VERSION     "0.1.43"

//#define USE_RTC
#define USE_SPIFFS

#if defined(USE_RTC)
#define RTC_REGISTER_ADDRESS            (128 / 4)   // first 128 bytes of UserData ae reserved for OTA (eboot)
#endif

#define ACCESSORY_NAME			        ("ESP8266_OUTLET")
// #define ACCESSORY_SN                    ("SN_1122334455")  //SERIAL_NUMBER
#define ACCESSORY_SN  			        ("SN")  //SERIAL_NUMBER
#define ACCESSORY_MANUFACTURER 	        ("Arduino Homekit")
#define ACCESSORY_MODEL  		        ("ESP8266")

#define ACCESSORY_CHARACTERISTIC_NAME   "Outlet"
#define ACCESSORY_PASSWORD              "111-11-111"
#define ACCESSORY_SETUP_ID              "ABCD"

#define PIN_LEDD    13	//D7    LED
#define PIN_RELAY   14	//D5    RELAY
#define PIN_BUTTON	4	//D2    BUTTON
// #define PIN_LEDD	16	//D0    LED
// #define PIN_RELAY  2	//D4    RELAY
// #define PIN_BUTTON	0	//Dx    BUTTON

#define COMMON_NUMEL(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#endif
