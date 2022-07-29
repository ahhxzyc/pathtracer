#pragma once

#include "Types.h"
#include "Ray.h"

// clamp a float to [a,b]
float clamp(float a, float b, float f);

// check if a float lies in [0,1]
bool inside01(float f);

// a random number in [0,1]
float rand01();

// calculate the mixed product of 3 vectors
float mixedProduct(const Vec3f &a, const Vec3f &b, const Vec3f &c);

// distance from point to ray
float distFromPointToRay(const Vec3f &p, const Ray &r);

// check whether a vector is (0,0,0)
bool almostZero(Vec3f v);