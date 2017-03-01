// Balancing robot example using the Romi 32U4 library.
// This demo is tuned for the 50:1 gearmotor and 45:21 plastic gears;
// you will need to adjust the parameters in balance() for your robot.

#include <Romi32U4.h>
#include <Wire.h>
#include <LSM6.h>

const uint8_t CALIBRATION_ITERATIONS=100;
const int16_t MOTOR_SPEED_LIMIT=400;
const uint8_t UPDATE_TIME_MS=10;

LSM6 imu;
int32_t g_y_zero;
Romi32U4Motors motors;
Romi32U4Encoders encoders;
Romi32U4Buzzer buzzer;
Romi32U4ButtonA buttonA;

int32_t angle; // millidegrees
int16_t angle_rate; // degrees/s
int32_t distance_left;
int16_t speed_left;
int32_t distance_right;
int16_t speed_right;
int16_t motor_speed;

void balance()
{
  // drift toward w=0 with timescale ~10s
  angle = angle*999/1000;

  int32_t diff = angle_rate + angle/200;
  if(diff < 0 && angle > 0 || diff > 0 && angle < 0)
    ledYellow(1);
  else
    ledGreen(1);

  motor_speed +=
    diff / 8
    + (distance_left + distance_right)/150
    + (speed_left + speed_right)*30/100;
  
  if(motor_speed > MOTOR_SPEED_LIMIT)
  {
    motor_speed = MOTOR_SPEED_LIMIT;
  }
  if(motor_speed < -MOTOR_SPEED_LIMIT)
  {
    motor_speed = -MOTOR_SPEED_LIMIT;
  }

  int16_t distance_diff = distance_left - distance_right;

  motors.setSpeeds(motor_speed - distance_diff/2,
    motor_speed + distance_diff/2);
}

void lying_down()
{
  // reset things so it doesn't go crazy
  motor_speed = 0;
  distance_left = 0;
  distance_right = 0;
  motors.setSpeeds(0, 0);
  buzzer.stopPlaying();

  if(angle_rate > -2 && angle_rate < 2)
  {
    // it's really calm, so we know the angles
    if(imu.a.z > 0)
    {
      angle = 110000;
    }
    else
    {
      angle = -109000;
    }
    distance_left = 0;
    distance_right = 0;
  }
}

void integrate_gyro()
{
  angle_rate = (imu.g.y - g_y_zero)/29; // convert from full-scale 1000 deg/s to deg/s
  angle += angle_rate * UPDATE_TIME_MS;
}

void integrate_encoders()
{
  static int16_t last_counts_left;
  int16_t counts_left = encoders.getCountsLeft();
  speed_left = (counts_left - last_counts_left);
  distance_left += counts_left - last_counts_left;
  last_counts_left = counts_left;

  static int16_t last_counts_right;
  int16_t counts_right = encoders.getCountsRight();
  speed_right = (counts_right - last_counts_right);
  distance_right += counts_right - last_counts_right;
  last_counts_right = counts_right;
}

void setup()
{
  // initialize IMU
  Wire.begin();
  if (!imu.init())
  {
    while(true) Serial.println("Failed to detect and initialize IMU!");
  }
  imu.enableDefault();
  imu.writeReg(LSM6::CTRL2_G, 0b01011000); // 208 Hz, 1000 deg/s

  // wait for IMU readings to stabilize
  ledRed(1);
  delay(1000);
  
  // calibrate the gyro
  int i;
  int32_t g_y_total=0;
  for(i=0;i<CALIBRATION_ITERATIONS;i++)
  {
    imu.read();
    g_y_total += imu.g.y;
    delay(1);
  }

  g_y_zero = g_y_total / CALIBRATION_ITERATIONS;

  // my motors are reversed
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);
  
  ledRed(0);
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

  distance_left += drive_left;
  distance_right += drive_right;
  speed_left += drive_left;
  speed_right += drive_right;
}

void loop()
{
  static uint16_t last_ms;
  static uint8_t lying_down_count = 0;
  uint16_t ms = millis();

  // lock our balancing updates to 100 Hz
  if(ms - last_ms < UPDATE_TIME_MS) return;
  ledRed(ms - last_ms > UPDATE_TIME_MS+1);
  ledYellow(0);
  ledGreen(0);
  last_ms = ms;

  imu.read();
  integrate_gyro();
  integrate_encoders();
 
  if(imu.a.x < 0)
  {
    if(buttonA.getSingleDebouncedPress())
    {
      motors.setSpeeds(0,0);
      buzzer.play("!frfr");
      while(buzzer.isPlaying());
      buzzer.play(">c2");
      motors.setSpeeds(-400,-400);
      delay(400);
      motors.setSpeeds(200,200);
      for(uint8_t i=0;i<20;i++)
      {
        delay(10);
        imu.read();
        integrate_gyro();
      }
      motor_speed = 0;
      distance_left = 0;
      distance_right = 0;
     }
    else
    {
      lying_down();
    }
  }
  else
  {
    // Once you have it balancing well, uncomment this line to make
    // it drive around and play a song.
    // drive_around();
    balance();
  }
}
