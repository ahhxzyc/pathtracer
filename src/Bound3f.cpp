#include "Bound3f.h"
#include <limits>

Bound3f::Bound3f() : 
        m_min(Vec3f(std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max())),
        m_max(Vec3f(std::numeric_limits<float>::min(),
                std::numeric_limits<float>::min(),
                std::numeric_limits<float>::min()))
{
    
}

Bound3f::Bound3f(Vec3f minp, Vec3f maxp) :
        m_min(minp), m_max(maxp)
{
    
}
//
//Bound3f::Bound3f(Triangle tri) {
//    m_min = glm::min(glm::min(tri.p[0], tri.p[1]), tri.p[2]);
//    m_max = glm::max(glm::max(tri.p[0], tri.p[1]), tri.p[2]);
//}

// merge two AABBs
Bound3f Bound3f::merge(Bound3f box) {
    return Bound3f(
            glm::min(m_min, box.m_min),
            glm::max(m_max, box.m_max)
    );
}

// intersection with a ray
bool Bound3f::intersect(Ray ray, float tmin, float tmax) const
{
    Vec3f v0 = (m_min - ray.orig) / ray.dir;
    Vec3f v1 = (m_max - ray.orig) / ray.dir;
    float t0 = std::numeric_limits<float>::min();
    float t1 = std::numeric_limits<float>::max();
    for (int i = 0; i < 3; i ++ ) {
        t0 = max(t0, min(v0[i], v1[i]));
        t1 = min(t1, max(v0[i], v1[i]));
    }
    t0 = std::max(t0, tmin);
    t1 = std::min(t1, tmax);
    return t0 < t1;
}


Bound3f Bound3f::getSubBox(int idx) const {
    int x = idx & 1;
    int y = (idx & 2) >> 1;
    int z = (idx & 4) >> 2;
    float side_len = (m_max[0] - m_min[0]) * 0.5f;
    Vec3f base(m_min[0] + x * side_len,
                m_min[1] + y * side_len,
                m_min[2] + z * side_len);
    Vec3f temp(base[0] + side_len,
                base[1] + side_len,
                base[2] + side_len);
    return Bound3f(base, temp);
}


bool Bound3f::contain(Vec3f p) {
    for (int i = 0; i < 3; i ++ )
        if (p[i] > m_max[i] || p[i] < m_min[i])
            return false;
    return true;
}

//
//bool Bound3f::contain(Triangle tri) {
//    return contain(tri.p[0]) && contain(tri.p[1]) && contain(tri.p[2]);
//}