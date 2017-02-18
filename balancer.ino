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
Romi32U4Motors motors;

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
  print("Balancer 0.0.2");
  print("c) Calibrate");
  print("t) Start/stop tests");
  print("m) Start/stop motors");
}

bool testing = 0;
void start_stop_tests()
{
  testing = !testing;
  state.reset();
}

void set_motors(int16_t speed)
{
  motors.setSpeeds(-speed, -speed);
}

bool run_motors = 0;
void start_stop_motors()
{
  run_motors = !run_motors;
}

void update_motors()
{
  if(!run_motors || state.general_state != State::BALANCING)
  {
    set_motors(0);
    return;
  }

  if(state.angle < 0 && imu.w < 0)
    set_motors(-50);
  else if(state.angle > 0 && imu.w > 0)
    set_motors(50);
  else
  {
    set_motors(-imu.w * 26/10);
  }
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
  case 'm':
    start_stop_motors();
    break;
  default:
    help();
  }
  Serial.print("> ");
}

void integrate()
{
  imu.read();
  state.integrate(millis(), imu.w, imu.a_x, imu.a_z);
}

void do_tests()
{
  static uint8_t cycle = 0;

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
  integrate();
  update_motors();
  if(testing) do_tests();

  delay(10);
  input();
}
