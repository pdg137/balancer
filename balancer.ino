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
Romi32U4Encoders encoders;

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
  print("Balancer 0.0.3");
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

bool run_motors = true;
void start_stop_motors()
{
  run_motors = !run_motors;
}

void update_motors()
{
  static int16_t speed;
  int16_t target, ramp;
  if(!run_motors || state.general_state != State::BALANCING)
  {
    set_motors(0);
    return;
  }

  int32_t target_a = 15*10000; // 15 degrees
  static int8_t dir = 1;
  if(state.distance < -200)
    dir = +1;
  else if(state.distance > 200)
    dir = -1;

  ledRed(0);
  ledYellow(0);
  ledGreen(0);
  ramp = 20;

  if(dir*state.angle < 0)
  {
    // wrong half
    target = -150*dir;
    ramp = 20;
  }
  else if(dir*state.angle < dir*target_a)
  {
    // above the target angle; just wait
    target = speed;
    ledRed(1);
    ledGreen(1);
    ledYellow(1);
  }
  else if(dir*state.angle_rate > 0)
  {
    // below the target angle and falling - catch
    target = 150*dir;
    ledRed(1);
  }
  else
  {
    // below the target angle and rising - aim for the angle at speed 0
    int32_t intercept = state.angle - state.angle_rate*state.angle_rate*12/state.angle*1000;
    if(dir*intercept < dir*target_a)
    {
      // aiming too high
      target = -dir*150;
      ledYellow(1);
    }
    else
    {
      // aiming too low
      target = dir*150;
      ledGreen(1);
    }
    ramp = dir*(target_a - intercept)/60000;
    if(ramp > 30)
      ramp = 30;
    if(ramp < 0) // shouldn't happen
      ramp = 0;
  }

  if(speed < target)
    speed += ramp;
  else if(speed > target)
    speed -= ramp;
  set_motors(speed);
}

void calibrate()
{
  print("Calibrating...");
  imu.calibrate();
  snprintf(report, sizeof(report), "Gyro calibration: %6d", imu.g_y_zero);
  print(report);
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
    calibrate();
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
  state.integrate(millis(), imu.w, imu.a_x, imu.a_z, encoders.getCountsLeft());
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

    snprintf(report, sizeof(report), "%c angle: %c%d.%04d rate: %d enc: %d",
      general_state,
      negative ? '-' : '+', angle_int, angle_frac,
      (int16_t)state.angle_rate,
      (int16_t)state.distance);
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
  if(testing) do_tests();

  uint16_t ms = millis();
  if(ms > 1000 && !imu.calibrated) calibrate();

  if(ms > 2000 && ms < 2400)
    set_motors(-200);
  else if(ms >= 2400 && ms < 2500)
    set_motors(+200);
  else
    update_motors();

  delay(10);
  input();
}
