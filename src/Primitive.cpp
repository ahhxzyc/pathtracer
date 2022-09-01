#include "Primitive.h"
#include "Utils.h"


std::optional<Intersection> Triangle::Intersect(const Ray &ray, float tmin, float tmax) const
{
    Vec3f e1 = p[1] - p[0];
    Vec3f e2 = p[2] - p[0];
    Vec3f d = ray.dir;
    float denominator = mixedProduct(-d, e1, e2);
    if (fabs(denominator) < 1e-10)
        return {};
    Vec3f s = ray.orig - p[0];
    float t = mixedProduct(s, e1, e2) / denominator;
    float u = mixedProduct(-d, s, e2) / denominator;
    float v = mixedProduct(-d, e1, s) / denominator;
    if (t > tmin && t < tmax && inside01(u) && inside01(v) && inside01(u + v))
    {
        Intersection is;
        is.t = t;
        is.point = ray.orig + t * d;
        is.normal = (1 - u - v) * n[0] + u * n[1] + v * n[2];
        is.primitive = this;
        is.bary[0] = 1 - u - v;
        is.bary[1] = u;
        is.bary[2] = v;

        if (glm::dot(ray.dir, is.normal) > 0.f)
        {
            is.normal = -is.normal;
            is.backface = true;
        }
        return {is};
    }
    return {};
}
