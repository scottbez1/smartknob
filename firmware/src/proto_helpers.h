#pragma once

#include "proto_gen/smartknob.pb.h"

bool config_eq(PB_SmartKnobConfig& first, PB_SmartKnobConfig& second) {
    return first.detent_strength_unit == second.detent_strength_unit
        && first.endstop_strength_unit == second.endstop_strength_unit
        && first.num_positions == second.num_positions
        && first.position == second.position
        && first.position_width_radians == second.position_width_radians
        && first.snap_point == second.snap_point
        && strcmp(first.text, second.text) == 0;
}

bool state_eq(PB_SmartKnobState& first, PB_SmartKnobState& second) {
    return first.has_config == second.has_config
        && (!first.has_config || config_eq(first.config, second.config))
        && first.current_position == second.current_position
        && first.sub_position_unit == second.sub_position_unit;
}
