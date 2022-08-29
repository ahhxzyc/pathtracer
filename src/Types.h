#pragma once

#include <glm/glm.hpp>

using Vec3f = glm::vec3;
using Vec2f = glm::vec2;

#define PI 3.1415926

//class Vec3f : public Eigen::Vector3f
//{
//public:
//    Vec3f() {}
//    Vec3f(float x) : Eigen::Vector3f(x) {}
//    Vec3f(float x, float y, float z) : Eigen::Vector3f(x, y, z) {}
//
//    Vec3f operator* (const Vec3f &v)
//    {
//        return Vec3f(x()*v.x(), y() * v.y(), z() * v.z());
//    }
//};
//std::ostream &operator<< (std::ostream &os, const Vec3f &v)
//{
//    os << v.x() << ',' << v.y() << ',' << v.z();
//    return os;
//}