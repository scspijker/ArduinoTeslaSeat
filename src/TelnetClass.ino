#ifndef TELNETCLASS_INO
#define TELNETCLASS_INO

#include <WiFi.h>
#include <Arduino.h>
#include "TeslaSeatEvents.ino"

struct TelnetResult {
  Event event = Event::UNKNOWN;
  int relayNumber = 0;
  bool relayOn = false;
  int direction = 0; // For direction enum value
};

// Lightweight Telnet server for debugging and simple events.
class Telnet
{
private:
      uint16_t _port;
      String _cmdBuffer;
      WiFiServer telnetServer;
      WiFiClient telnetClient;
      TelnetResult repeatResult;
      
      bool authenticated = false;

      void acceptClient() {
            if (!telnetServer.hasClient()) return;

            // Only accept a new client if we don't have one already
            if (!telnetClient || !telnetClient.connected()) {
                  if (telnetClient) telnetClient.stop();
                  telnetClient = telnetServer.available();
                  if (telnetClient && telnetClient.connected()) {
                        telnetClient.println("=== Tesla Seat Console ===");
                        telnetClient.println("Please authenticate...");
                  }
            } else {
                  // If we already have a connected client, refuse the new one
                  WiFiClient newClient = telnetServer.available();
                  if (newClient) newClient.stop();
            }
      }

      TelnetResult readClient() {
            if (!(telnetClient && telnetClient.connected())) return TelnetResult();

            while (telnetClient.available()) {
                  char c = (char)telnetClient.read();
                  if (c == '\r' || c == '\n') {
                        if (_cmdBuffer.length() > 0) {
                              TelnetResult result = processCommand(_cmdBuffer);
                              _cmdBuffer = "";
                              return result;
                        }
                  } else if ((c >= 48 && c <= 57) || // 0-9
                             (c >= 65 && c <= 90) || // A-Z
                             (c >= 97 && c <= 122) || // a-z
                             c == ' ') {
                        _cmdBuffer += c;
                  } else if (c == 8) { // Backspace or DEL
                        if (_cmdBuffer.length() > 0) {
                              _cmdBuffer.remove(_cmdBuffer.length() - 1);
                        }
                  } else if (c == 27) {
                        telnetClient.println("Received escape, disconnecting...");
                        disconnect();
                  }

            }

            return TelnetResult();
      }

      TelnetResult processCommand(String cmd) {
            cmd.trim();
            cmd.toLowerCase();

            Serial.println("TELNET: " + cmd);

            if (!(telnetClient && telnetClient.connected())) return TelnetResult();

            TelnetResult result;

            if (authenticated == false) {
                  if (cmd == String("auth ") + String(TELNET_PASSWORD)) {
                        authenticated = true;
                        telnetClient.println("Authentication successful!");
                        printHelp();
                  } else {
                        telnetClient.println("Please authenticate...");
                  }
                  return result;
            }

            if (cmd == "help" || cmd == "?") {
                  printHelp();
            } else if (cmd == "status") {
                  result.event = Event::STATUS;
            } else if (cmd == "r") {
                  result = repeatResult;
            } else if (cmd.startsWith("relay ")) {
                  // Expect: relay <n> on|off
                  int firstSpace = cmd.indexOf(' ');
                  int secondSpace = cmd.indexOf(' ', firstSpace + 1);
                  if (secondSpace > firstSpace) {
                        int relay = cmd.substring(firstSpace + 1, secondSpace).toInt();
                        String state = cmd.substring(secondSpace + 1);
                        if (relay >= 1 && relay <= 16) {
                              bool on = (state == "on");
                              result.event = Event::RELAY;
                              result.relayNumber = relay;
                              result.relayOn = on;
                        }
                  }
            } else if (cmd.startsWith("seat horizontal ")) {
                  // seat horizontal <dir>
                  if (cmd.length() > 15) {
                        String dirStr = cmd.substring(15);
                        dirStr.trim();
                        if (dirStr == "forward") {
                              result.event = Event::SEAT_HORIZONTAL;
                              result.direction = (int)HorizontalDirection::FORWARD;
                        } else if (dirStr == "back") {
                              result.event = Event::SEAT_HORIZONTAL;
                              result.direction = (int)HorizontalDirection::BACK;
                        }
                  }
            } else if (cmd.startsWith("seat vertical ")) {
                  // seat vertical <dir>
                  if (cmd.length() > 13) {
                        String dirStr = cmd.substring(13);
                        dirStr.trim();
                        if (dirStr == "up") {
                              result.event = Event::SEAT_VERTICAL;
                              result.direction = (int)VerticalDirection::UP;
                        } else if (dirStr == "down") {
                              result.event = Event::SEAT_VERTICAL;
                              result.direction = (int)VerticalDirection::DOWN;
                        }
                  }
            } else if (cmd.startsWith("back angle ")) {
                  // back angle <dir>
                  if (cmd.length() > 10) {
                        String dirStr = cmd.substring(10);
                        dirStr.trim();
                        if (dirStr == "forward") {
                              result.event = Event::BACK_ANGLE;
                              result.direction = (int)HorizontalDirection::FORWARD;
                        } else if (dirStr == "back") {
                              result.event = Event::BACK_ANGLE;
                              result.direction = (int)HorizontalDirection::BACK;
                        }
                  }
            } else if (cmd.startsWith("seat angle ")) {
                  // seat angle <dir>
                  int spacePos = cmd.indexOf(' ', 9);
                  if (cmd.length() > 10) {
                        String dirStr = cmd.substring(10);
                        dirStr.trim();
                        if (dirStr == "up") {
                              result.event = Event::SEAT_ANGLE;
                              result.direction = (int)VerticalDirection::UP;
                        } else if (dirStr == "down") {
                              result.event = Event::SEAT_ANGLE;
                              result.direction = (int)VerticalDirection::DOWN;
                        }
                  }
            } else if (cmd.startsWith("headrest ")) {
                  // headrest <dir>
                  if (cmd.length() > 8) {
                        String dirStr = cmd.substring(8);
                        dirStr.trim();
                        if (dirStr == "up") {
                              result.event = Event::HEADREST;
                              result.direction = (int)VerticalDirection::UP;
                        } else if (dirStr == "down") {
                              result.event = Event::HEADREST;
                              result.direction = (int)VerticalDirection::DOWN;
                        }
                  }
            } else if (cmd.startsWith("lumbar horizontal ")) {
                  // lumbar horizontal <dir>
                  if (cmd.length() > 17) {
                        String dirStr = cmd.substring(17);
                        dirStr.trim();
                        if (dirStr == "forward") {
                              result.event = Event::LUMBAR_HORIZONTAL;
                              result.direction = (int)HorizontalDirection::FORWARD;
                        } else if (dirStr == "back") {
                              result.event = Event::LUMBAR_HORIZONTAL;
                              result.direction = (int)HorizontalDirection::BACK;
                        }
                  }
            } else if (cmd.startsWith("lumbar vertical ")) {
                  // lumbar vertical <dir>
                  if (cmd.length() > 16) {
                        String dirStr = cmd.substring(16);
                        dirStr.trim();
                        if (dirStr == "up") {
                              result.event = Event::LUMBAR_VERTICAL;
                              result.direction = (int)VerticalDirection::UP;
                        } else if (dirStr == "down") {
                              result.event = Event::LUMBAR_VERTICAL;
                              result.direction = (int)VerticalDirection::DOWN;
                        }
                  }
            } else if (cmd == "calib") {
                  result.event = Event::CALIB;
            } else if (cmd == "demo") {
                  result.event = Event::DEMO;
            } else if (cmd == "reboot") {
                  result.event = Event::REBOOT;
            } else {
                  telnetClient.println("Unknown command - type 'help'");
            }

            return result;
      }

      void printHelp() {
            telnetClient.println("");
            telnetClient.println("Available commands");
            telnetClient.println("------------------------------------------------------------------");
            telnetClient.println("help                            -> this list");
            telnetClient.println("status                          -> active relays");
            telnetClient.println("relay <n> on|off                -> turn relay <n> on or off (1-16)");
            telnetClient.println("calib                           -> raw analog values");
            telnetClient.println("reboot                          -> restart ESP32");
            telnetClient.println("seat horizontal <dir>           -> move seat (forward/back)");
            telnetClient.println("seat vertical <dir>             -> move seat up/down");
            telnetClient.println("back angle <dir>                -> change back angle (forward/back)");
            telnetClient.println("seat angle <dir>                -> change seat angle (up/down)");
            telnetClient.println("headrest <dir>                  -> move headrest (up/down)");
            telnetClient.println("lumbar horizontal <dir>         -> move lumbar horizontal (forward/back)");
            telnetClient.println("lumbar vertical <dir>           -> move lumbar vertical (up/down)");
            telnetClient.println("r                               -> repeat last command");
            telnetClient.println("------------------------------------------------------------------");
            telnetClient.println("");
      }

public:
      Telnet(uint16_t port = 23) : _port(port), telnetServer(port) {
      }

      void setup() {
            Serial.println("Starting Telnet server...");
            telnetServer.begin();
            telnetServer.setNoDelay(true);
            Serial.printf("Telnet server running on port %u\n", _port);
      }

      void disconnect() {
            authenticated = false;
            if(telnetClient) {
                  telnetClient.stop();
            }
      }

      TelnetResult loop() {
            if (telnetClient) {
                  // Check for disconnection first
                  if (!telnetClient.connected()) {
                        disconnect();
                  }
            }

            acceptClient();
            TelnetResult result = readClient();

            if (result.event != Event::UNKNOWN) {
                  repeatResult = result;
            }

            return result;
      }

      void println(const String &msg) {
            if (telnetClient && telnetClient.connected()) {
                  telnetClient.println(msg);
            }
      }

      void print(const String &msg) {
            if (telnetClient && telnetClient.connected()) {
                  telnetClient.print(msg);
            }
      }
};

#endif