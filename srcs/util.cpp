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

float fmapf(float n, float s1, float e1, float s2, float e2) {
	return (s2 + (e2 - s2) * ((n - s1) / (e1 - s1)));
}