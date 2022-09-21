#include "Bound3f.h"
#include "Log.h"
#include <limits>

Bound3f::Bound3f() : 
        m_min(Vec3f(std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max())),
    m_max(Vec3f(std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest()))
{
    
}

Bound3f::Bound3f(Vec3f minp, Vec3f maxp) :
        m_min(minp), m_max(maxp)
{
    
}


Bound3f Bound3f::Union(Bound3f box) const
{
    return Bound3f(
            glm::min(m_min, box.m_min),
            glm::max(m_max, box.m_max));
}

Bound3f Bound3f::Union(Point3f point) const
{
    return Bound3f(glm::min(m_min, point), glm::max(m_max, point));
}

int Bound3f::MaxExtent() const
{
    auto diag = glm::abs(m_max - m_min);
    if (diag.x > diag.y && diag.x > diag.z)
    {
        return 0;
    }
    else if (diag.y > diag.z)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}


bool Bound3f::contain(Vec3f p) {
    for (int i = 0; i < 3; i ++ )
        if (p[i] > m_max[i] || p[i] < m_min[i])
            return false;
    return true;
}


Point3f Bound3f::Center() const
{
    return (m_max + m_min) * 0.5f;
}

bool Bound3f::ExistIntersection(const Ray &ray) const
{
    Vec3f v0 = (m_min - ray.origin) / ray.dir;
    Vec3f v1 = (m_max - ray.origin) / ray.dir;
    auto tmin = ray.minT, tmax = ray.maxT;
    for (int i = 0; i < 3; i ++)
    {
        if (v0[i] > v1[i]) std::swap(v0[i], v1[i]);
        // Magic trick to reduce rounding error in pbrt 3ed 3.1.2 Ray�CBounds Intersections
        v1[i] *= 1.001f;
        tmin = tmin < v0[i] ? v0[i] : tmin;
        tmax = tmax > v1[i] ? v1[i] : tmax;
    }
    return tmin < tmax;
}
