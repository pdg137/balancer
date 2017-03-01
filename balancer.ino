// This example shows how to blink the three user LEDs on the
// Romi 32U4.

#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>

const int16_t CALIBRATION_ITERATIONS=100;
const int16_t MOTOR_SPEED_LIMIT=400;

LSM6 imu;
int32_t g_y_zero;
Romi32U4Motors motors;
Romi32U4Encoders encoders;

int32_t angle; // millidegrees
int32_t angle_rate; // degrees/s
int32_t distance_left;
int16_t speed_left;
int32_t distance_right;
int16_t speed_right;
int16_t motor_speed;

int16_t limit(int16_t speed, int16_t l)
{
  if(speed > l)
    speed = l;
  else if(speed < -l)
    speed = -l;
  return speed;
}

void balance()
{
  // drift toward w=0 with timescale ~10s
  angle = angle*999/1000;

  int32_t diff = angle_rate + angle/200;
  if(diff < 0 && angle > 0 || diff > 0 && angle < 0)
    ledYellow(1);
  else
    ledGreen(1);

  motor_speed +=
    diff / 15
    + (distance_left + distance_right)/800
    + (speed_left + speed_right)/4;
  motor_speed = limit(motor_speed, MOTOR_SPEED_LIMIT);

  int16_t distance_diff = distance_left - distance_right;
  motors.setSpeeds(motor_speed - distance_diff/5, motor_speed + distance_diff/5);
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
      angle = -109000;
    }
    distance_left = 0;
    distance_right = 0;
  }
}

void integrate()
{
  imu.read();

  angle_rate = (((int32_t)imu.g.y)*1000 - g_y_zero)/28570; // convert from full-scale 1000 deg/s to deg/s
  angle += angle_rate * 10; // 100 Hz update rate

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

  // my motors are reverse
  motors.flipLeftMotor(true);
  motors.flipRightMotor(true);
  
  ledRed(0);
}

void loop()
{
  static uint16_t last_ms;
  uint16_t ms = millis();

  // lock our balancing updates to 100 Hz
  if(ms - last_ms < 10) return;
  ledRed(ms - last_ms > 11);
  ledYellow(0);
  ledGreen(0);
  last_ms = ms;
  
  integrate();
 
  if(imu.a.x < 0)
  {
    lying_down();
  }
  else
  {
    balance();
  }
}
