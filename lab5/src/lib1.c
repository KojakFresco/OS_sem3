#include "contracts.h"

#include <stdbool.h>

#include <math.h>

float SinIntegral(float A, float B, float const e) {
    bool const minus = B < A;
    if (minus) {
        float const temp = A;
        A = B;
        B = temp;
    }

    float result = 0;
    for (float x = A; x < B; x += e) {
        result += e * sinf(x);
    }
    return minus ? -result : result;
}

int GCF(int A, int B) {
    if (A <= 0 || B <= 0) {
        return -1;
    }
    if (A < B) {
        int const temp = A;
        A = B;
        B = temp;
    }
    while (A % B != 0) {
        int const temp = A % B;
        A = B;
        B = temp;
    }
    return B;
}
