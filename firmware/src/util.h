
#pragma once

template <typename T> T CLAMP(const T& value, const T& low, const T& high) 
{
  return value < low ? low : (value > high ? high : value); 
}

#define COUNT_OF(A) (sizeof(A) / sizeof(A[0]))
