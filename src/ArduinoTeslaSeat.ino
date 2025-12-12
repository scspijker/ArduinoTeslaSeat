#include <WiFi.h>
#include "ESP32RelayClass.ino"
#include "TelnetClass.ino"
#include "TeslaSeatController.ino"
#include "TeslaSeatControls.ino"

// Create relay board instance with pin configuration
// Pin assignments: latchPin=12, clockPin=13, dataPin=14, oePin=5
RelayBoard relayBoard(12, 13, 14, 5);

// Loop speed
const unsigned long loopDelayMs = 2;

// Setup Telnet instance
Telnet telnet(23);

// Setup TeslaSeatController instance
TeslaSeatController seatController(relayBoard);

// Setup TeslaSeatControls instance
TeslaSeatControls seatControls;

// WiFi credentials from build flags in platformio.ini
const char* wifiSsid = WIFI_SSID;
const char* wifiPassword = WIFI_PASSWORD;
unsigned long lastActivity = millis();
unsigned long wifiTimeout = 1000 * 60 * 15; // 15 minutes

void setup() {
  Serial.begin(115200);
  logln("Serial session started!");

  log("Setting up relayboard... ");
  relayBoard.begin();
  logln("done! ");

  startWifi(true);

  log("Setting up seat controls... ");
  seatControls.begin();
  logln("done!");

  logln("Startup sequence complete!");
}

void startWifi(boolean firstTime) {
  if (wifiSsid != nullptr && strlen(wifiSsid) > 1 && wifiPassword != nullptr && strlen(wifiPassword) > 1) {
    logln("Setting up WiFi... ");
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid, wifiPassword);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      logln("connection failed!");
    } else {
      logf("Connected with IP: %s\n", WiFi.localIP().toString().c_str());
      telnet.setup();
    }
  } else if (firstTime) {
    logln("WiFi credentials not set, skipping WiFi and Telnet setup.");
  }
}

void stopWifi() {
  logln("Stopping WiFi to save power...");
  telnet.disconnect();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  unsigned long now = millis();

  // Slow 1 second loop with modulo
  if (now % 5000 < loopDelayMs) {
    if (WiFi.status() == WL_CONNECTED && now - lastActivity >= wifiTimeout) {
      stopWifi();
    } else if (WiFi.status() != WL_CONNECTED && now - lastActivity < wifiTimeout) {
      startWifi(false);
    }
  }

  // Medium 100ms loop
  if (now % 100 < loopDelayMs) {
    handleTelnet();
  }

  // Fast loop
  handleSeatControls();
  delay(loopDelayMs);
}

void handleSeatControls() {
  ControlEvent controlEvent = seatControls.loop();
  if (controlEvent.event != Event::UNKNOWN) {
    lastActivity = millis();
    handleSeatCommand(controlEvent.event, controlEvent.direction, controlEvent.state);
  }
}

void handleTelnet() {
  static unsigned long seatCommandTime = 0;
  static Event pendingCommand = Event::UNKNOWN;
  static int pendingDirection = 0;

  TelnetResult lastResult = telnet.loop();
  if (lastResult.event != Event::UNKNOWN) {

    lastActivity = millis();

    if (pendingCommand != Event::UNKNOWN) {
      // If a command is already pending, release it first
      handleSeatCommand(pendingCommand, pendingDirection, ButtonEvent::RELEASED);
      pendingCommand = Event::UNKNOWN;
    }

    Serial.println("Received Telnet command: " + String((int)lastResult.event));
    switch(lastResult.event) {
      case Event::SEAT_HORIZONTAL:
      case Event::SEAT_VERTICAL:
      case Event::BACK_ANGLE:
      case Event::SEAT_ANGLE:
      case Event::HEADREST:
      case Event::LUMBAR_HORIZONTAL:
      case Event::LUMBAR_VERTICAL:
        // Start movement (PRESSED)
        pendingCommand = lastResult.event;
        pendingDirection = lastResult.direction;
        handleSeatCommand(pendingCommand, pendingDirection, ButtonEvent::PRESSED);
        seatCommandTime = millis();
        break;
      case Event::RELAY:  setRelay(lastResult.relayNumber, lastResult.relayOn); break;
      case Event::CALIB:  calibrate(); break;
      case Event::STATUS: status(); break;
      case Event::DEMO:   demoLoop(); break;
      case Event::REBOOT: reboot(); break;
      default:                    break;
    }
  }

    // Auto-release after 500ms
  if (pendingCommand != Event::UNKNOWN && (millis() - seatCommandTime > 500)) {
    handleSeatCommand(pendingCommand, pendingDirection, ButtonEvent::RELEASED);
    pendingCommand = Event::UNKNOWN;
  }
}

void handleSeatCommand(Event cmd, int direction, ButtonEvent event) {
  switch(cmd) {
    case Event::SEAT_HORIZONTAL:
      seatController.moveSeat(HorizontalDirection(direction), event);
      break;
    case Event::SEAT_VERTICAL:
      seatController.moveSeat(VerticalDirection(direction), event);
      break;
    case Event::BACK_ANGLE:
      seatController.changeSeatBackAngle(HorizontalDirection(direction), event);
      break;
    case Event::SEAT_ANGLE:
      seatController.changeSeatAngle(VerticalDirection(direction), event);
      break;
    case Event::HEADREST:
      seatController.moveHeadRest(VerticalDirection(direction), event);
      break;
    case Event::LUMBAR_HORIZONTAL:
      seatController.moveLumbar(HorizontalDirection(direction), event);
      break;
    case Event::LUMBAR_VERTICAL:
      seatController.moveLumbar(VerticalDirection(direction), event);
      break;
    default:
      break;
  }
}

/*
 * Command handling
 */

void setRelay(int relayNum, bool state) {
  relayBoard.setRelay(relayNum, state);
  logln(String("Relay ") + String(relayNum) + String(" turned ") + (state ? "ON" : "OFF"));
}

void calibrate() {
  logln("Starting calibration sequence... ");
  // Loop through analog inputs, fetch their values, and log them to telnet
  for (int pin = 32; pin <= 39; pin++) {
    int rawValue = analogRead(pin);
    logln(String("Analog pin ") + String(pin) + String(": ") + String(rawValue));
  }
  logln("Done.");
}

void status() {
  log("Relays currently on: ");
  for (int i = 1; i <= 16; i++) {
    if (relayBoard.getRelayState(i)) {
      log(String(i));
    } else {
      log(" ");
    }
  }
  logln("");
}

void demoLoop() {
  // Turn on relays one by one (cascading effect)
  log("Starting demo loop...  ");
  for (int i = 1; i <= 16; i++) {
        if (i == 1) { relayBoard.setRelay(16, false); } else { relayBoard.setRelay(i-1, false); }
        relayBoard.setRelay(i, true);
        

        if (i > 10) log("\b");
        logf("\b%d", i);
        delay(250);
  }
  relayBoard.setRelay(16, false);
  logln("\b\bDone.");
}

void reboot() {
  logln("Rebooting system...");
  telnet.disconnect();
  delay(500);
  ESP.restart();
}

/*
 * Logging functions - log to both Serial and Telnet
 */
void logln(const String &msg) {
  Serial.println(msg);
  telnet.println(msg);
}

void log(const String &msg) {
  Serial.print(msg);
  telnet.print(msg);
}

void logf(const char *format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Serial.print(buffer);
  telnet.print(String(buffer));
}