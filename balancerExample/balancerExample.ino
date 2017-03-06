// Balancing robot example using the Romi 32U4 library.
// This demo is tuned for the 50:1 gearmotor and 45:21 plastic gears;
// you will need to adjust the parameters in balance() for your robot.

#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>
#include "balance.h"

Romi32U4Buzzer buzzer;
Romi32U4ButtonA buttonA;

uint16_t start_time;

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
  "<bcd-d e-efg- ga-ab- a4 gr";

void playSong()
{
  if(!buzzer.isPlaying())
  {
    buzzer.playFromProgramSpace(song);
  }
}

void driveAround()
{
  uint16_t time = millis() - start_time;
  uint16_t left, right;
  const uint16_t TURN_TIME = 1650;
  const uint16_t STRAIGHT_TIME = 600;
  if(time < STRAIGHT_TIME)
  {
    left = -20;
    right = -20;
  }
  else if(time < STRAIGHT_TIME+TURN_TIME)
  {
    left = -22;
    right = -5;
  }
  else if(time < STRAIGHT_TIME+TURN_TIME+STRAIGHT_TIME)
  {
    left = -20;
    right = -20;
  }
  else if(time < STRAIGHT_TIME+TURN_TIME+STRAIGHT_TIME+TURN_TIME)
  {
    left = -5;
    right = -22;
  }
  else
  {
    start_time += STRAIGHT_TIME+TURN_TIME+STRAIGHT_TIME+TURN_TIME;
    return;
  }

  balanceDrive(left, right);
}

void standUp()
{
  motors.setSpeeds(0,0);
  ledGreen(0);
  ledRed(0);
  ledYellow(0);
  delay(5000);
  buzzer.play("!>grms>g16>g16>g2");
  ledGreen(1);
  ledRed(1);
  ledYellow(1);
  while(buzzer.isPlaying());

  for(uint8_t i=0;i<40;i++)
  {
    motors.setSpeeds(-i*20, -i*20);
    delay(UPDATE_TIME_MS);
    balanceUpdateSensors();
    if(speedLeft < -40)
    {
      break;
    }
  }

  for(uint8_t i=0;i<20;i++)
  {
    motors.setSpeeds(i*20,i*20);
    delay(UPDATE_TIME_MS);
    balanceUpdateSensors();
    if(angle < 60000)
    {
      break;
    }
  }
  motorSpeed = 150;
  balanceResetEncoders();
}

void loop()
{
  static bool started = false;
  balanceUpdate();

  if(isBalancing() || started)
  {
    // Once you have it balancing well, uncomment these lines for
    // something fun.
    started = true;
    playSong();
    driveAround();
  }
  else
  {
    buzzer.stopPlaying();
    if(buttonA.getSingleDebouncedPress())
    {
      standUp();
      start_time = millis();
    }
  }

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
