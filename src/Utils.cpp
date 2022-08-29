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
