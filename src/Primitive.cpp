#include "Primitive.h"
#include "Utils.h"
#include "Bound3f.h"

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
        is.normal = glm::normalize((1 - u - v) * n[0] + u * n[1] + v * n[2]);
        is.uv = (1-u-v) * uv[0] + u * uv[1] + v * uv[2];
        is.primitive = this;
        is.wo = -ray.dir;
        if (glm::dot(ray.dir, is.normal) > 0.f)
        {
            is.normal = -is.normal;
            is.backface = true;
        }
        return {is};
    }
    return {};
}

PrimitiveSample Triangle::Sample() const
{
    // uniform sampling inside the triangle
    // ref: https://jsfiddle.net/jniac/fmx8bz9y/
    float u = rand01(), v = rand01();
    if (u + v > 1)
    {
        u = 1 - u;
        v = 1 - v;
    }
    auto P = (1 - u - v) * p[0] + u * p[1] + v * p[2];
    auto N = glm::normalize((1 - u - v) * n[0] + u * n[1] + v * n[2]);
    return { P, N, 1.f / Area() };

}

float Triangle::Area() const
{
    Vec3f e1 = p[1] - p[0];
    Vec3f e2 = p[2] - p[0];
    return glm::length(glm::cross(e1, e2)) * 0.5f;
}

Bound3f Triangle::BoundingBox() const
{
    Bound3f box;
    for (int i = 0; i < 3; i ++ )
        box = box.Union(p[i]);
    return box;
}

Point3f Triangle::Center() const
{
    return (p[0] + p[1] + p[2]) / 3.f;
}

bool Triangle::ExistIntersection(const Ray &ray, float tmin, float tmax) const
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
    return t > tmin && t < tmax && inside01(u) && inside01(v) && inside01(u + v);
}

GeometricPrimitive::GeometricPrimitive(
    const std::shared_ptr<Material> &material) : 
    m_Material(material)
{

}

void Intersection::BuildBSDF()
{
    // init local orthonormal basis of the BSDF
    auto &onb = bsdf.onb;
    new (&onb) CoordinateSystem(normal);
    auto localWo = onb.ToLocal(wo);

    // add bxdfs to the BSDF
    auto material = primitive->GetMaterial();
    auto kd = material->kd_map->get(uv[0], uv[1]);
    auto ks = material->ks;

    //auto sum = kd + ks;
    //for (int i = 0; i < 3; i ++)
    //{
    //    if (sum[i] > 1.f)
    //    {
    //        kd /= sum[i];
    //        ks /= sum[i];
    //    }
    //}

    bsdf.bxdfs.push_back(std::make_shared<LambertianDiffuse>(kd));
    if (glm::length(ks) > 0.01f)
    {
        bsdf.bxdfs.push_back(std::make_shared<BlinnPhongSpecular>(ks, material->shininess, localWo));
    }

}
