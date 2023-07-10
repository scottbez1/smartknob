#pragma once

#include <functional>

#include "proto_gen/smartknob.pb.h"

typedef std::function<void(PB_SmartKnobConfig&)> ConfigCallback;
typedef std::function<void(void)> MotorCalibrationCallback;
