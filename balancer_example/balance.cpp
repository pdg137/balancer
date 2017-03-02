#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>
#include "balance.h"

LSM6 imu;
int32_t gYZero;
Romi32U4Motors motors;
Romi32U4Encoders encoders;

int32_t angle; // millidegrees
int32_t angleRate; // degrees/s
int32_t distanceLeft;
int32_t speedLeft;
int32_t distanceRight;
int32_t speedRight;
int16_t motorSpeed;
bool isBalancingStatus;

bool isBalancing()
{
  return isBalancingStatus;
}

void balance()
{
  // drift toward w=0 with timescale ~10s
  angle = angle*999/1000;

  int32_t diff = angleRate*ANGLE_RATE_RATIO + angle;
  if(diff < -15000 || diff > 15000)
    ledYellow(1);
  else
    ledGreen(1);

  motorSpeed += (
    + ANGLE_RESPONSE * diff
    + DISTANCE_RESPONSE * (distanceLeft + distanceRight)
    + SPEED_RESPONSE * (speedLeft + speedRight)
    ) / 100 / GEAR_RATIO;

  if(motorSpeed > MOTOR_SPEED_LIMIT)
  {
    motorSpeed = MOTOR_SPEED_LIMIT;
  }
  if(motorSpeed < -MOTOR_SPEED_LIMIT)
  {
    motorSpeed = -MOTOR_SPEED_LIMIT;
  }

  int16_t distanceDiff = distanceLeft - distanceRight;

  motors.setSpeeds(motorSpeed - distanceDiff/2,
    motorSpeed + distanceDiff/2);
}

void lyingDown()
{
  // reset things so it doesn't go crazy
  motorSpeed = 0;
  distanceLeft = 0;
  distanceRight = 0;
  motors.setSpeeds(0, 0);

  if(angleRate > -2 && angleRate < 2)
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
    distanceLeft = 0;
    distanceRight = 0;
  }
}

void integrateGyro()
{
  angleRate = (imu.g.y - gYZero)/29; // convert from full-scale 1000 deg/s to deg/s
  angle += angleRate * UPDATE_TIME_MS;
}

void integrateEncoders()
{
  static int16_t lastCountsLeft;
  int16_t countsLeft = encoders.getCountsLeft();
  speedLeft = (countsLeft - lastCountsLeft);
  distanceLeft += countsLeft - lastCountsLeft;
  lastCountsLeft = countsLeft;

  static int16_t lastCountsRight;
  int16_t countsRight = encoders.getCountsRight();
  speedRight = (countsRight - lastCountsRight);
  distanceRight += countsRight - lastCountsRight;
  lastCountsRight = countsRight;
}

void balanceDrive(int16_t driveLeftTicks, int16_t driveRightTicks)
{
  distanceLeft += driveLeftTicks;
  distanceRight += driveRightTicks;
  speedLeft += driveLeftTicks;
  speedRight += driveRightTicks;
}

void balanceSetup()
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
  int32_t total=0;
  for(i=0;i<CALIBRATION_ITERATIONS;i++)
  {
    imu.read();
    total += imu.g.y;
    delay(1);
  }

  gYZero = total / CALIBRATION_ITERATIONS;

  ledRed(0);
}

void balanceResetEncoders()
{
  distanceLeft = 0;
  distanceRight = 0;
}

void balanceUpdateSensors()
{
  imu.read();
  integrateGyro();
  integrateEncoders();
}

void balanceUpdate()
{
  static uint16_t lastMillis;
  uint16_t ms = millis();

  // lock our balancing updates to 100 Hz
  if(ms - lastMillis < UPDATE_TIME_MS) return;
  ledRed(ms - lastMillis > UPDATE_TIME_MS+1);
  ledYellow(0);
  ledGreen(0);
  lastMillis = ms;

  balanceUpdateSensors();

  if(imu.a.x < 0)
  {
    lyingDown();
    isBalancingStatus = false;
  }
  else
  {
    balance();
    isBalancingStatus = true;
  }
}
