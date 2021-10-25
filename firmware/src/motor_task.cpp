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
    queue_ = xQueueCreate(1, sizeof(KnobConfig));
    assert(queue_ != NULL);
}

MotorTask::~MotorTask() {}


// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(7);
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


    // motor.useMonitoring(Serial);

    motor.init();

    tlv.update();
    delay(10);

    motor.initFOC(6, Direction::CW);
    Serial.println(motor.zero_electric_angle);

    command.add('M', &doMotor, "foo");
    // command.add('D', &doDetents, "Detents");
    motor.monitor_downsample = 0; // disable monitor at first - optional

    // disableCore0WDT();

    float current_detent_center = motor.shaft_angle;
    KnobConfig config = {
        .num_positions = 2,
        .position = 0,
        .position_width_radians = 60 * _PI / 180,
        .detent_strength_unit = 0,
    };

    float idle_check_velocity_ewma = 0;
    uint32_t last_idle_start = 0;
    uint32_t last_debug = 0;

    while (1) {
        motor.loopFOC();

        if (xQueueReceive(queue_, &config, 0) == pdTRUE) {
            Serial.println("Got new config");
            current_detent_center = motor.shaft_angle;
        }

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
            // if (millis() - last_debug > 100) {
            //     last_debug = millis();
            //     Serial.print("Moving detent center. ");
            //     Serial.print(current_detent_center);
            //     Serial.print(" ");
            //     Serial.println(motor.shaft_angle);
            // }
        }

        float angle_to_detent_center = motor.shaft_angle - current_detent_center;
        if (angle_to_detent_center > config.position_width_radians * 1.05 && (config.num_positions <= 0 || config.position > 0)) {
            current_detent_center += config.position_width_radians;
            angle_to_detent_center -= config.position_width_radians;
            config.position--;
        } else if (angle_to_detent_center < -config.position_width_radians * 1.05 && (config.num_positions <= 0 || config.position < config.num_positions - 1)) {
            current_detent_center -= config.position_width_radians;
            angle_to_detent_center += config.position_width_radians;
            config.position++;
        }

        float dead_zone_adjustment = CLAMP(
            angle_to_detent_center,
            fmaxf(-config.position_width_radians*DEAD_ZONE_DETENT_PERCENT, -DEAD_ZONE_RAD),
            fminf(config.position_width_radians*DEAD_ZONE_DETENT_PERCENT, DEAD_ZONE_RAD));

        bool out_of_bounds = config.num_positions > 0 && ((angle_to_detent_center > 0 && config.position == 0) || (angle_to_detent_center < 0 && config.position == config.num_positions - 1));
        motor.PID_velocity.limit = out_of_bounds ? 10 : 3;
        motor.PID_velocity.P = out_of_bounds ? 4 : config.detent_strength_unit * 4;
        motor.PID_velocity.D = config.detent_strength_unit * 0.04;

        if (fabsf(motor.shaft_velocity) > 20) {
            // Don't apply torque if velocity is too high (helps avoid feedback loop)
            motor.move(0);
        } else {
            motor.move(motor.PID_velocity(-angle_to_detent_center + dead_zone_adjustment));
        }

        display_task_.setData({
            .num_positions = config.num_positions,
            .current_position = config.position,
            .sub_position_unit = -angle_to_detent_center / config.position_width_radians,
            .position_width_radians = config.position_width_radians,
        });

        motor.monitor();
        // command.run();
    }
}

void MotorTask::setConfig(const KnobConfig& config) {
    xQueueOverwrite(queue_, &config);
}
