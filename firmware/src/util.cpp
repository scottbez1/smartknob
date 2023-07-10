#include "util.h"

float lerp(const float value, const float inMin, const float inMax, const float min, const float max) {
    // Map the input value from the input range to the output range
    return ((value - inMin) / (inMax - inMin)) * (max - min) + min;
}
