
#pragma once

template <typename T> T CLAMP(const T& value, const T& low, const T& high) 
{
  return value < low ? low : (value > high ? high : value); 
}

#define COUNT_OF(A) (sizeof(A) / sizeof(A[0]))

float lerp(const float value, const float inMin, const float inMax, const float min, const float max);

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}
