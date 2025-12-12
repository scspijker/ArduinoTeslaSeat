// TeslaSeatControls.ino

#ifndef TESLA_SEAT_CONTROLS_H
#define TESLA_SEAT_CONTROLS_H

#include "TeslaSeatEvents.ino"  // For Event and ButtonEvent enums

#define BUFFER_SIZE 10
#define NUM_PINS 3

// Returned when a control change (press/release) is detected
struct ControlEvent {
  Event event;
  int direction;
  ButtonEvent state;
  ControlEvent() : event(Event::UNKNOWN), direction(0), state(ButtonEvent::RELEASED) {}
  ControlEvent(Event e, int d, ButtonEvent s) : event(e), direction(d), state(s) {}
};

class TeslaSeatControls {
private:
  const int keyboardPins[NUM_PINS] = {34, 36, 39};
  int pollInterval = 8;  // ms
  int actInterval = 8;   // ms
  int keyboardAccuracy = 100;
  int keyboardMiniumValue = 5;

  long keyboardBuffers[NUM_PINS][BUFFER_SIZE];
  int bufferIndex[NUM_PINS];
  int bufferCount[NUM_PINS];

  struct KeyMapping {
    int pinIndex;
    int value;
    Event event;
    int direction;  // 1 for positive (forward/up), -1 for negative (backward/down)
  };

  KeyMapping mappings[12] = {
    // Pin 39
    {2, 1250, Event::SEAT_HORIZONTAL, (int)HorizontalDirection::FORWARD},
    {2, 300, Event::SEAT_HORIZONTAL, (int)HorizontalDirection::BACK},
    {2, 3000, Event::SEAT_ANGLE, (int)VerticalDirection::UP},
    {2, 2150, Event::SEAT_ANGLE, (int)VerticalDirection::DOWN},
    // Pin 36
    {1, 3000, Event::SEAT_VERTICAL, (int)VerticalDirection::UP},
    {1, 2200, Event::SEAT_VERTICAL, (int)VerticalDirection::DOWN},
    {1, 1250, Event::BACK_ANGLE, (int)HorizontalDirection::FORWARD},
    {1, 300, Event::BACK_ANGLE, (int)HorizontalDirection::BACK},
    // Pin 34
    {0, 3185, Event::LUMBAR_VERTICAL, (int)VerticalDirection::UP},
    {0, 1875, Event::LUMBAR_VERTICAL, (int)VerticalDirection::DOWN},
    {0, 1125, Event::LUMBAR_HORIZONTAL, (int)HorizontalDirection::FORWARD},
    {0, 2500, Event::LUMBAR_HORIZONTAL, (int)HorizontalDirection::BACK}
  };

  // Track last pressed per pin
  KeyMapping* lastPressed[NUM_PINS];

  unsigned long lastPollTime = 0;
  unsigned long lastActTime = 0;

  long getAverage(int pin) {
    long sum = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      int reading = keyboardBuffers[pin][i];
      if(reading == 0) return 0; // Incomplete buffer)
      sum += reading;
    }
    return sum / BUFFER_SIZE;
  }

  KeyMapping* findMapping(int pin, int avg) {
    KeyMapping* best = nullptr;
    int minDiff = keyboardAccuracy + 1;
    for (int i = 0; i < 12; i++) {  // Assuming 12 mappings
      if (mappings[i].pinIndex == pin) {
        int diff = abs(avg - mappings[i].value);
        if (diff < minDiff) {
          minDiff = diff;
          best = &mappings[i];
        }
      }
    }
    if (minDiff <= keyboardAccuracy && avg > keyboardMiniumValue) {
      return best;
    }
    return nullptr;
  }

public:
  TeslaSeatControls() {
    for (int p = 0; p < NUM_PINS; p++) {
      bufferIndex[p] = 0;
      bufferCount[p] = 0;
      lastPressed[p] = nullptr;
      for (int i = 0; i < BUFFER_SIZE; i++) {
        keyboardBuffers[p][i] = 0;
      }
    }
  }

  void begin() {
    for (int p = 0; p < NUM_PINS; p++) {
      pinMode(keyboardPins[p], INPUT);
    }
  }

  ControlEvent loop() {
    unsigned long now = millis();

    // Poll readings
    if (now - lastPollTime >= pollInterval) {
      lastPollTime = now;
      for (int p = 0; p < NUM_PINS; p++) {
        int reading = analogRead(keyboardPins[p]);
        if (reading < 50 || reading > 4000) reading = 0; // Discard out-of-range readings
        keyboardBuffers[p][bufferIndex[p]] = reading;
        bufferIndex[p] = (bufferIndex[p] + 1) % BUFFER_SIZE;
        if (bufferCount[p] < BUFFER_SIZE) bufferCount[p]++;
      }
    }

    // Act on averages
    if (now - lastActTime >= actInterval) {
      lastActTime = now;
      for (int p = 0; p < NUM_PINS; p++) {
        if (bufferCount[p] == 0) continue;
        long avg = 0;
        for (int i = 0; i < bufferCount[p]; i++) {
          avg += keyboardBuffers[p][i];
        }
        avg /= bufferCount[p];

        KeyMapping* current = findMapping(p, (int)avg);

        if (current != lastPressed[p]) {
          // If a previous mapping was pressed, return its RELEASE first
          if (lastPressed[p] != nullptr) {
            KeyMapping* released = lastPressed[p];
            lastPressed[p] = current; // update state
            return ControlEvent(released->event, released->direction, ButtonEvent::RELEASED);
          }
          // Otherwise, if a new mapping is pressed, return its PRESSED
          if (current != nullptr) {
            lastPressed[p] = current;
            return ControlEvent(current->event, current->direction, ButtonEvent::PRESSED);
          }
          lastPressed[p] = current;
        }
      }
    }

    return ControlEvent();
  }
};

#endif
