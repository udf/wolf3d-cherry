#pragma once

#include <cmath>
#include "Point.hpp"

const float pi = static_cast<float>(std::atan(1) * 4);

float radians(float a);
float sin_d(float a);
float cos_d(float a);
float fmapf(float n, float s1, float e1, float s2, float e2);

float square(float val);

// Euclidian distance for comparisons (without sqrt)
float comp_euc_dist(Point<float> p1, Point<float> p2);