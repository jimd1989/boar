#pragma once

/* Return the lesser of two values. */
#define LESSER(x, y) (x > y ? y : x)

float truncateFloat(const float, const float);
float liftFloat(const float, const float);
float clip(const float);
float expCurve(const float);
float unipolar(const float);
float interpolate(const float *, const int, const float);
