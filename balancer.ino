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
  print("b) Check batteries");
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

void set_motors(int16_t left, int16_t right)
{
  if(left > 0)
    left += 20;
  if(left < 0)
    left -= 20;

  if(right > 0)
    right += 35;
  if(right < 0)
    right -= 35;
  motors.setSpeeds(-left, -right);
}

int16_t limit(int16_t speed, int16_t l)
{
  if(speed > l)
    speed = l;
  else if(speed < -l)
    speed = -l;
  return speed;
}

bool run_motors = true;
void start_stop_motors()
{
  run_motors = !run_motors;
}

int16_t speed;
void update_motors()
{
  if(!run_motors || state.general_state != State::BALANCING)
  {
    ledRed(0);
    ledYellow(0);
    ledGreen(0);
    speed = 0;
    state.distance_left = 0;
    state.distance_right = 0;
    set_motors(0, 0);
    return;
  }

  ledRed(0);
  ledYellow(0);
  ledGreen(0);

  int32_t diff = state.angle_rate + state.angle/200;
  if(diff < 0 && state.angle > 0)
    ledRed(1);
  else if(diff > 0 && state.angle < 0)
    ledYellow(1);
  else
    ledGreen(1);

  speed += diff / 400;
  //speed += (((int32_t)state.distance_left) + state.distance_right)/400;
  speed = limit(speed, 150);

  int16_t speed2 = speed ;
  speed2 += (state.speed_left + state.speed_right)/4;
  int16_t speed_left = limit(speed2, 150);
  int16_t speed_right = limit(speed2, 150);
  
  set_motors(speed_left, speed_right);
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
  case 'b':
    snprintf(report, sizeof(report), "Battery voltage: %d mV", readBatteryMillivolts());
    print(report);
    break;
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
  state.integrate(imu.w, imu.a_x, imu.a_z, encoders.getCountsLeft(),  encoders.getCountsRight());
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

    snprintf(report, sizeof(report), "%c angle: %c%d.%04d rate: %d enc: %d %d speed: %d",
      general_state,
      negative ? '-' : '+', angle_int, angle_frac,
      (int16_t)state.angle_rate,
      (int16_t)state.distance_left,
      state.speed_left,
      speed);
    Serial.println(report);
    cycle = 0;
  }
}

void setup()
{
  imu.init();
}

enum phase_t { PHASE_WAIT, PHASE_BACK, PHASE_JUMP, PHASE_FORWARD, PHASE_LEFT };

phase_t phase = PHASE_WAIT;

void loop()
{
  static uint16_t last_ms;
  uint16_t ms = millis();

  input();

  // lock our balancing updates to 100 Hz
  if(ms - last_ms < 10) return;
  last_ms = ms;
  
  integrate();
  if(testing) do_tests();


  switch(phase)
  {
    case PHASE_WAIT:
      ledRed(1);
      set_motors(0,0);
      if(ms > 1000 && !imu.calibrated)
      {
        calibrate();
      }
      if(ms > 2000)
      {
        phase = PHASE_FORWARD;
      }
      break;
    case PHASE_BACK:
      set_motors(-150, -150);
      if(ms > 2300)
        phase = PHASE_JUMP;
      break;
    case PHASE_JUMP:
      set_motors(150, 150);
      if(state.angle < 60000 || ms > 2600)
        phase = PHASE_FORWARD;
      break;
    case PHASE_FORWARD:
      update_motors();
      /*
      state.drive_speed_left = -5;
      if(ms%6000 < 5200)
      {
        state.drive_speed_right = -5;
      }
      else
      {
        state.drive_speed_right = 5;
      }
      */
      break;
  }
}
