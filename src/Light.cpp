#include "Light.h"
#include "Primitive.h"
#include "Utils.h"


LightSample AreaLight::Sample(const Point3f &shadingPoint) const
{
    auto sample = m_Primitive.Sample();
    auto wi = sample.point - shadingPoint;
    auto d2 = glm::length2(wi);
    wi = glm::normalize(wi);
    auto pdf = sample.pdf * d2 / abs_dot(sample.normal, -wi, 0.0001f);
    return { sample.point, wi, m_Radiance, pdf};
}
