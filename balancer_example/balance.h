const uint8_t CALIBRATION_ITERATIONS=100;
const int16_t MOTOR_SPEED_LIMIT=400;
const uint8_t UPDATE_TIME_MS=10;
const int16_t GEAR_RATIO=111; // 50:1 motors with 45:21 plastic gears

extern int32_t angle; // millidegrees
extern int32_t angle_rate; // degrees/s
extern Romi32U4Motors motors;

void balance_drive(int16_t ticks_left, int16_t ticks_right);
void balance_setup();
void balance_update_sensors();
void balance_reset_encoders();
void balance_update();
bool is_balancing();
