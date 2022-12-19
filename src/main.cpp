#include <Arduino.h>

#include <settings.h>
#include <ota_secret.h>

#include <ESP8266WiFi.h>

#include "WiFiManager.h"

#if defined(USE_SPIFFS)
//#include <FS.h>
#include <LittleFS.h>
#endif

// #include <ArduinoOTA.h>
#include "ESP8266WebServer.h"
#include "ESP8266HTTPUpdateServer.h"

#include <arduino_homekit_server.h>
#include "ButtonDebounce.h"
#include "ButtonHandler.h"

#include <led_timer.h>

ButtonDebounce btn(PIN_BUTTON, INPUT_PULLUP, LOW);
ButtonHandler btnHandler(10000);

void accessory_init();
void switchToggle();
void builtinledSetStatus(bool on);
void homekit_setup();
void homekit_loop();
void my_homekit_report();

bool runOnce = false;
bool switch_power = false;

//const char *ssid = "SilentNight-2G";
//const char *password = "HarisPoteris2012!!";

#define SIMPLE_INFO(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

void IRAM_ATTR btnInterrupt() {
	btn.update();
}

void blink_led(int interval, int count) {
	for (int i = 0; i < count; i++) {
		builtinledSetStatus(true);
		delay(interval);
		builtinledSetStatus(false);
		delay(interval);
	}
}

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t name;
extern "C" homekit_characteristic_t serial_number;
extern "C" homekit_characteristic_t cha_switch_on;
extern "C" homekit_characteristic_t cha_bright;

void handleRoot() {
	String s = "";

	s += 	"<!DOCTYPE HTML><html>" \
	     	"<head>" \
  			"	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">" \
  			"	<style>" \
    		"		html {" \
     		"			font-family: Arial;" \
     		"			display: inline-block;" \
     		"			margin: 0px auto;" \
     		"			text-align: left;" \
    		"		}" \
    		"		h2 { font-size: 2.0rem; }" \
    		"		p { font-size: 1.0rem; }" \
    		"		.units { font-size: 1.2rem; }" \
  			"	</style>" \
			"</head>";
	s +=	"<body>" \
  			"	<h2>Arduino HomeKit Outlet v";
	s +=	VERSION;
	s +=	"</h2>" \
	  		"	<p>" \
    		"		<b>Name:</b>" \
    		"		<span id=\"system_name\">";
	s +=	name.value.string_value;
	s +=	"</span>" \
  			"	</p>" \
  			"	<p>" \
    		"		<b>Current Outlet State:</b>" \
    		"		<span id=\"outlet_state\">";
	s +=	switch_power ? "ON" : "OFF";
	s +=	"</span>" \
  			"	</p>" \
  			"	<p>" \
    		"		<b>S/N:</b>" \
    		"		<span id=\"serial_number\">";
	s +=	serial_number.value.string_value;
	s +=	"</span>" \
  			"	</p>" \
  			"	<p>" \
    		"		<b>MAC:</b>" \
    		"		<span id=\"mac_address\">";
	s +=	String(WiFi.macAddress());
	s +=	"</span>" \
  			"	</p>" \
  			"	<p>" \
    		"		<b>Reset reason:</b>" \
    		"		<span id=\"reset_reason\">";
	s +=	ESP.getResetReason() + " / " + ESP.getResetInfo();
	s +=	"</span>" \
  			"	</p>" \
  			"	<p>&nbsp;</p>" \
  			"	<p>- <strong>Short Press</strong> - Toggle ON/OFF</p>" \
  			"	<p>- <strong>Double Press</strong> - n/a</p>" \
  			"	<p>- <strong>Long Press (&gt;5sec)</strong> - Reset HomeKit pairing and WiFi settings.</p>" \
  			"	<p>- <strong>PowerUp with Button Pressed</strong> - Run WiFi Manager</p>" \
  			"	<p>&nbsp;</p>" \
  			"	<p><a href=\"/update\">Update FW</a></p>" \
			"</body>" \
			"</html>";
	
	httpServer.send(200, "text/html", s);
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void lowCPUspeed() {
	if (system_get_cpu_freq() != SYS_CPU_80MHZ) {
		system_update_cpu_freq(SYS_CPU_80MHZ);
		INFO("Update the CPU to run at 80 MHz");
	}
}

void setup() {
	Serial.begin(115200);
	Serial.setRxBufferSize(32);
	Serial.setDebugOutput(false);

	ledTimerBegin();

	ledTimerSetPattern(blink50);

#if defined(USE_RTC)
	uint32_t rtcMem;
	ESP.rtcUserMemoryRead(RTC_REGISTER_ADDRESS, &rtcMem, sizeof(rtcMem));
	// system_rtc_mem_read(56, &rtcMem, sizeof(rtcMem));
	Serial.print("\nBackup Register Read = ");
	Serial.println(rtcMem);
	switch_power = (rtcMem == 1);
	runOnce = true;
#endif

#if defined(USE_SPIFFS)
	if (LittleFS.begin()) {
		Serial.println("LitteFS begin OK");
	} else {
		Serial.println("LittleFS begin ERROR!");
	}
	File f = LittleFS.open("/state.bkp", "r");
	if (f) {
		Serial.println("File open read OK");
		String state = f.readString(); //read();
		Serial.println(state);

		// set last state here
		switch_power = (state == "ON");
		
		f.close();
	} else {
		Serial.println("File open read ERROR!");
	}
	runOnce = true;
#endif

#if !defined(USE_SPIFFS) && !defined(USE_RTC)
	pinMode(PIN_RELAY, OUTPUT);
	switch_power = (digitalRead(PIN_RELAY) == 1);
	runOnce = true;
#endif

	accessory_init();

	ledTimerSetPattern(blink250);

	btn.setCallback(std::bind(&ButtonHandler::handleChange, &btnHandler,
			std::placeholders::_1));
	btn.setInterrupt(btnInterrupt);
	btnHandler.setIsDownFunction(std::bind(&ButtonDebounce::checkIsDown, &btn));
	btnHandler.setCallback([](button_event e) {
		if (e == BUTTON_EVENT_SINGLECLICK) {
			SIMPLE_INFO("Button Event: SINGLECLICK");
			switchToggle();
		} else if (e == BUTTON_EVENT_DOUBLECLICK) {
			SIMPLE_INFO("Button Event: DOUBLECLICK");
		} else if (e == BUTTON_EVENT_LONGCLICK) {
			SIMPLE_INFO("Button Event: LONGCLICK");
			SIMPLE_INFO("Rebooting...");
			homekit_storage_reset();
			//WiFi.disconnect(true);
			ESP.eraseConfig();
			blink_led(50, 10);
//			digitalWrite(PIN_LED, LOW);
			ESP.restart(); // or system_restart();
		}
	});

	WiFiManager wifiManager;
	wifiManager.setSaveConfigCallback(saveConfigCallback);
	wifiManager.setTimeout(300);
  	wifiManager.setConfigPortalTimeout(300);
	wifiManager.setShowPassword(true);

	//WiFi.mode(WIFI_STA);
	//WiFi.persistent(false);
	//WiFi.disconnect(false);
	//WiFi.setAutoReconnect(true);
	//WiFi.begin(ssid, password);

	pinMode(PIN_BUTTON, INPUT_PULLUP);
	if (LOW == digitalRead(PIN_BUTTON)) {
		wifiManager.startConfigPortal();
	}
	wifiManager.autoConnect();

	wifiManager.stopConfigPortal();
	WiFi.mode(WIFI_STA);

	WiFi.printDiag(Serial);

	SIMPLE_INFO("");
	SIMPLE_INFO("SketchSize: %d", ESP.getSketchSize());
	SIMPLE_INFO("FreeSketchSpace: %d", ESP.getFreeSketchSpace());
	SIMPLE_INFO("FlashChipSize: %d", ESP.getFlashChipSize());
	SIMPLE_INFO("FlashChipRealSize: %d", ESP.getFlashChipRealSize());
	SIMPLE_INFO("FlashChipSpeed: %d", ESP.getFlashChipSpeed());
	SIMPLE_INFO("SdkVersion: %s", ESP.getSdkVersion());
	SIMPLE_INFO("FullVersion: %s", ESP.getFullVersion().c_str());
	SIMPLE_INFO("CpuFreq: %dMHz", ESP.getCpuFreqMHz());
	SIMPLE_INFO("FreeHeap: %d", ESP.getFreeHeap());
	SIMPLE_INFO("ResetInfo: %s", ESP.getResetInfo().c_str());
	SIMPLE_INFO("ResetReason: %s", ESP.getResetReason().c_str());
	INFO_HEAP();

	// Route for root / web page
	httpServer.on("/", HTTP_GET, handleRoot);

	httpUpdater.setup(&httpServer, OTA_USER, OTA_PASSWORD);
	httpServer.begin();

	ledTimerSetPattern(blink500);

	homekit_setup();

	INFO_HEAP();

	// lowCPUspeed();
}

void loop() {
	httpServer.handleClient();
	homekit_loop();
}

void builtinledSetStatus(bool on) {
	ledTimerSetAccessoryState(on);
}

void saveCurrentState() {
#if defined(USE_SPIFFS)
	File f = LittleFS.open("/state.bkp", "w");
	if (f) {
		Serial.println("File open write OK");
		f.write(switch_power ? "ON" : "OFF");
		f.close();
	} else {
		Serial.println("File open write ERROR!");
	}
#endif

#if defined(USE_RTC)
	uint32_t rtcMem = (switch_power ? 1 : 0);
	ESP.rtcUserMemoryWrite(RTC_REGISTER_ADDRESS, &rtcMem, sizeof(rtcMem));
	Serial.print("Backup Register Write = ");
	Serial.println(rtcMem);
#endif
}

//==============================
// Homekit setup and loop
//==============================

void switchToggle() {
	switch_power = !switch_power;
	saveCurrentState();
	builtinledSetStatus(!switch_power);
	// digitalWrite(PIN_RELAY, switch_power ? HIGH : LOW);
	cha_switch_on.value.bool_value = switch_power;
	homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

void accessory_init() {
	Serial.println("Init accessory...");
	// digitalWrite(PIN_RELAY, switch_power ? HIGH : LOW);
	pinMode(PIN_RELAY, OUTPUT);
	builtinledSetStatus(!switch_power);
}

void cha_switch_on_setter(const homekit_value_t value) {
	printf("setter\n");
// if (value.format == homekit_format_bool) {
	bool on = value.bool_value;
	switch_power = on;
	saveCurrentState();
	// cha_switch_on.value.bool_value = on;	//sync the value
	Serial.printf("Switch: %s\n", on ? "ON" : "OFF");
	builtinledSetStatus(!switch_power);
	// digitalWrite(PIN_RELAY, switch_power ? HIGH : LOW);
// }
}

homekit_value_t cha_switch_on_getter() {
	printf("getter\n");
	return HOMEKIT_BOOL_CPP(switch_power);
}

void cha_brightness_setter(const homekit_value_t value) {
	printf("brightness setter\n");
	int brightness = value.int_value;
	cha_bright.value.int_value = brightness;

	printf("Brightness: %d\n", brightness);

	//current_brightness = brightness;

	//updateColor();

	int pwm_brightness = (int)map(brightness, 0, 100, 0, 1024);
	printf("PWM: %d\n", pwm_brightness);

	analogWrite(PIN_RELAY, pwm_brightness);
}

void homekit_setup() {
	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(mac);

	int name_len = snprintf(NULL, 0, "%s_%02X%02X%02X",
			name.value.string_value, mac[3], mac[4], mac[5]);
	char *name_value = (char*) malloc(name_len + 1);
	snprintf(name_value, name_len + 1, "%s_%02X%02X%02X",
			name.value.string_value, mac[3], mac[4], mac[5]);
	name.value = HOMEKIT_STRING_CPP(name_value);

	int serial_number_len = snprintf(NULL, 0, "%s_%02X%02X%02X",
			serial_number.value.string_value, mac[3], mac[4], mac[5]);
	char *serial_number_value = (char*) malloc(serial_number_len + 1);
	snprintf(serial_number_value, serial_number_len + 1, "%s_%02X%02X%02X",
			serial_number.value.string_value, mac[3], mac[4], mac[5]);
	serial_number.value = HOMEKIT_STRING_CPP(serial_number_value);

	cha_switch_on.setter = cha_switch_on_setter;
	cha_switch_on.getter = cha_switch_on_getter;

	cha_bright.setter = cha_brightness_setter;

	// httpUpdater.setup(&httpServer, OTA_USER, serial_number_value);
	// httpServer.begin();

	arduino_homekit_setup(&config);
}

void homekit_loop() {
	if (runOnce) {
		runOnce = false;
#if defined(USE_SPIFFS) || defined(USE_RTC)
		cha_switch_on.value.bool_value = switch_power;
		homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
#endif
	}
	btnHandler.loop();
	arduino_homekit_loop();
	static uint32_t next_heap_millis = 0;
	uint32_t time = millis();
	if (time > next_heap_millis) {
		SIMPLE_INFO("heap: %d, sockets: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
		next_heap_millis = time + 5000;
	}

	if (WiFi.status() == WL_CONNECTED) {
		ledTimerSetPattern(blinkFullLoad);
	} else {
		ledTimerSetPattern(blink250);
	}
}
