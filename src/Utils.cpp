#include <algorithm>
#include <random>
#include "Utils.h"
using namespace std;


float abs_dot(const Vec3f &a, const Vec3f &b, float min_value)
{
    return std::max(glm::dot(a, b), min_value);
}

float clamp(float a, float b, float f) {
    f = max(f, a);
    f = min(f, b);
    return f;
}


float rand01() {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<> distrib(0, 1);
    return distrib(generator);
    //return (float) (rand() % 1000) / 1000.f;
}

bool inside01(float f) {
    return f > 0.f && f < 1.f;
}


float mixedProduct(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
    return glm::dot(glm::cross(a, b), c);
}

float distFromPointToRay(const Vec3f &p, const Ray &r) {
    Vec3f op = p - r.orig;
    float t = glm::dot(op, glm::normalize(r.dir));
    float s = glm::length(op);
    return sqrtf(s*s - t*t);
}

bool almostZero(Vec3f v) {
    float eps = 1e-3;
    return fabs(v[0]) < eps && fabs(v[1]) < eps && fabs(v[2]) < eps;
}

CoordinateSystem::CoordinateSystem(const Vec3f &normal) 
    : n(normal)
{
    auto a = (std::abs(n.x) > 0.9f ? Vec3f(0,1,0) : Vec3f(1,0,0));
    t = glm::normalize(glm::cross(a, n));
    b = glm::cross(n, t);
}

Vec3f CoordinateSystem::ToLocal(const Vec3f &worldVec) const
{
    return Vec3f(
        glm::dot(worldVec, t),
        glm::dot(worldVec, b),
        glm::dot(worldVec, n));
}

Vec3f CoordinateSystem::ToWorld(const Vec3f &localVec) const
{
    auto v0 = localVec.x * t;
    auto v1 = localVec.y * b;
    auto v2 = localVec.z * n;
    return v0 + v1 + v2;
}
