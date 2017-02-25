#include "state.h"

// w: deg/s * 10^-1
// angle: degrees * 10^-4
void State::integrate(int16_t w, int16_t a_x, int16_t a_z,
  int16_t counts_left, int16_t counts_right)
{
  speed_left = (counts_left - last_counts_left);
  distance_left += counts_left - last_counts_left - drive_speed_left;
  last_counts_left = counts_left;

  speed_right = (counts_right - last_counts_right);
  distance_right += counts_right - last_counts_right - drive_speed_right;
  last_counts_right = counts_right;

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

  angle_rate = w;
  angle += angle_rate * 10; // 100 Hz update rate

  switch(general_state)
  {
  case BALANCING:
    // drift toward w=0 with timescale ~10s
    angle = angle*999/1000;
    break;
  case ON_TOP:
    angle = -1090000;
    break;
  case ON_BOTTOM:
    angle = 1100000;
    break;
  }

  count ++;
}

