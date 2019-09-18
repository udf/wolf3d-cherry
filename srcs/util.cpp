#include "util.hpp"

float radians(float a) {
    return a * (pi / 180);
}

float sin_d(float a) {
    return std::sin(radians(a));
}

float cos_d(float a) {
    return std::cos(radians(a));
}