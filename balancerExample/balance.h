// This code was developed for a unit using 50:1 motors and 45:21
// plastic gears, for an overall gear ratio of 111.  Adjust the ratio
// below to scale various constants in the balancing algorithm to
// match your robot.
const int16_t GEAR_RATIO=111;

// This constant limits the maximum motor speed.  If your gear ratio
// is lower than what we used, or if you are testing changes to the
// code, you might want to reduce it to prevent your robot from
// zooming away when things go wrong.
const int16_t MOTOR_SPEED_LIMIT=400;

// This constant relates the angle to its rate of change for a robot
// that is falling from a nearly-vertical position or rising up to
// that position.  The relationship is nearly linear; if you have the
// 80mm wheels it should be about 200 - the angle is ~200 times its
// rate, so for example when it falls to 90 degrees it will be moving
// at 90,000/200 = 450 deg/s.
const int16_t ANGLE_RATE_RATIO=200;

// The following three constants define a PID-like algorithm for
// balancing.  Each one determines how much the motors will respond to
// the corresponding variable being off from zero.  See the code in
// balance.cpp for exactly how they are used.  To get it balancing
// from scratch, start with them all at zero and adjust them as
// follows:

// ANGLE_RESPONSE determines the response to a combination of angle
// and angle_rate; the combination measures how far the robot is from
// a stable trajectory.  To test this, use your hand to flick the
// robot up from a resting position.  With a value that is too low, it
// won't stop itself in time; a high value will cause it to slam back
// into the ground or oscillate wildly back and forth.  When
// ANGLE_RESPONSE is adjusted properly, the robot will move just
// enough to stop itself.  However, after stopping itself, it will be
// moving and keep moving in the same direction, usually driving
// faster and faster until it reaches its maximum motor speed and
// falls over.  That's where the next constants come in.
const int16_t ANGLE_RESPONSE=7;

// DISTANCE_RESPONSE determines how much it resists being moved away
// from its starting point.  Counterintuitively, this constant is
// positive: to move forwards, the robot actually has to first roll
// its wheels backwards, so that it can *fall* forwards.  When this
// constant is adjusted properly, the robot will no longer zoom off in
// one directon, but it will drive back and forth a few times before
// falling down.
const int16_t DISTANCE_RESPONSE=73;

// SPEED_RESPONSE supresses the large back-and-forth oscillations
// caused by DISTANCE_RESPONSE.  Increase this until these
// oscillations die down after a few cycles; but if you increase it
// too much it will tend to shudder or vibrate wildly.
const int16_t SPEED_RESPONSE=3300;

// The balancing code is all based on a 100 Hz update rate; if you
// change this, you will have to adjust many other things.
const uint8_t UPDATE_TIME_MS=10;

// Take 100 measurements initially to calibrate the gyro.
const uint8_t CALIBRATION_ITERATIONS=100;

// These variables will be accessible from your project.
extern int32_t angle; // units: millidegrees
extern int32_t angleRate; // units: degrees/s (or millidegrees/ms)
extern Romi32U4Motors motors;

// Call this function to adjust the robot's encoder measurements,
// which will cause it to drive in the corresponding direction.
void balanceDrive(int16_t ticks_left, int16_t ticks_right);

// Call this in your setup() to initialize and calibrate the IMU.
void balanceSetup();

// Call this in loop() to run the full balancing algorithm.
void balanceUpdate();

// Returns true if (according to the balancing algorithm) the robot is
// trying to balance.  When it falls down it shuts off the motors, and
// this function will return false
bool isBalancing();

// Sometimes you will want to take control of the motors but keep
// updating the balancing code's encoders and angle measurements so
// you don't lose track of the robot's position and angle.  Call this
// every 10ms (UPDATE_TIME_MS) to update the sensors, and you will be
// able to resume balancing immediately when you are done.
void balanceUpdateSensors();

// Call this function to reset the encoders.  This is useful after a
// large motion, so that robot does not try to make a huge correction
// to get back to "zero".
void balanceResetEncoders();
