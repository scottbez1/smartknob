#include <SimpleFOC.h>

#include "motor_task.h"
#include "tlv_sensor.h"

template <typename T> T CLAMP(const T& value, const T& low, const T& high) 
{
  return value < low ? low : (value > high ? high : value); 
}

static const float DEAD_ZONE_DETENT_PERCENT = 0.2;
static const float DEAD_ZONE_RAD = 3 * _PI / 180;


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

    disableCore0WDT();

    float current_detent_center = motor.shaft_angle;

    int min = 0;
    int max = 255;
    int value = 0;


    while (1) {

        motor.loopFOC();

        if (fabs(detents) < 0.01) {
            motor.move(0);
            display_task_.set_angle(motor.shaft_angle);
            current_detent_center = motor.shaft_angle;
        } else {
            float detent_width = 2*PI/detents;

            // if (millis() - last > 500) {
            //     last = millis();
            //     current_detent_center += detent_width;
            // }

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

            float dead_zone = CLAMP(
                angle_to_detent_center,
                fmaxf(-detent_width*DEAD_ZONE_DETENT_PERCENT, -DEAD_ZONE_RAD),
                fminf(detent_width*DEAD_ZONE_DETENT_PERCENT, DEAD_ZONE_RAD));
            
            if (fabsf(motor.shaft_velocity) > 20) {
                motor.move(0);
            } else {
                motor.move(motor.PID_velocity(-angle_to_detent_center + dead_zone));
            }

            display_task_.set_angle(-value * PI / 180);
        }
        motor.monitor();
        command.run();
    }
}
