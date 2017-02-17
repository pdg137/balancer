#include "state.h"

// w: deg/s * 10^-1
// angle: degrees * 10^-4
void State::integrate(int16_t current_millis, int16_t w, int16_t a_x, int16_t a_z)
{
  if(!last_millis_set)
  {
    last_millis = current_millis;
    last_millis_set = true;
    return;
  }

  // check what its general state is
  if(a_x > 0)
  {
    general_state = BALANCING;
  }
  else if(w > -10 && w < 10)
  {
    if(a_z > 0)
    {
      general_state = ON_BOTTOM;
    }
    else
    {
      general_state = ON_TOP;
    }
  }
  else
  {
    general_state = UNSTABLE;
  }

  uint8_t dt = current_millis - last_millis;
  last_millis += dt;

  angle += ((int32_t)w) * dt;

  switch(general_state)
  {
  case BALANCING:
    // drift toward w=0 with timescale ~10s
    angle = angle*999/1000;
    break;
  case ON_TOP:
    angle = -1070000;
    break;
  case ON_BOTTOM:
    angle = 1130000;
    break;
  }

  count ++;
}

