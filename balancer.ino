// This example shows how to blink the three user LEDs on the
// Romi 32U4.

#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>

char report[80];
LSM6 imu;

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
  print("Balancer 0.0.1");
  print("c) Calibrate");
  print("t) Start/stop tests");
}

int16_t a_x_zero, a_z_zero, g_y_zero;
const int16_t CALIBRATION_ITERATIONS=100;
void calibrate()
{
  print("Calibrating...");
  int i;
  int32_t a_x_total, a_z_total, g_y_total;
  for(i=0;i<CALIBRATION_ITERATIONS;i++)
  {
    imu.read();
    a_x_total += imu.a.x;
    a_z_total += imu.a.z;
    g_y_total += imu.g.y;
    delay(1);
  }

  a_x_zero = a_x_total / CALIBRATION_ITERATIONS;
  a_z_zero = a_z_total / CALIBRATION_ITERATIONS;
  g_y_zero = g_y_total / CALIBRATION_ITERATIONS;

  snprintf(report, sizeof(report), "A: %6d %6d    G: %6d",
    a_x_zero, a_z_zero, g_y_zero);
  print("Done.");
}

bool testing = 0;
void start_stop_tests()
{
  testing = !testing;
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
  case 'c':
    calibrate();
    break;
  case 't':
    start_stop_tests();
    break;
  default:
    help();
  }
  Serial.print("> ");
}

void do_tests()
{
  imu.read();

  snprintf(report, sizeof(report), "A: %6d %6d    G: %6d",
    imu.a.x - a_x_zero, imu.a.z - a_z_zero,
    imu.g.y - g_y_zero);
  Serial.println(report);
}

void setup()
{
  Wire.begin();

  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    while (1);
  }
  imu.enableDefault();
  //imu.writeReg(CTRL2_G, 0x80);
}

void loop()
{
  if(testing) do_tests();

  input();
}
