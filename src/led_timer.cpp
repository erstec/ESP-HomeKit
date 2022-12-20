#include <led_timer.h>
#include <Arduino.h>
#include <settings.h>
#include "ESP8266TimerInterrupt.h"

tsLEDpattern ledPatterns[] = {
    { blink50,      { 50,   50,     }, },
    { blink250,     { 250,  250,    }, },
    { blink500,     { 500,  500,    }, },
    { blinkFullLoad,{ 1000, 1000,   }, },
};

static tsLEDpattern currentPattern;
volatile uint8_t currentPatternIdx;
volatile uint32_t currentPatternTime;

volatile bool statusLed = false;

static bool accessoryState = false;

#define TIMER_INTERVAL_MS       50

// Init ESP8266 timer 0
ESP8266Timer ITimer;

//=======================================================================
void ICACHE_RAM_ATTR TimerHandler()
{
    if (currentPattern.name == blinkFullLoad) {
        digitalWrite(PIN_LEDD, !accessoryState);
        return;
    }

    if (currentPatternIdx == 0) statusLed = 0;  // always start from ON

    if (currentPatternTime >= currentPattern.steps[currentPatternIdx]) {
        currentPatternTime = 0;

        currentPatternIdx++;
        if (currentPatternIdx >= COMMON_NUMEL(currentPattern.steps)) currentPatternIdx = 0;
        
        statusLed = !statusLed;
        digitalWrite(PIN_LEDD, statusLed);
    }

    currentPatternTime += TIMER_INTERVAL_MS;
}

void ledTimerBegin() {
    pinMode(PIN_LEDD, OUTPUT);
    // Interval in microsecs
    ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 1000, TimerHandler);
}

void ledTimerSetPattern(teLEDpatternNames patternName) {
    for (uint8_t i = 0; i < COMMON_NUMEL(ledPatterns); i++) {
        if (ledPatterns[i].name == patternName) {
            currentPattern = ledPatterns[i];
            currentPatternIdx = 0;
            currentPatternTime = 0;
        }
    }
}

void ledTimerSetAccessoryState(bool state) {
    accessoryState = state;
}
