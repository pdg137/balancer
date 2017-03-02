#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>
#include "balance.h"

LSM6 imu;
int32_t g_y_zero;
Romi32U4Motors motors;
Romi32U4Encoders encoders;

int32_t angle; // millidegrees
int32_t angle_rate; // degrees/s
int32_t distance_left;
int32_t speed_left;
int32_t distance_right;
int32_t speed_right;
int16_t motor_speed;
bool is_balancing_status;

bool is_balancing()
{
  return is_balancing_status;
}

void balance()
{
  // drift toward w=0 with timescale ~10s
  angle = angle*999/1000;

  int32_t diff = angle_rate*ANGLE_RATE_RATIO + angle;
  if(diff < -15000 || diff > 15000)
    ledYellow(1);
  else
    ledGreen(1);

  motor_speed += (
    + ANGLE_RESPONSE * diff
    + DISTANCE_RESPONSE * (distance_left + distance_right)
    + SPEED_RESPONSE * (speed_left + speed_right)
    ) / 100 / GEAR_RATIO;

  if(motor_speed > MOTOR_SPEED_LIMIT)
  {
    motor_speed = MOTOR_SPEED_LIMIT;
  }
  if(motor_speed < -MOTOR_SPEED_LIMIT)
  {
    motor_speed = -MOTOR_SPEED_LIMIT;
  }

  int16_t distance_diff = distance_left - distance_right;

  motors.setSpeeds(motor_speed - distance_diff/2,
    motor_speed + distance_diff/2);
}

void lying_down()
{
  // reset things so it doesn't go crazy
  motor_speed = 0;
  distance_left = 0;
  distance_right = 0;
  motors.setSpeeds(0, 0);

  if(angle_rate > -2 && angle_rate < 2)
  {
    // it's really calm, so we know the angles
    if(imu.a.z > 0)
    {
      angle = 110000;
    }
    else
    {
      angle = -110000;
    }
    distance_left = 0;
    distance_right = 0;
  }
}

void integrate_gyro()
{
  angle_rate = (imu.g.y - g_y_zero)/29; // convert from full-scale 1000 deg/s to deg/s
  angle += angle_rate * UPDATE_TIME_MS;
}

void integrate_encoders()
{
  static int16_t last_counts_left;
  int16_t counts_left = encoders.getCountsLeft();
  speed_left = (counts_left - last_counts_left);
  distance_left += counts_left - last_counts_left;
  last_counts_left = counts_left;

  static int16_t last_counts_right;
  int16_t counts_right = encoders.getCountsRight();
  speed_right = (counts_right - last_counts_right);
  distance_right += counts_right - last_counts_right;
  last_counts_right = counts_right;
}

void balance_drive(int16_t drive_left_ticks, int16_t drive_right_ticks)
{
  distance_left += drive_left_ticks;
  distance_right += drive_right_ticks;
  speed_left += drive_left_ticks;
  speed_right += drive_right_ticks;
}

void balance_setup()
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

  g_y_zero = g_y_total / CALIBRATION_ITERATIONS;

  ledRed(0);
}

void balance_reset_encoders()
{
  distance_left = 0;
  distance_right = 0;
}

void balance_update_sensors()
{
  imu.read();
  integrate_gyro();
  integrate_encoders();
}

void balance_update()
{
  static uint16_t last_ms;
  static uint8_t lying_down_count = 0;
  uint16_t ms = millis();

  // lock our balancing updates to 100 Hz
  if(ms - last_ms < UPDATE_TIME_MS) return;
  ledRed(ms - last_ms > UPDATE_TIME_MS+1);
  ledYellow(0);
  ledGreen(0);
  last_ms = ms;

  balance_update_sensors();

  if(imu.a.x < 0)
  {
    lying_down();
    is_balancing_status = false;
  }
  else
  {
    // Once you have it balancing well, uncomment this line to make
    // it drive around and play a song.
    // drive_around();
    balance();
    is_balancing_status = true;
  }
}
