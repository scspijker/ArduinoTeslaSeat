#ifndef TESLASEATCONTROLLER_INO
#define TESLASEATCONTROLLER_INO

#include "TeslaSeatEvents.ino"
#include "ESP32RelayClass.ino"

class TeslaSeatController {
private:
  RelayBoard& relayBoard;

public:
  TeslaSeatController(RelayBoard& rb) : relayBoard(rb) {}

  void moveSeat(HorizontalDirection dir, ButtonEvent event) {
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
        relayBoard.setRelay(9, false);
        relayBoard.setRelay(8, true);
      } else if (dir == VerticalDirection::DOWN) {
        relayBoard.setRelay(8, false);
        relayBoard.setRelay(9, true);
      }
    } else if (event == ButtonEvent::RELEASED) {
      relayBoard.setRelay(8, false);
      relayBoard.setRelay(9, false);
    }
  }

  void moveLumbar(HorizontalDirection dir, ButtonEvent event) {
    if (event == ButtonEvent::PRESSED) {
      if (dir == HorizontalDirection::FORWARD) {
        relayBoard.setRelay(11, false);
        relayBoard.setRelay(10, false); // Also coupled to 11, so switch together to prevent moving two things at once

        relayBoard.setRelay(12, true);
      } else if (dir == HorizontalDirection::BACK) {
        relayBoard.setRelay(12, false);

        relayBoard.setRelay(11, true);
        relayBoard.setRelay(10, true); // Also coupled to 11, so switch together to prevent moving two things at once
      }
    } else if (event == ButtonEvent::RELEASED) {
      relayBoard.setRelay(11, false);
      relayBoard.setRelay(10, false); // Also coupled to 11, so switch together to prevent moving two things at once

      relayBoard.setRelay(12, false);
    }
  }

  void moveLumbar(VerticalDirection dir, ButtonEvent event) {
    if (event == ButtonEvent::PRESSED) {
      if (dir == VerticalDirection::UP) {
        relayBoard.setRelay(11, false);
        relayBoard.setRelay(12, false); // Also coupled to 11, so switch together to prevent moving two things at once

        relayBoard.setRelay(10, true);
      } else if (dir == VerticalDirection::DOWN) {
        relayBoard.setRelay(10, false);

        relayBoard.setRelay(11, true);
        relayBoard.setRelay(12, true); // Also coupled to 11, so switch together to prevent moving two things at once
      }
    } else if (event == ButtonEvent::RELEASED) {
      relayBoard.setRelay(11, false);
      relayBoard.setRelay(12, false); // Also coupled to 11, so switch together to prevent moving two things at once
      
      relayBoard.setRelay(10, false);
    }
  }
};

#endif
