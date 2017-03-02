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

  ledRed(1);
  balanceSetup();
  ledRed(0);
}

const char song[] PROGMEM =
  "!O6 T240"
  "l32ab-b>cl8r br b-bb-a a-r gr g-4 g4"
  "a-r gr g-gg-f er e-r d4 e-4"
  "gr msd8d8ml d-4d4"
  "l32efg-gl8r msd8d8ml d-4d4"
  "cd-de-efg-g l32fg-ga-l8r gr g-4 gr";

void driveAround()
{
  if(!buzzer.isPlaying())
  {
    buzzer.playFromProgramSpace(song);
  }

  uint16_t time = millis() % 8192;
  uint16_t driveLeft, driveRight;
  if(time < 1900)
  {
    driveLeft = 20;
    driveRight = 20;
  }
  else if(time < 4096)
  {
    driveLeft = 25;
    driveRight = 15;
  }
  else if(time < 4096+1900)
  {
    driveLeft = 20;
    driveRight = 20;
  }
  else
  {
    driveLeft = 15;
    driveRight = 25;
  }

  balanceDrive(driveLeft, driveRight);
}

void loop()
{
  if(!isBalancing())
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
        delay(UPDATE_TIME_MS);
        balanceUpdateSensors();
        if(angle < 60000)
          break;
      }
      balanceResetEncoders();
    }
  }

  balanceUpdate();

  // Illuminate the red LED if the last full update was too slow.
  ledRed(balanceUpdateDelayed());

  // Display feedback on the yellow and green LEDs.  This is useful
  // for calibrating ANGLE_RATE_RATIO: if the robot is released from
  // nearly vertical and falls onto its bottom, the green LED should
  // remain lit the entire time.  If it is tilted beyond vertical and
  // given a push to fall back to its bottom side, the yellow LED
  // should remain lit until it hits the ground.  In practice, it is
  // hard to achieve both of these perfectly, but if you can get
  // close, your constant will probably be good enough for balancing.
  int32_t diff = angleRate*ANGLE_RATE_RATIO - angle;
  if(diff > 0)
  {
    // On the top side, or pushed from the top side over to the bottom.
    ledYellow(1);
    ledGreen(0);
  }
  else
  {
    // On the bottom side, or pushed from the bottom over to the top.
    ledYellow(0);
    ledGreen(1);
  }
}
