#pragma once

#include <glm/glm.hpp>

using Vec3f = glm::vec3;
using Vec2f = glm::vec2;

using Point3i = glm::ivec3;
using Point2i = glm::ivec2;
using Point3f = glm::vec3;
using Point2f = glm::vec2;
using Size2i = glm::ivec2;

using Color3f = glm::vec3;
using Color3b = glm::u8vec3;

#define PI 3.1415926f

struct Ray
{
    Vec3f origin;
    Vec3f dir;
    float minT = 0.001f;
    float maxT = std::numeric_limits<float>::max();

    Ray(Vec3f o, Vec3f d) 
        : origin(o), dir(glm::normalize(d))
    {

    }

    static Ray between(const Point3f &a, const Point3f &b)
    {
        auto time = glm::length(b - a);
        Ray ray(a, (b - a) / time);
        ray.maxT = time - 0.0001f;
        return ray;
    }
};

struct CoordinateSystem
{
    Vec3f t, b, n;
    CoordinateSystem() = default;
    CoordinateSystem(const Vec3f& normal);
    Vec3f ToLocal(const Vec3f& worldVec) const;
    Vec3f ToWorld(const Vec3f& localVec) const;
};

float abs_dot(const Vec3f &a, const Vec3f &b, float min_value = 0.f);

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