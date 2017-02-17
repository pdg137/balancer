// This example shows how to blink the three user LEDs on the
// Romi 32U4.

#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>
#include "imu.h"

char report[80];
IMU imu;

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
    print("Calibrating...");
    imu.calibrate();
    snprintf(report, sizeof(report), "A: %6d %6d    G: %6d",
      imu.a_x_zero, imu.a_z_zero, imu.g_y_zero);
  print(report);
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

  snprintf(report, sizeof(report), "A: %6d %6d    w: %6d",
    imu.a_x, imu.a_z, imu.w );
  Serial.println(report);
}

void setup()
{
  imu.init();
}

void loop()
{
  if(testing) do_tests();

  delay(10);
  input();
}
