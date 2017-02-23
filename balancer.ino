// This example shows how to blink the three user LEDs on the
// Romi 32U4.

#include <Romi32U4.h>

char report[80];
Romi32U4Motors motors;
Romi32U4Encoders encoders;

int16_t limit(int16_t speed, int16_t l)
{
  if(speed > l)
    speed = l;
  else if(speed < -l)
    speed = -l;
  return speed;
}

bool run_motors = true;
void start_stop_motors()
{
  run_motors = !run_motors;
}

void setup()
{
  motors.flipRightMotor(true);
  motors.flipLeftMotor(true);
}

void loop()
{
  static uint16_t last_ms;
  uint16_t ms = millis();

  // lock updates to 100 Hz
  if(ms - last_ms < 10) return;
  last_ms = ms;

  static uint16_t last_counts;
  int16_t new_counts = ((int32_t)encoders.getCountsLeft()) - last_counts;
  last_counts += new_counts;

  static int16_t last_p;
  int16_t p = new_counts - 1;
  int16_t d = p - last_p;
  last_p = p;
  static uint16_t i;
  
  i = limit(i+p, 150);

  int16_t speed = limit(-p*10 - i, 150);
  static uint8_t count = 0;
  if(count == 10)
  {
    snprintf(report, sizeof(report), "%d %d", readBatteryMillivolts(), speed);
    Serial.println(report);
    count = 0;    
  }
  count ++;

  motors.setSpeeds(speed, 0);
}
