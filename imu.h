#include <Wire.h>
#include <LSM6.h>

class IMU
{
  public:

    IMU(void) {}

    void init();
    void calibrate();
    void read();

    int32_t a_x_zero, a_z_zero, g_y_zero; // in 1000ths
    int16_t a_x, a_z;
    int16_t w; // deg/s * 10^-1
    LSM6 device;
};

