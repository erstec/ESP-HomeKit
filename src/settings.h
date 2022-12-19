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

#define ACCESSORY_CHARACTERISTIC_NAME   "LedStrip"
#define ACCESSORY_PASSWORD              "111-11-111"
#define ACCESSORY_SETUP_ID              "ABCD"

#define PIN_LED     2

#define PIN_SW1     9
#define PIN_SW2     10
#define PIN_SW3     13

#define PIN_OUT1    4
#define PIN_OUT2    12
#define PIN_OUT3    14

#define PIN_LEDD    PIN_LED
#define PIN_OUT_RED PIN_OUT1
#define PIN_OUT_GRN PIN_OUT2
#define PIN_OUT_BLU PIN_OUT3
#define PIN_BUTTON	PIN_SW3

#define COMMON_NUMEL(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#endif
