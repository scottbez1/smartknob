#include <SimpleFOC.h>

#include "motor_task.h"
#include "tlv_sensor.h"


template <typename T> T CLAMP(const T& value, const T& low, const T& high) 
{
  return value < low ? low : (value > high ? high : value); 
}

static const float DEAD_ZONE_DETENT_PERCENT = 0.2;
static const float DEAD_ZONE_RAD = 1 * _PI / 180;

static const float IDLE_VELOCITY_EWMA_ALPHA = 0.001;
static const float IDLE_VELOCITY_RAD_PER_SEC = 0.05;
static const uint32_t IDLE_CORRECTION_DELAY_MILLIS = 500;
static const float IDLE_CORRECTION_MAX_ANGLE_RAD = 5 * PI / 180;
static const float IDLE_CORRECTION_RATE_ALPHA = 0.0005;


MotorTask::MotorTask(const uint8_t task_core, DisplayTask& display_task) : Task{"Motor", 8192, 1, task_core}, display_task_(display_task) {
}

MotorTask::~MotorTask() {}


// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(7);
BLDCDriver6PWM driver = BLDCDriver6PWM(27, 26, 25, 33, 32, 13);

TlvSensor tlv = TlvSensor();


Commander command = Commander(Serial);


float detents = 36;

void doMotor(char* cmd) { command.motor(&motor, cmd); }
void doDetents(char* cmd) { command.scalar(&detents, cmd); }

void MotorTask::run() {
    driver.voltage_power_supply = 5;
    driver.init();

    Wire.begin();
    Wire.setClock(400000);
    tlv.init();

    motor.linkDriver(&driver);

    motor.controller = MotionControlType::torque;
    motor.voltage_limit = 5;
    motor.linkSensor(&tlv);

    // Not actually using the velocity loop; but I'm using those PID variables
    // because SimpleFOC studio supports updating them easily over serial for tuning.
    motor.PID_velocity.P = 4;
    motor.PID_velocity.I = 0;
    motor.PID_velocity.D = 0.04;
    motor.PID_velocity.output_ramp = 10000;
    motor.PID_velocity.limit = 10;


    motor.init();


    motor.initFOC(-0.2, Direction::CW);

    command.add('M', &doMotor, "foo");
    command.add('D', &doDetents, "Detents");
    motor.useMonitoring(Serial);
    motor.monitor_downsample = 0; // disable monitor at first - optional

    // disableCore0WDT();

    float current_detent_center = motor.shaft_angle;

    int min = 0;
    int max = 255;
    int value = 0;

    float idle_check_velocity_ewma = 0;
    uint32_t last_idle_start = 0;
    uint32_t last_debug = 0;

    while (1) {

        motor.loopFOC();

        idle_check_velocity_ewma = motor.shaft_velocity * IDLE_VELOCITY_EWMA_ALPHA + idle_check_velocity_ewma * (1 - IDLE_VELOCITY_EWMA_ALPHA);
        if (fabsf(idle_check_velocity_ewma) > IDLE_VELOCITY_RAD_PER_SEC) {
            last_idle_start = 0;
        } else {
            if (last_idle_start == 0) {
                last_idle_start = millis();
            }
        }

        // If we are not moving and we're close to the center (but not exactly there), slowly adjust the centerpoint to match the current position
        if (last_idle_start > 0 && millis() - last_idle_start > IDLE_CORRECTION_DELAY_MILLIS && fabsf(motor.shaft_angle - current_detent_center) < IDLE_CORRECTION_MAX_ANGLE_RAD) {
            current_detent_center = motor.shaft_angle * IDLE_CORRECTION_RATE_ALPHA + current_detent_center * (1 - IDLE_CORRECTION_RATE_ALPHA);
            if (millis() - last_debug > 100) {
                last_debug = millis();
                Serial.print("Moving detent center. ");
                Serial.print(current_detent_center);
                Serial.print(" ");
                Serial.println(motor.shaft_angle);
            }
        }

        if (fabs(detents) < 0.01) {
            motor.move(0);
            display_task_.set_angle(motor.shaft_angle);
            current_detent_center = motor.shaft_angle;
        } else {
            float detent_width = 2*PI/detents;

            float angle_to_detent_center = motor.shaft_angle - current_detent_center;
            if (angle_to_detent_center > detent_width * 1.2 && value > min) {
                current_detent_center += detent_width;
                angle_to_detent_center -= detent_width;
                value--;
            } else if (angle_to_detent_center < -detent_width * 1.2 && value < max) {
                current_detent_center -= detent_width;
                angle_to_detent_center += detent_width;
                value++;
            }

            float dead_zone_adjustment = CLAMP(
                angle_to_detent_center,
                fmaxf(-detent_width*DEAD_ZONE_DETENT_PERCENT, -DEAD_ZONE_RAD),
                fminf(detent_width*DEAD_ZONE_DETENT_PERCENT, DEAD_ZONE_RAD));
            
            if (fabsf(motor.shaft_velocity) > 20) {
                // Don't apply torque if velocity is too high (helps avoid feedback loop)
                motor.move(0);
            } else {
                motor.move(motor.PID_velocity(-angle_to_detent_center + dead_zone_adjustment));
            }

            display_task_.set_angle(-value * PI / 180);
        }
        motor.monitor();
        command.run();
    }
}
