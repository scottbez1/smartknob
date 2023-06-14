#pragma once

#include "proto_gen/smartknob.pb.h"

#define PROTOBUF_PROTOCOL_VERSION (1)

bool config_eq(PB_SmartKnobConfig& first, PB_SmartKnobConfig& second) {
    return first.detent_strength_unit == second.detent_strength_unit
        && first.endstop_strength_unit == second.endstop_strength_unit
        && first.position == second.position
        && first.position_nonce == second.position_nonce
        && first.min_position == second.min_position
        && first.max_position == second.max_position
        && first.position_width_radians == second.position_width_radians
        && first.snap_point == second.snap_point
        && first.sub_position_unit == second.sub_position_unit
        && strcmp(first.text, second.text) == 0
        && first.detent_positions_count == second.detent_positions_count
        && memcmp(first.detent_positions, second.detent_positions, first.detent_positions_count * sizeof(first.detent_positions[0]))
        && first.snap_point_bias == second.snap_point_bias;
}

bool state_eq(PB_SmartKnobState& first, PB_SmartKnobState& second) {
    return first.has_config == second.has_config
        && (!first.has_config || config_eq(first.config, second.config))
        && first.current_position == second.current_position
        && first.sub_position_unit == second.sub_position_unit;
}
