//
// Created by amir on 4/28/24.
//

#include <stdint.h>
#define F (1 << 14)
typedef int32_t real; //upon the given information, we know that the fixed point number is 17.14, so we can use int32_t to represent the fixed point number
//click on int32_t to see the definition of int32_t , it is used as integer or real-floating point number

real int_to_fp(int n) {
    return n * F;
}

int fp_to_int_round_to_zero(real x) {
    return x / F;
}

int fp_to_int_round_to_nearest( real x) {
    if (x >= 0) {
        return (x + F/2) / F;
    } else {
        return (x - F/2) / F;
    }
}

real add_fp_fp( real x, real y) {
    return x+y;
}

real sub_fp_fp( real x,  real y) {
    return x-y;
}

real add_fp_int(real x, int n) {
    return x + int_to_fp(n);
}

real sub_fp_int(real x, int n) {
    return x - int_to_fp(n);
}

real mul_fp_fp(real x, real y) {
    return ((int64_t)x) * y / F;
}

real mul_fp_int( real x, int n) {
    return x * n;
}

real div_fp_fp( real x,  real y) {

    return ((int64_t)x) * F / y;
}

real div_fp_int(real x, int n) {
    return x / n;
}