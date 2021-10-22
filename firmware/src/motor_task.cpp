#include <SimpleFOC.h>

#include "motor_task.h"
#include "tlv_sensor.h"

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

    float attract_angle = 0;

    disableCore0WDT();

    while (1) {
        motor.loopFOC();

        if (fabs(detents) < 0.01) {
            motor.move(0);
            display_task_.set_angle(motor.shaft_angle);
        } else {
            float detent_angle = 2*PI/detents;

            float error = (attract_angle - motor.shaft_angle);
            motor.move(motor.PID_velocity(error));

            attract_angle = round(motor.shaft_angle/detent_angle)*detent_angle;
            display_task_.set_angle(attract_angle);
        }
        motor.monitor();
        command.run();
    }
}
