#pragma once

#include "Types.h"
#include "Intersection.h"
#include "Ray.h"
#include "Material.h"
#include <array>
using namespace std;

class Model;

class Triangle {
public:
    Triangle();
    Triangle(const array<Vec3f, 3> &verts);

    // intersection test with a ray
    bool intersect(Ray ray, Intersection &inter) const;

    float area() const ;
    Vec3f center() const;
    Vec3f normal() const;

public:
    Vec3f p[3];
    Vec3f n[3];
    Material mMaterial;
};