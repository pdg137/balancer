#include "imu.h"

const int16_t CALIBRATION_ITERATIONS=100;

void IMU::read()
{
  device.read();
  a_x = device.a.x;
  a_z = device.a.z;
  w = (((int32_t)device.g.y)*1000 - g_y_zero)/2857; // convert full-scale 1000 deg/s to deg/s*10^-1
}

void IMU::calibrate()
{
  int i;
  int32_t g_y_total=0;
  for(i=0;i<CALIBRATION_ITERATIONS;i++)
  {
    device.read();
    g_y_total += device.g.y;
    delay(1);
  }

  g_y_zero = g_y_total * 1000 / CALIBRATION_ITERATIONS;
  calibrated = true;
}

void IMU::init()
{
  Wire.begin();

  if (!device.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    while (1);
  }
  device.enableDefault();
  device.writeReg(LSM6::CTRL2_G, 0b01011000); // 208 Hz, 1000 deg/s
}

