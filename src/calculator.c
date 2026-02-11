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
        fprintf(stderr, "Error: Division by zero!\n");
        // In a real application, you might want to return a special value (e.g., NaN)
        // or set an error flag instead of exiting. For this exercise, we'll exit.
        exit(1);
    }
    return a / b;
}

double modulus(double a, double b) {
    if (b == 0) {
        fprintf(stderr, "Error: Modulus by zero!\n");
        exit(1);
    }
    return fmod(a, b);
}