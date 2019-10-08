#pragma once

#include <cmath>
#include "Point.hpp"

const float pi = static_cast<float>(std::atan(1) * 4);

inline float radians(float a) {
    return a * (pi / 180);
}

inline float sin_d(float a) {
    return std::sin(radians(a));
}

inline float cos_d(float a) {
    return std::cos(radians(a));
}

inline float fmapf(float n, float s1, float e1, float s2, float e2) {
    return (s2 + (e2 - s2) * ((n - s1) / (e1 - s1)));
}

inline float square(float val) {
    return val * val;
}

// Euclidian distance for comparisons (without sqrt)
inline float comp_euc_dist(Point<float> p1, Point<float> p2) {
    return square(p1.x - p2.x) + square(p1.y - p2.y);
}

inline float frac(float val) {
    return val - (float)((int)val);
}