#ifndef LEDTIMER_H
#define LEDTIMER_H

typedef enum {
    blink50,
    blink250,
    blink500,
    blinkFullLoad,
} teLEDpatternNames;

typedef struct {
    teLEDpatternNames name;
    unsigned short steps[2];
} tsLEDpattern;

void ledTimerBegin();
void ledTimerSetPattern(teLEDpatternNames patternName);
void ledTimerSetAccessoryState(bool state);

#endif
