#include "calculator.h"
#include <stdio.h> // Required for fprintf and stderr
#include <stdlib.h> // Required for exit
#include <math.h>   // Required for fmod

double add(double a, double b) {
    return a + b;
}

double subtract(double a, double b) {
    return a - b;
}

double multiply(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    if (b == 0) {
        return NAN;
    }
    return a / b;
}

double modulus(double a, double b) {
    if (b == 0) {
        return NAN;
    }
    return fmod(a, b);
}