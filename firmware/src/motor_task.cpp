#include <SimpleFOC.h>

#include "motor_task.h"
#include "tlv_sensor.h"
#include "mt6701_sensor.h"
#include "util.h"

static const float DEAD_ZONE_DETENT_PERCENT = 0.2;
static const float DEAD_ZONE_RAD = 1 * _PI / 180;

static const float IDLE_VELOCITY_EWMA_ALPHA = 0.001;
static const float IDLE_VELOCITY_RAD_PER_SEC = 0.05;
static const uint32_t IDLE_CORRECTION_DELAY_MILLIS = 500;
static const float IDLE_CORRECTION_MAX_ANGLE_RAD = 5 * PI / 180;
static const float IDLE_CORRECTION_RATE_ALPHA = 0.0005;


MotorTask::MotorTask(const uint8_t task_core) : Task("Motor", 8192, 1, task_core) {
    queue_ = xQueueCreate(1, sizeof(KnobConfig));
    assert(queue_ != NULL);
}

MotorTask::~MotorTask() {}


// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(POLE_PAIRS);
BLDCDriver6PWM driver = BLDCDriver6PWM(PIN_UH, PIN_UL, PIN_VH, PIN_VL, PIN_WH, PIN_WL);

#if SENSOR_TLV
    TlvSensor encoder = TlvSensor();
#elif SENSOR_MT6701
    MT6701Sensor encoder = MT6701Sensor();
#endif

Commander command = Commander(Serial);


void doMotor(char* cmd) { command.motor(&motor, cmd); }

void MotorTask::run() {
    // Hardware-specific configuration:
    // TODO: make this easier to configure
    // Tune zero offset to the specific hardware (motor + mounted magnetic sensor).
    // SimpleFOC is supposed to be able to determine this automatically (if you omit params to initFOC), but
    // it seems to have a bug (or I've misconfigured it) that gets both the offset and direction very wrong!
    // So this value is based on experimentation.
    // TODO: dig into SimpleFOC calibration and find/fix the issue
    // float zero_electric_offset = -0.6; // original proto
    //float zero_electric_offset = 0.4; // handheld 1
    float zero_electric_offset = -0.8; // handheld 2
    bool tlv_invert = true;
    Direction foc_direction = Direction::CCW;


    driver.voltage_power_supply = 5;
    driver.init();

    #if SENSOR_TLV
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(400000);
    encoder.init(Wire, tlv_invert);
    #endif

    #if SENSOR_MT6701
    encoder.init();
    #endif

    motor.linkDriver(&driver);

    motor.controller = MotionControlType::torque;
    motor.voltage_limit = 5;
    motor.linkSensor(&encoder);

    // Not actually using the velocity loop; but I'm using those PID variables
    // because SimpleFOC studio supports updating them easily over serial for tuning.
    motor.PID_velocity.P = 4;
    motor.PID_velocity.I = 0;
    motor.PID_velocity.D = 0.04;
    motor.PID_velocity.output_ramp = 10000;
    motor.PID_velocity.limit = 10;


    // motor.useMonitoring(Serial);

    motor.init();

    encoder.update();
    delay(10);

    motor.initFOC(zero_electric_offset, foc_direction);
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

    uint32_t last_publish = 0;

    while (1) {
        motor.loopFOC();

        if (xQueueReceive(queue_, &config, 0) == pdTRUE) {
            Serial.println("Got new config");
            current_detent_center = motor.shaft_angle;

            // Update derivative factor of torque controller based on detent width.
            // If the D factor is large on coarse detents, the motor ends up making noise because the P&D factors amplify the noise from the sensor.
            // This is a piecewise linear function so that fine detents (small width) get a higher D factor and coarse detents get a small D factor.
            // Fine detents need a nonzero D factor to artificially create "clicks" each time a new value is reached (the P factor is small
            // for fine detents due to the smaller angular errors, and the existing P factor doesn't work well for very small angle changes (easy to
            // get runaway due to sensor noise & lag)).
            // TODO: consider eliminating this D factor entirely and just "play" a hardcoded haptic "click" (e.g. a quick burst of torque in each
            // direction) whenever the position changes when the detent width is too small for the P factor to work well.
            const float derivative_lower_strength = config.detent_strength_unit * 0.04;
            const float derivative_upper_strength = config.detent_strength_unit * 0;
            const float derivative_position_width_lower = 5 * PI / 180;
            const float derivative_position_width_upper = 10 * PI / 180;
            const float raw = derivative_lower_strength + (derivative_upper_strength - derivative_lower_strength)/(derivative_position_width_upper - derivative_position_width_lower)*(config.position_width_radians - derivative_position_width_lower);
            motor.PID_velocity.D = CLAMP(
                raw,
                min(derivative_lower_strength, derivative_upper_strength),
                max(derivative_lower_strength, derivative_upper_strength)
            );
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
        if (angle_to_detent_center > config.position_width_radians * config.snap_point && (config.num_positions <= 0 || config.position > 0)) {
            current_detent_center += config.position_width_radians;
            angle_to_detent_center -= config.position_width_radians;
            config.position--;
        } else if (angle_to_detent_center < -config.position_width_radians * config.snap_point && (config.num_positions <= 0 || config.position < config.num_positions - 1)) {
            current_detent_center -= config.position_width_radians;
            angle_to_detent_center += config.position_width_radians;
            config.position++;
        }

        float dead_zone_adjustment = CLAMP(
            angle_to_detent_center,
            fmaxf(-config.position_width_radians*DEAD_ZONE_DETENT_PERCENT, -DEAD_ZONE_RAD),
            fminf(config.position_width_radians*DEAD_ZONE_DETENT_PERCENT, DEAD_ZONE_RAD));

        bool out_of_bounds = config.num_positions > 0 && ((angle_to_detent_center > 0 && config.position == 0) || (angle_to_detent_center < 0 && config.position == config.num_positions - 1));
        motor.PID_velocity.limit = 10; //out_of_bounds ? 10 : 3;
        motor.PID_velocity.P = out_of_bounds ? config.endstop_strength_unit * 4 : config.detent_strength_unit * 4;



        if (fabsf(motor.shaft_velocity) > 60) {
            // Don't apply torque if velocity is too high (helps avoid positive feedback loop/runaway)
            motor.move(0);
        } else {
            motor.move(motor.PID_velocity(-angle_to_detent_center + dead_zone_adjustment));
        }

        if (millis() - last_publish > 10) {
            publish({
                .current_position = config.position,
                .sub_position_unit = -angle_to_detent_center / config.position_width_radians,
                .config = config,
            });
            last_publish = millis();
        }

        motor.monitor();
        // command.run();

        delay(1);
    }
}

void MotorTask::setConfig(const KnobConfig& config) {
    xQueueOverwrite(queue_, &config);
}

void MotorTask::addListener(QueueHandle_t queue) {
    listeners_.push_back(queue);
}

void MotorTask::publish(const KnobState& state) {
    for (auto listener : listeners_) {
        xQueueOverwrite(listener, &state);
    }
}
