#include <Wire.h>
#include <LSM6.h>

class IMU
{
  public:

    void init();
    void calibrate();
    void read();

    int32_t g_y_zero;
    int16_t a_x, a_z;
    int16_t w; // deg/s * 10^-1
    LSM6 device;
};

