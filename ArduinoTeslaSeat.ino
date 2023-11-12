#include <RingBuf.h>
#include "set.h"

float measuringVoltage = 5.1675;
int pollInterval = 8;
int actInterval = 8;
int keyboardAccuracy = 25;
int keyboardMiniumValue = 5;

int amountOfKeyboards = 3;
const int keyboardPins[3] = { A2, A1, A0 };
const float keyboardVoltageDividerResistance[3] = { 994.4, 996.2, 994.8 };
RingBuf<long, 10> keyboardBuffers[3] = {
  RingBuf<long, 10>(), RingBuf<long, 10>(), RingBuf<long, 10>()
};

struct KeyboardKey {
  int value;
  String motor;
  String direction;
  int relayPin;
  int secondaryRelayPin;
};

int amountOfKeysPerKeyboard = 4;
KeyboardKey keyboardKeys[4][4] = {
  {{ 658, "Shift", "Forward", 28 }, { 905, "Shift", "Backward", 29 }, { 208, "Tilt", "Up", 25 }, { 424, "Tilt", "Down", 22 }},
  {{ 209, "Lift", "Up", 24 }, { 422, "Lift", "Down", 26, 27 }, { 655, "Recline", "Forward", 27}, { 899, "Recline", "Backward", 26, 24 }},
  {{ 175, "Lumbar Vertical", "Up", 32 }, { 499, "Lumbar Vertical", "Down", 33, 30 }, { 687, "Lumbar Horizontal", "Forward", 30},  { 334, "Lumbar Horizontal", "Backward", 33, 32 }},
  {{ 999, "Headrest", "Up", 31}, { 9999, "Headrest", "Down", 29}, {}, {}}
};

Set activatedRelays;

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  for(int relayPin = 22; relayPin <= 38; relayPin++) {
    digitalWrite(relayPin, HIGH); // Write first to prevent flickering
    pinMode(relayPin, OUTPUT);
  }

  while (!Serial) { delay(10); }
  Serial.println("Setup complete...");
}

int calculateResistanceFor(int keyboard) {
    int analogInputPin = keyboardPins[keyboard];
    int readValue = analogRead(analogInputPin);

    // We can do all this to be fancy, or we can save CPU cycles 🤷

    // // Voltage dividers on analog inputs with known resistor R1
    // float R1 = keyboardVoltageDividerResistance[keyboard];
    // float R1_Voltage = readValue / 1023.0 * measuringVoltage;
    // float R1_Current_mA = R1_Voltage * 1000.0 / R1;
    
    // // Now we can calculate "unknown" R2
    // float R2_Voltage = measuringVoltage - R1_Voltage;
    // int R2 = (int) (R2_Voltage / R1_Current_mA * 1000.0);

    // return R2;

    return readValue;
}

bool keyboardIsNearKey(int keyboardValue, int keyValue) {
  return (keyboardValue - keyboardAccuracy < keyValue) && (keyboardValue + keyboardAccuracy > keyValue);
}

int debounce(int keyboardIndex) {
  RingBuf<long, 10> keyboardBuffer = keyboardBuffers[keyboardIndex];
  if (!keyboardBuffer.isFull()) return -1;

  long max = 0, average = 0, min = 99999;
  for(int i = 0; i < keyboardBuffer.size() ; i++) {
    long value = keyboardBuffer[i];
    if (value > max) max = value;
    if (value > min) min = value;
    average += value;
  }
  average /= keyboardBuffer.size();

  if (max - min > keyboardAccuracy) return -1;

  return average;
}

void writeRelays(Set relayPins, int state) {
    int n = relayPins.first();
    while (n != -1)
    {
      digitalWrite(n, state);
      n = relayPins.next();
    }
}

int l = 0;
unsigned long previousMillis = 0;        // will store last time the loop was called
void loop() {
  if (previousMillis + pollInterval > millis()) return;
  previousMillis = millis();

  // Fast stuff
  bool interaction = false;
  for (int keyboardIndex = 0; keyboardIndex < amountOfKeyboards; keyboardIndex++) {
    int readValue = calculateResistanceFor(keyboardIndex);

    if (readValue > keyboardMiniumValue) {
      keyboardBuffers[keyboardIndex].pushOverwrite(readValue);
      interaction = true;
    } else if (!keyboardBuffers[keyboardIndex].isEmpty()) {
      keyboardBuffers[keyboardIndex].clear();
    }
  }

  // Stop when there is no interaction
  l++;
  if (interaction && l >= actInterval) {
    l = 0;

    Set activateKeysRelay;
    for (int keyboardIndex = 0; keyboardIndex < amountOfKeyboards; keyboardIndex++) {
      int keyboardValue = debounce(keyboardIndex);

      if (keyboardValue == -1) continue;

      // Check keys
      for (int keyIndex = 0; keyIndex < amountOfKeysPerKeyboard; keyIndex++) {
        if (keyboardIsNearKey(keyboardValue, keyboardKeys[keyboardIndex][keyIndex].value)) {
          KeyboardKey keyPressed = keyboardKeys[keyboardIndex][keyIndex];
          // Serial.print(keyPressed.motor);
          // Serial.print(" ");
          // Serial.print(keyPressed.direction);
          // Serial.print(", activate relay ");
          // Serial.println(keyPressed.relayPin);
          activateKeysRelay.add(keyPressed.relayPin);

          if (keyPressed.secondaryRelayPin > 0) {
            // Some DOWN/BACK's are shared. So we switch on
            // the other motor's UP/FORWARD, so for the other motor
            // both leads are positive and no movement will ensue.
            activateKeysRelay.add(keyPressed.secondaryRelayPin);
          }
        }
      }
    }

    if (activatedRelays != activateKeysRelay) {

      Set deactivateRelays = activatedRelays - activateKeysRelay;
      Set activateRelays = activateKeysRelay - activatedRelays;

      if (!deactivateRelays.isEmpty()) {
        writeRelays(deactivateRelays, HIGH);
      }
      
      if (!activateRelays.isEmpty()) {
        writeRelays(activateRelays, LOW);
      }

      activatedRelays = activateKeysRelay;
      if (!activatedRelays.isEmpty()) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.print("Active relays");
        int n = activatedRelays.first();
        while (n != -1)
        {
          Serial.print(", ");
          Serial.print(n);
          n = activatedRelays.next();
        }
        Serial.println(".");
      }
    }
  } else if (!interaction) {
    l = 0;
    if (!activatedRelays.isEmpty()) {
      writeRelays(activatedRelays, HIGH);
      activatedRelays.clear();
      Serial.println("All relays inactive.");
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

