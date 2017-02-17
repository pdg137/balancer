#include <Romi32U4.h>

class State
{
  public:
  State() {}

  uint16_t count;
  int32_t angle; // degrees * 10^-4
  unsigned long last_millis = 0;
  bool last_millis_set = false; 

  void reset()
  {
    angle = 0;
    last_millis_set = false;
  }

  // w: deg/s * 10^-1
  void integrate(int16_t w, int16_t current_millis);
};

