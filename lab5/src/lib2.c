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
    for (float x = A + e; x < B; x += e) {
        result += e * (sinf(x) + sinf(x - e)) / 2;
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
    int ans = 1;
    for (int i = 1; i <= B; ++i) {
        if (A % i == 0 && B % i == 0) {
            ans = i;
        }
    }
    return ans;
}