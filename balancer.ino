// This example shows how to blink the three user LEDs on the
// Romi 32U4.

#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>
#include "state.h"

const int16_t CALIBRATION_ITERATIONS=100;

LSM6 imu;
int32_t g_y_zero;
State state;
Romi32U4Motors motors;
Romi32U4Encoders encoders;

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
int16_t speed;
void update_motors()
{
  if(state.general_state != State::BALANCING)
  {
    ledYellow(0);
    ledGreen(0);
    speed = 0;
    state.distance_left = 0;
    state.distance_right = 0;
    set_motors(0, 0);
    return;
  }

  ledYellow(0);
  ledGreen(0);

  int32_t diff = state.angle_rate + state.angle/200;
  if(diff < 0 && state.angle > 0 || diff > 0 && state.angle < 0)
    ledYellow(1);
  else
    ledGreen(1);

  speed += diff / 200;
  speed += (((int32_t)state.distance_left) + state.distance_right)/800;
  speed += (state.speed_left + state.speed_right)/4;
  speed = limit(speed, 150);

  int16_t distance_diff = state.distance_left - state.distance_right;
  int16_t speed_left = limit(speed - distance_diff/30, 150);
  int16_t speed_right = limit(speed + distance_diff/30, 150);
  
  set_motors(speed_left, speed_right);
}

void integrate()
{
  imu.read();

  int16_t angle_rate; // deg/s*10^-1
  angle_rate = (((int32_t)imu.g.y)*1000 - g_y_zero)/2857; // convert from full-scale 1000 deg/s
  state.integrate(angle_rate, imu.a.x, imu.a.z, encoders.getCountsLeft(),  encoders.getCountsRight());
}

void setup()
{
  // initialize IMU
  Wire.begin();
  if (!imu.init())
  {
    while(true) Serial.println("Failed to detect and initialize IMU!");
  }
  imu.enableDefault();
  imu.writeReg(LSM6::CTRL2_G, 0b01011000); // 208 Hz, 1000 deg/s

  // wait for IMU readings to stabilize
  ledRed(1);
  delay(1000);
  
  // calibrate the gyro
  int i;
  int32_t g_y_total=0;
  for(i=0;i<CALIBRATION_ITERATIONS;i++)
  {
    imu.read();
    g_y_total += imu.g.y;
    delay(1);
  }

  g_y_zero = g_y_total * 1000 / CALIBRATION_ITERATIONS;
  
  ledRed(0);
}

void loop()
{
  static uint16_t last_ms;
  uint16_t ms = millis();

  // lock our balancing updates to 100 Hz
  if(ms - last_ms < 10) return;
  ledRed(ms - last_ms > 11);
  last_ms = ms;
  
  integrate();
  update_motors();
}
