#include "state.h"

// w: deg/s * 10^-1
// angle: degrees * 10^-4
void State::integrate(int16_t w, int16_t current_millis)
{
  if(!last_millis_set)
  {
    last_millis = current_millis;
    last_millis_set = true;
    return;
  }

  uint8_t dt = current_millis - last_millis;
  last_millis += dt;

  angle += w * dt;
  count ++;
}

