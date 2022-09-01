//
//#include "Triangle.h"
//#include "Utils.h"
//
//#include <iostream>
//
//
//Triangle::Triangle() : 
//        Triangle({Vec3f(0,0,0), Vec3f(0,0,0), Vec3f(0,0,0)})
//{
//    
//}
//
//Triangle::Triangle(const array<Vec3f, 3> &verts) {
//    for (int i = 0; i < 3; i ++ )
//        p[i] = verts[i];
//}
//
//
//bool Triangle::intersect(Ray ray, Intersection &inter, float tmin, float tmax) const {
//    Vec3f e1 = p[1] - p[0];
//    Vec3f e2 = p[2] - p[0];
//    Vec3f d = ray.dir;
//    float denominator = mixedProduct(-d, e1, e2);
//    if (fabs(denominator) < 1e-10)
//        return false;
//    Vec3f s = ray.orig - p[0];
//    float t = mixedProduct(s, e1, e2) / denominator;
//    float u = mixedProduct(-d, s, e2) / denominator;
//    float v = mixedProduct(-d, e1, s) / denominator;
//    if (t > tmin && t < tmax && inside01(u) && inside01(v) && inside01(u + v)) {
//        inter.yes = true;
//        inter.t = t;
//        inter.point = ray.orig + t * d;
//        inter.normal = (1-u-v)*n[0] + u*n[1] + v*n[2];
//        inter.tri = this;
//        inter.material = mMaterial;
//        inter.bary[0] = 1-u-v;
//        inter.bary[1] = u;
//        inter.bary[2] = v;
//
//        if (glm::dot(ray.dir, inter.normal) > 0.f)
//        {
//            inter.normal = -inter.normal;
//            inter.backface = true;
//        }
//        return true;
//    }
//    return false;
//}
//
//
//float Triangle::area() const {
//    Vec3f e1 = p[1] - p[0];
//    Vec3f e2 = p[2] - p[0];
//    return glm::length(glm::cross(e1, e2)) * 0.5f;
//}
//
//Vec3f Triangle::center() const {
//    return (p[0] + p[1] + p[2]) / 3.f;
//}
//
//Vec3f Triangle::normal() const {
//    Vec3f e1 = p[1] - p[0];
//    Vec3f e2 = p[2] - p[0];
//    return glm::normalize(glm::cross(e1, e2));
//}