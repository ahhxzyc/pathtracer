#include "Light.h"
#include "Primitive.h"


LightSample AreaLight::Sample(const Point3f &point) const
{
    auto sample = m_Primitive.Sample();
    auto dir = glm::normalize(sample.point - point);
    return { sample.point, dir, m_Radiance, sample.pdf };
}
