#ifndef ESP32RELAYCLASS_INO
#define ESP32RELAYCLASS_INO

// RelayBoard class to manage 16 relays via 74HC595 shift registers
// Example board at https://www.amazon.co.uk/gp/product/B0DHF121MQ

class RelayBoard
{
private:
      int latchPin;
      int clockPin;
      int dataPin;
      int oePin;
      uint16_t relayState; // Stores the current state of all 16 relays

      const int MAX_ACTIVE_RELAYS = 16;

      // Update the shift registers with current relay state
      void updateRelays()
      {
            digitalWrite(latchPin, LOW);
            shiftOut(dataPin, clockPin, MSBFIRST, (relayState >> 8));
            shiftOut(dataPin, clockPin, MSBFIRST, relayState);
            digitalWrite(latchPin, HIGH);
      }

public:
      // Constructor
      RelayBoard(int latch, int clock, int data, int oe)
          : latchPin(latch), clockPin(clock), dataPin(data), oePin(oe), relayState(0)
      {     
      }

      // Initialize the relay board - sets up pins and turns all relays off
      void begin()
      {
            pinMode(latchPin, OUTPUT);
            pinMode(dataPin, OUTPUT);
            pinMode(clockPin, OUTPUT);
            pinMode(oePin, OUTPUT);

            relayState = 0;
            updateRelays();
      }

      // Set individual relay state (relay number 1-16, state true=ON/false=OFF)
      void setRelay(int relayNum, bool state)
      {
            if (relayNum < 1 || relayNum > 16)
                  return;

            int bitPos = relayNum - 1;

            if (state)
            {
                  int count = 0;
                  for (int i = 1; i <= 16; i++) if (getRelayState(i)) count++;
                  if (count >= MAX_ACTIVE_RELAYS) {
                      return;
                  }
                  relayState |= (1 << bitPos);
            }
            else
            {
                  relayState &= ~(1 << bitPos);
            }

            updateRelays();
      }

      void allOn()
      {
            relayState = 0xFFFF;
            updateRelays();
      }

      void allOff()
      {
            relayState = 0x0000;
            updateRelays();
      }

      bool getRelayState(int relayNum)
      {
            if (relayNum < 1 || relayNum > 16)
                  return false;
            int bitPos = relayNum - 1;
            return (relayState & (1 << bitPos)) != 0;
      }
};

#endif