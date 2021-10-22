#include <SimpleFOC.h>

#include "motor_task.h"
#include "tlv_sensor.h"

MotorTask::MotorTask(const uint8_t task_core, DisplayTask& display_task) : Task{"Motor", 8192, 1, task_core}, display_task_(display_task) {
}

MotorTask::~MotorTask() {}


// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(7, 8);
BLDCDriver6PWM driver = BLDCDriver6PWM(27, 26, 25, 33, 32, 13);

TlvSensor tlv = TlvSensor();


Commander command = Commander(Serial);
void doMotor(char* cmd) { command.motor(&motor, cmd); }


void MotorTask::run() {
    driver.voltage_power_supply = 5;
    driver.init();

    Wire.begin();
    Wire.setClock(400000);
    tlv.init();

    motor.linkDriver(&driver);

    motor.current_limit = 0.6;
    motor.linkSensor(&tlv);

    motor.LPF_angle = 0.01;

    motor.LPF_velocity.Tf = 0.05;

    motor.PID_velocity.P = 0.2;
    motor.PID_velocity.I = 0;
    motor.PID_velocity.D = 0;
    motor.PID_velocity.output_ramp = 1000;

    motor.P_angle.P = 0.1;
    motor.P_angle.I = 0;
    motor.P_angle.D = 0;

    motor.init();




    // float pp_search_voltage = 2; // maximum power_supply_voltage/2
    // float pp_search_angle = 28*PI; // search electrical angle to turn

    // // move motor to the electrical angle 0
    // motor.controller = MotionControlType::angle_openloop;
    // motor.voltage_limit=pp_search_voltage;
    // motor.move(0);
    // _delay(1000);
    // // read the sensor angle
    // tlv.update();
    // float angle_begin = tlv.getAngle();
    // _delay(50);

    // // move the motor slowly to the electrical angle pp_search_angle
    // float motor_angle = 0;
    // while(motor_angle <= pp_search_angle){
    //   motor_angle += 0.01f;
    //   tlv.update(); // keep track of the overflow
    //   motor.move(motor_angle);
    // }
    // _delay(1000);
    // // read the sensor value for 180
    // tlv.update(); 
    // float angle_end = tlv.getAngle();
    // _delay(50);
    // // turn off the motor
    // motor.move(0);
    // _delay(1000);

    // // calculate the pole pair number
    // int pp = round((pp_search_angle)/(angle_end-angle_begin));

    // Serial.print(F("Estimated PP : "));
    // Serial.println(pp);
    // Serial.println(F("PP = Electrical angle / Encoder angle "));
    // Serial.print(pp_search_angle*180/PI);
    // Serial.print(F("/"));
    // Serial.print((angle_end-angle_begin)*180/PI);
    // Serial.print(F(" = "));
    // Serial.println((pp_search_angle)/(angle_end-angle_begin));
    // Serial.println();


    // // a bit of monitoring the result
    // if(pp <= 0 ){
    //   Serial.println(F("PP number cannot be negative"));
    //   Serial.println(F(" - Try changing the search_voltage value or motor/sensor configuration."));
    //   return;
    // }else if(pp > 30){
    //   Serial.println(F("PP number very high, possible error."));
    // }else{
    //   Serial.println(F("If PP is estimated well your motor should turn now!"));
    //   Serial.println(F(" - If it is not moving try to relaunch the program!"));
    //   Serial.println(F(" - You can also try to adjust the target voltage using serial terminal!"));
    // }





    motor.initFOC(0.602, Direction::CW);


    // add the motor to the commander interface
    // The letter (here 'M') you will provide to the SimpleFOCStudio
    command.add('M', &doMotor, "foo");
    // tell the motor to use the monitoring
    motor.useMonitoring(Serial);
    motor.monitor_downsample = 0; // disable monitor at first - optional

    while (1) {
        motor.move();
        motor.loopFOC();
        motor.monitor();
        command.run();
        display_task_.set_angle(motor.shaft_angle);
    }
}