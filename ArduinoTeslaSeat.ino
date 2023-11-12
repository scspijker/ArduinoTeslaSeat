#include <RingBuf.h>

struct KeyboardKey {
  int resistance;
  String motor;
  String direction;
};

float measuringVoltage = 5.1675;
int keyboardAccuracy = 25;
int keyboardMiniumValue = 10;

int amountOfKeyboards = 3;
const int keyboardPins[3] = { A2, A1, A0 };
const float keyboardVoltageDividerResistance[3] = { 994.4, 996.2, 994.8 };
RingBuf<long, 10> keyboardBuffers[3] = {
  RingBuf<long, 10>(), RingBuf<long, 10>(), RingBuf<long, 10>()
};

int amountOfKeysPerKeyboard = 4;
KeyboardKey keyboardKeys[3][4] = {
  {{ 550, "Shift", "Forward" }, { 140, "Shift", "Backward" }, { 3896, "Tilt", "Up"}, { 1400, "Tilt", "Down"}},
  {{ 3880, "Lift", "Up" }, { 1425, "Lift", "Down" }, { 580, "Recline", "Forward"}, { 142, "Recline", "Backward"}},
  {{ 4820, "Lumbar Vertical", "Up" }, { 1055, "Lumbar Vertical", "Down" }, { 522, "Lumbar Horizontal", "Forward"},  { 2055, "Lumbar Horizontal", "Backward"}}
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) { delay(10); }

  Serial.println("Setup complete...");
}

int calculateResistanceFor(int keyboard) {
    int analogInputPin = keyboardPins[keyboard];
    int readValue = analogRead(analogInputPin);

    // Voltage dividers on analog inputs with known resistor R1
    float R1 = keyboardVoltageDividerResistance[keyboard];
    float R1_Voltage = readValue / 1023.0 * measuringVoltage;
    float R1_Current_mA = R1_Voltage * 1000.0 / R1;
    
    // Now we can calculate "unknown" R2
    float R2_Voltage = measuringVoltage - R1_Voltage;
    int R2 = (int) (R2_Voltage / R1_Current_mA * 1000.0);

    return R2;
}

bool keyboardIsNearKey(int keyboardResistance, int keyResistance) {
  return (keyboardResistance - 100 < keyResistance) && (keyboardResistance + 100 > keyResistance);
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

int l = 0;
void loop() {

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
  l = interaction ? l + 1 : 0;
  if (l == 10) {
    l = 0;

    for (int keyboardIndex = 0; keyboardIndex < amountOfKeyboards; keyboardIndex++) {
      int keyboardValue = debounce(keyboardIndex);

      if (keyboardValue == -1) continue;

      // Check keys
      for (int keyIndex = 0; keyIndex < amountOfKeysPerKeyboard; keyIndex++) {
        if (keyboardIsNearKey(keyboardValue, keyboardKeys[keyboardIndex][keyIndex].resistance)) {
          KeyboardKey keyPressed = keyboardKeys[keyboardIndex][keyIndex];
          Serial.print(keyPressed.motor);
          Serial.print(" ");
          Serial.println(keyPressed.direction);
        }
      }
    }
  }

  delay(10);
}

