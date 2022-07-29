
#include "Triangle.h"
#include "Utils.h"



Triangle::Triangle() : 
        Triangle({Vec3f(0,0,0), Vec3f(0,0,0), Vec3f(0,0,0)})
{
    
}

Triangle::Triangle(const array<Vec3f, 3> &verts) {
    for (int i = 0; i < 3; i ++ )
        p[i] = verts[i];
}


bool Triangle::intersect(Ray ray, Intersection &inter) const {
    Vec3f e1 = p[1] - p[0];
    Vec3f e2 = p[2] - p[0];
    Vec3f d = ray.dir;
    float denominator = mixedProduct(d, e1, e2);
    if (fabs(denominator) < 1e-10)
        return false;
    Vec3f s = ray.orig - p[0];
    float t = -mixedProduct(s, e1, e2) / denominator;
    float b0 = mixedProduct(d, s, e2) / denominator;
    float b1 = mixedProduct(d, e1, s) / denominator;
    if (t > 0 && inside01(b0) && inside01(b1) && inside01(b0 + b1)) {
        inter.yes = true;
        inter.t = t;
        inter.point = ray.orig + t * d;
        inter.normal = (1-b0-b1)*n[0] + b0*n[1] + b1*n[2];
        inter.material = mMaterial;
        return true;
    }
    return false;
}


float Triangle::area() const {
    Vec3f e1 = p[1] - p[0];
    Vec3f e2 = p[2] - p[0];
    return e1.cross(e2).norm() * 0.5f;
}

Vec3f Triangle::center() const {
    return (p[0] + p[1] + p[2]) / 3;
}

Vec3f Triangle::normal() const {
    Vec3f e1 = p[1] - p[0];
    Vec3f e2 = p[2] - p[0];
    return e1.cross(e2).normalized();
}