#include "imu.h"

const int16_t CALIBRATION_ITERATIONS=100;

void IMU::read()
{
  device.read();
  a_x = device.a.x;
  a_z = device.a.z;
  g_y = device.g.y - g_y_zero;
}

void IMU::calibrate()
{
  int i;
  int32_t a_x_total=0, a_z_total=0, g_y_total=0;
  for(i=0;i<CALIBRATION_ITERATIONS;i++)
  {
    device.read();
    a_x_total += device.a.x;
    a_z_total += device.a.z;
    g_y_total += device.g.y;
    delay(1);
  }

  a_x_zero = a_x_total / CALIBRATION_ITERATIONS;
  a_z_zero = a_z_total / CALIBRATION_ITERATIONS;
  g_y_zero = g_y_total / CALIBRATION_ITERATIONS;
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
//  imu.writeReg(CTRL2_G, 0x80);
}

