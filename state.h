#include <Romi32U4.h>

class State
{
  public:
  State() {}

  uint16_t count;
  int32_t angle; // degrees * 10^-4
  int32_t angle_rate; // degrees/s * 10-1
  unsigned long last_millis = 0;
  bool last_millis_set = false;
  int32_t distance;
  int16_t speed;
  int16_t last_counts_left;

  enum general_state_t { BALANCING, ON_BOTTOM, ON_TOP, UNSTABLE }; 
  general_state_t general_state;

  void reset()
  {
    angle = 0;
    last_millis_set = false;
  }

  // w: deg/s * 10^-1
  void integrate(int16_t current_millis, int16_t w, int16_t a_x, int16_t a_y,
    int16_t counts_left);
};

