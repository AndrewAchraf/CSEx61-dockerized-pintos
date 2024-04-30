//
// Created by amir on 4/30/24.
//

#ifndef PINTOS_PROJECT_FIXED_POINT_H
#define PINTOS_PROJECT_FIXED_POINT_H

#endif //PINTOS_PROJECT_FIXED_POINT_H

#include <stdint.h>
#define F (1 << 14)

typedef int32_t real;

real int_to_fp(int n);
int fp_to_int_round_to_zero( real x);
int fp_to_int_round_to_nearest( real x);
real add_fp_fp( real x,  real y);
real sub_fp_fp( real x,  real y);
real add_fp_int( real x, int n);
real sub_fp_int( real x, int n);
real mul_fp_fp( real x,  real y);
real mul_fp_int( real x, int n);
real div_fp_fp( real x,  real y);
real div_fp_int( real x, int n);
