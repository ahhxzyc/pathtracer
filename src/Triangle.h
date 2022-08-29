#pragma once

#include "Types.h"
#include "Ray.h"
#include "Material.h"
#include <array>
#include <memory>
using namespace std;

class Model;
class Intersection;

class Triangle {
public:
    Triangle();
    Triangle(const array<Vec3f, 3> &verts);

    // intersection test with a ray
    bool intersect(Ray ray, Intersection &inter, float tmin, float tmax) const;

    float area() const ;
    Vec3f center() const;
    Vec3f normal() const;

public:
    Vec3f p[3];
    Vec3f n[3];
    Vec2f t[3];
    std::shared_ptr<Material> mMaterial;
};