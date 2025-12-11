#ifndef TESLASEATEVENTS_INO
#define TESLASEATEVENTS_INO

enum class Event: uint8_t {
  UNKNOWN           = 0,
  SEAT_HORIZONTAL   = 1,
  SEAT_VERTICAL     = 2,
  BACK_ANGLE        = 3,
  SEAT_ANGLE        = 4,
  HEADREST          = 5,
  LUMBAR_HORIZONTAL = 6,
  LUMBAR_VERTICAL   = 7,
  HELP              = 8,
  STATUS            = 9,
  RELAY             = 10,
  CALIB             = 11,
  DEMO              = 12,
  REBOOT            = 13
};


enum class HorizontalDirection: uint8_t {
  FORWARD = 0,
  BACK    = 1
};

enum class VerticalDirection: uint8_t {
  UP    = 0,
  DOWN  = 1
};

enum class ButtonEvent: uint8_t {
  PRESSED   = 0,
  RELEASED  = 1
};

#endif // TESLASEATEVENTS_INO