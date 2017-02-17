// This example shows how to blink the three user LEDs on the
// Romi 32U4.

#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>
#include "imu.h"
#include "state.h"

char report[80];
IMU imu;
State state;

void print(char *string)
{
  Serial.println(string);
}

char read()
{
  return Serial.read();
}

void help()
{
  print("Balancer 0.0.1");
  print("c) Calibrate");
  print("t) Start/stop tests");
}

bool testing = 0;
void start_stop_tests()
{
  testing = !testing;
  state.reset();
}

void input()
{
  char c = read();
  if(c == -1)
  {
    return;
  }

  Serial.println(c);
  switch(c)
  {
  case 'c':
    print("Calibrating...");
    imu.calibrate();
    snprintf(report, sizeof(report), "Gyro calibration: %6d", imu.g_y_zero);
  print(report);
    break;
  case 't':
    start_stop_tests();
    break;
  default:
    help();
  }
  Serial.print("> ");
}


void do_tests()
{
  static uint8_t cycle = 0;
  
  imu.read();
  state.integrate(millis(), imu.w, imu.a_x, imu.a_z);

  cycle += 1;
  if(cycle == 10)
  {
    int32_t angle = state.angle; // *10^4
    bool negative = false;

    if(angle < 0)
    {
      angle = -angle;
      negative = true;
    }

    int16_t angle_int = angle/10000;
    int16_t angle_frac = angle - angle_int*10000L;

    char general_state;
    switch(state.general_state)
    {
    case State::BALANCING: general_state = '|'; break;
    case State::ON_TOP:    general_state = '/'; break;
    case State::ON_BOTTOM: general_state = '\\'; break;
    case State::UNSTABLE:  general_state = '*'; break;
    }

    snprintf(report, sizeof(report), "%c angle: %c%d.%04d",
      general_state,
      negative ? '-' : '+', angle_int, angle_frac );
    Serial.println(report);
    cycle = 0;
  }
}

void setup()
{
  imu.init();
}

void loop()
{
  if(testing) do_tests();

  delay(10);
  input();
}
