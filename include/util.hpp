#pragma once

#include <cmath>

const float pi = static_cast<float>(std::atan(1) * 4);

float radians(float a);
float sin_d(float a);
float cos_d(float a);
float fmapf(float n, float s1, float e1, float s2, float e2);