#ifndef TESLASEATEVENTS_INO
#define TESLASEATEVENTS_INO

enum class Event {
  UNKNOWN,
  SEAT_HORIZONTAL,
  SEAT_VERTICAL,
  BACK_ANGLE,
  SEAT_ANGLE,
  HEADREST,
  LUMBAR_HORIZONTAL,
  LUMBAR_VERTICAL,
  HELP,
  STATUS,
  RELAY,
  CALIB,
  DEMO,
  REBOOT
};


enum class HorizontalDirection {
  FORWARD,
  BACK
};

enum class VerticalDirection {
  UP,
  DOWN
};

enum class ButtonEvent {
  PRESSED,
  RELEASED
};

#endif // TESLASEATEVENTS_INO