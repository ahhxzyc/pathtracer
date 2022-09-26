#include "light.h"
#include "primitive.h"
#include "common.h"


LightSample AreaLight::Sample(const Intersection &ref) const
{
    // sample underlying primitive
    auto sample = primitive_.sample();
    auto wi = sample.point - ref.point;
    auto d2 = glm::length2(wi);
    wi = glm::normalize(wi);

    // is sample in both upper hemispheres ? 
    auto refCosine = glm::dot(ref.normal, wi);
    auto sampleCosine = glm::dot(sample.normal, -wi);
    float pdf = 0.f;
    if (refCosine > 0.f && sampleCosine > 0.f)
    {
        pdf = Pdf(ref.point, sample.point, sample.normal);
    }

    LightSample ret;
    ret.point   = sample.point;
    ret.Le      = radiance;
    ret.wi      = wi;
    ret.pdf     = pdf;
    return ret;
}

float AreaLight::Pdf(const Point3f &refPoint, const Vec3f &lightPoint, const Vec3f &lightNormal) const
{
    auto d = refPoint - lightPoint;
    auto dist2 = glm::length2(d);
    auto cosine = glm::dot(glm::normalize(d), lightNormal);
    return dist2 / cosine / primitive_.area();
}

Color3f AreaLight::Radiance(const Ray &ray) const
{
    auto is = primitive_.intersect(Ray(ray));
    if (!is || glm::dot(is->normal, -ray.dir) < 0.f)
        return Color3f(0);
    return radiance;
}

Point3f AreaLight::Center() const
{
    return primitive_.Center();
}
