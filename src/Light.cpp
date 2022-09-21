#include "Light.h"
#include "Primitive.h"
#include "Utils.h"


LightSample AreaLight::Sample(const Point3f &shadingPoint) const
{
    auto sample = m_Primitive.Sample();
    auto wi = sample.point - shadingPoint;
    auto d2 = glm::length2(wi);
    wi = glm::normalize(wi);

    // pdf is transformed to be on solid angle
    auto pdf = sample.pdf * d2 / abs_dot(sample.normal, -wi, 0.0001f);
    return { sample.point, wi, m_Radiance, pdf};
}

float AreaLight::Pdf(const Intersection &is, const Vec3f &wi) const
{
    Ray ray(is.point, wi);
    float ret = 0.f;
    auto lightIs = m_Primitive.Intersect(ray, 0.001f, std::numeric_limits<float>::max());
    if (lightIs)
    {
        auto d2 = glm::length2(lightIs->point - is.point);
        ret = d2 / abs_dot(lightIs->normal, -wi, 0.0001f) * m_Primitive.Area();
    }
    return ret;
}

Color3f AreaLight::Radiance(const Ray &ray) const
{
    return m_Radiance;
}

Point3f AreaLight::Center() const
{
    return m_Primitive.Center();
}
