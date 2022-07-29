#pragma once

#include "Types.h"
#include "Material.h"
#include <limits>
using namespace std;

struct Intersection {
    bool yes = false;
    float t = numeric_limits<float>::max();
    Vec3f point;
    Vec3f normal;
    Material material;
};