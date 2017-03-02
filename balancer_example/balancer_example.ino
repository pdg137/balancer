// Balancing robot example using the Romi 32U4 library.
// This demo is tuned for the 50:1 gearmotor and 45:21 plastic gears;
// you will need to adjust the parameters in balance() for your robot.

#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>
#include "balance.h"

Romi32U4Buzzer buzzer;
Romi32U4ButtonA buttonA;

void setup()
{
  // Uncomment these lines if your motors are reversed.
  // motors.flipLeftMotor(true);
  // motors.flipRightMotor(true);

  balance_setup();
}

const char song[] PROGMEM =
  "!O6 T240"
  "l32ab-b>cl8r br b-bb-a a-r gr g-4 g4"
  "a-r gr g-gg-f er e-r d4 e-4"
  "gr msd8d8ml d-4d4"
  "l32efg-gl8r msd8d8ml d-4d4"
  "cd-de-efg-g l32fg-ga-l8r gr g-4 gr";

void drive_around()
{
  if(!buzzer.isPlaying())
    buzzer.playFromProgramSpace(song);
  
  uint16_t time = millis() % 8192;
  uint16_t drive_left, drive_right;
  if(time < 1900)
  {
    drive_left = 20;
    drive_right = 20;
  }
  else if(time < 4096)
  {
    drive_left = 25;
    drive_right = 15;
  }
  else if(time < 4096+1900)
  {
    drive_right = 20;
    drive_left = 20;
  }
  else
  {
    drive_left = 15;
    drive_right = 25;
  }

  balance_drive(drive_left, drive_right);
}

void loop()
{
  if(!is_balancing())
  {
    buzzer.stopPlaying();
    if(buttonA.getSingleDebouncedPress())
    {
      motors.setSpeeds(0,0);
      buzzer.play("!frfr");
      while(buzzer.isPlaying());
      buzzer.play(">c2");
      motors.setSpeeds(-MOTOR_SPEED_LIMIT,-MOTOR_SPEED_LIMIT);
      delay(400);
      motors.setSpeeds(200,200);
      for(uint8_t i=0;i<20;i++)
      {
        delay(10);
        balance_update_sensors();
        if(angle < 60000)
          break;
      }
      balance_reset_encoders();
    }
  }
  
  balance_update();
}
