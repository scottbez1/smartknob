#pragma once

#if MOTOR_WANZHIDA_ONCE_TOP
#include "motors/wanzhida_once_top.h"
#elif MOTOR_MAD2804
#include "motors/mad2804.h"
#else
#error "No motor configuration specified!"
#endif
