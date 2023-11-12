struct KeyboardKey {
  int resistance;
  String motor;
  String direction;
};

float measuringVoltage = 5.2071;

int amountOfKeyboards = 3;
const int keyboardPins[3] = { A0, A1, A2 };
int keyboardValues[3] { 0, 0, 0 };

int amountOfKeysPerKeyboard = 4;
KeyboardKey keyboardKeys[3][4] = {
  {{ 546, "Shift", "Forward" }, { 141, "Shift", "Backward" }, { 3893, "Tilt", "Up"}, { 1395, "Tilt", "Down"}},
  {{ 4810, "Lift", "Up" }, { 1043, "Lift", "Down" }, { 495, "Recline", "Forward"}, { 2060, "Recline", "Backward"}},
  {{ 4810, "Lumbar Vertical", "Up" }, { 1043, "Lumbar Vertical", "Down" }, { 495, "Lumbar Horizontal", "Forward"},  { 2060, "Lumbar Horizontal", "Backward"}}
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) { delay(10); }

  Serial.println("Setup complete...");
}

int calculateResistanceFor(int analogInputPin) {
    int readValue = analogRead(analogInputPin);
    // Voltage dividers on analog inputs with known resistor R1
    float R1 = 994.0;
    
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

int i = 0;
void loop() {

  // Fast stuff
  bool interaction = false;
  for (int keyboardIndex = 0; keyboardIndex < amountOfKeyboards; keyboardIndex++) {
    keyboardValues[keyboardIndex] = (keyboardValues[keyboardIndex] + calculateResistanceFor(keyboardPins[keyboardIndex])) / 2;
    interaction |= keyboardValues[keyboardIndex] > 10;
  }

  // Debounce
  i = interaction ? i + 1 : 0;
  if (i == 10) {
    i = 0;

    Serial.println();
    for (int keyboardIndex = 0; keyboardIndex < amountOfKeyboards; keyboardIndex++) {
      int keyboardValue = keyboardValues[keyboardIndex];

      // Check keys
      for (int keyIndex = 0; keyIndex < amountOfKeysPerKeyboard; keyIndex++) {
        if (keyboardIsNearKey(keyboardValues[keyboardIndex], keyboardKeys[keyboardIndex][keyIndex].resistance)) {
          KeyboardKey keyPressed = keyboardKeys[keyboardIndex][keyIndex];
          Serial.print(keyPressed.motor);
          Serial.println(keyPressed.direction);
        }
      }
    }
  }

  delay(10);
}

