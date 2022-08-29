#pragma once

#include "Types.h"
#include "Material.h"

#include <limits>

class Triangle;

struct Intersection
{
    bool yes = false;
    bool is_back = false;
    float t = std::numeric_limits<float>::max();
    Vec3f point;
    Vec3f normal;
    const Triangle *tri = nullptr;
    float bary[3];
    std::shared_ptr<Material> material;

    Vec3f brdf(Vec3f const &wi, Vec3f const &wo);
};

struct Ray {
    // origin of the ray
    Vec3f orig;
    // the direction where the ray is cast, should always be unit vector
    Vec3f dir;

    Ray(Vec3f o, Vec3f d) : orig(o), dir(glm::normalize(d))
    {
    }

    // reflect after an intersection
    Ray reflect(const Intersection &inter) {
        Vec3f ref = dir + 2.f * inter.normal * glm::dot(inter.normal, -dir);
        return Ray( inter.point, ref );
    }
};