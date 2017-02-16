#include <Wire.h>
#include <LSM6.h>

class IMU
{
  public:

  IMU(void) {}
  
  void init();
  void calibrate();
  void read();

  int16_t a_x_zero, a_z_zero, g_y_zero;
  int16_t a_x, a_z, g_y;
  LSM6 device;
};

