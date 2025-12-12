#ifndef TESLASEATCONTROLLER_INO
#define TESLASEATCONTROLLER_INO

#include "TeslaSeatEvents.ino"
#include "ESP32RelayClass.ino"

class TeslaSeatController {
private:
  RelayBoard& relayBoard;

  // Last time seat was moved horizontally
  unsigned long lastSeatHorizontalMove = 0;

public:
  TeslaSeatController(RelayBoard& rb) : relayBoard(rb) {}

  void moveSeat(HorizontalDirection dir, ButtonEvent event) {
    lastSeatHorizontalMove = millis();
    if (event == ButtonEvent::PRESSED) {
      if (dir == HorizontalDirection::FORWARD) {
        relayBoard.setRelay(2, false);
        relayBoard.setRelay(1, true);
      } else if (dir == HorizontalDirection::BACK) {
        relayBoard.setRelay(1, false);
        relayBoard.setRelay(2, true);
      }
    } else if (event == ButtonEvent::RELEASED) {
      relayBoard.setRelay(1, false);
      relayBoard.setRelay(2, false);
    }
  }

  void moveSeat(VerticalDirection dir, ButtonEvent event) {
    if (event == ButtonEvent::PRESSED) {
      if (dir == VerticalDirection::UP) {
        relayBoard.setRelay(4, false);
        relayBoard.setRelay(3, false); // Also coupled to 4, so switch together to prevent moving two things at once

        relayBoard.setRelay(7, true); 
        
      } else if (dir == VerticalDirection::DOWN) {
        relayBoard.setRelay(7, false);

        relayBoard.setRelay(4, true);
        relayBoard.setRelay(3, true); // Also coupled to 4, so switch together to prevent moving two things at once
      }
    } else if (event == ButtonEvent::RELEASED) {
        relayBoard.setRelay(7, false);

        relayBoard.setRelay(4, false);
        relayBoard.setRelay(3, false); // Also coupled to 4, so switch together to prevent moving two things at once
    }
  }

  void changeSeatBackAngle(HorizontalDirection dir, ButtonEvent event) {
    if (event == ButtonEvent::PRESSED) {
      if (dir == HorizontalDirection::FORWARD) {
        relayBoard.setRelay(4, false);
        relayBoard.setRelay(7, false); // Also coupled to 4, so switch together to prevent moving two things at once

        relayBoard.setRelay(3, true);
      } else if (dir == HorizontalDirection::BACK) {
        relayBoard.setRelay(3, false);

        relayBoard.setRelay(4, true);
        relayBoard.setRelay(7, true); // Also coupled to 4, so switch together to prevent moving two things at once
      }
    } else if (event == ButtonEvent::RELEASED) {
        relayBoard.setRelay(3, false);

        relayBoard.setRelay(4, false);
        relayBoard.setRelay(7, false); // Also coupled to 4, so switch together to prevent moving two things at once
    }
  }

  void changeSeatAngle(VerticalDirection dir, ButtonEvent event) {
    if (event == ButtonEvent::PRESSED) {
      if (dir == VerticalDirection::UP) {
        relayBoard.setRelay(6, false);
        relayBoard.setRelay(5, true);
      } else if (dir == VerticalDirection::DOWN) {
        relayBoard.setRelay(5, false);
        relayBoard.setRelay(6, true);
      }
    } else if (event == ButtonEvent::RELEASED) {
      relayBoard.setRelay(5, false);
      relayBoard.setRelay(6, false);
    }
  }

  void moveHeadRest(VerticalDirection dir, ButtonEvent event) {
    if (event == ButtonEvent::PRESSED) {
      if (dir == VerticalDirection::UP) {
        relayBoard.setRelay(16, false);
        relayBoard.setRelay(8, true);
      } else if (dir == VerticalDirection::DOWN) {
        relayBoard.setRelay(8, false);
        relayBoard.setRelay(16, true);
      }
    } else if (event == ButtonEvent::RELEASED) {
      relayBoard.setRelay(8, false);
      relayBoard.setRelay(16, false);
    }
  }

  void moveLumbar(HorizontalDirection dir, ButtonEvent event) {
    if (event == ButtonEvent::PRESSED) {
      if (dir == HorizontalDirection::FORWARD) {
        relayBoard.setRelay(14, false);
        relayBoard.setRelay(15, false); // Also coupled to 14, so switch together to prevent moving two things at once

        relayBoard.setRelay(13, true);
      } else if (dir == HorizontalDirection::BACK) {
        relayBoard.setRelay(13, false);

        relayBoard.setRelay(14, true);
        relayBoard.setRelay(15, true); // Also coupled to 14, so switch together to prevent moving two things at once
      }
    } else if (event == ButtonEvent::RELEASED) {
      relayBoard.setRelay(14, false);
      relayBoard.setRelay(15, false); // Also coupled to 14, so switch together to prevent moving two things at once

      relayBoard.setRelay(13, false);
    }
  }

  void moveLumbar(VerticalDirection dir, ButtonEvent event) {
    // If seat was moved horizontally in the last 10 seconds, move headrest instead of lumbar
    if (millis() - lastSeatHorizontalMove < 10000) {
      moveHeadRest(dir, event);
      return;
    }

    if (event == ButtonEvent::PRESSED) {
      if (dir == VerticalDirection::UP) {
        relayBoard.setRelay(14, false);
        relayBoard.setRelay(13, false); // Also coupled to 14, so switch together to prevent moving two things at once

        relayBoard.setRelay(15, true);
      } else if (dir == VerticalDirection::DOWN) {
        relayBoard.setRelay(15, false);

        relayBoard.setRelay(14, true);
        relayBoard.setRelay(13, true); // Also coupled to 14, so switch together to prevent moving two things at once
      }
    } else if (event == ButtonEvent::RELEASED) {
      relayBoard.setRelay(14, false);
      relayBoard.setRelay(13, false); // Also coupled to 14, so switch together to prevent moving two things at once
      
      relayBoard.setRelay(15, false);
    }
  }
};

#endif
