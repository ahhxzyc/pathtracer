#pragma once

#include "Types.h"
#include "Ray.h"
#include "Film.h"
#include <memory>


class Camera
{
public:
    Camera(const Size2i &size);

    Ray GetRay(const Point2i loc) const;

    std::shared_ptr<Film> film;

    Vec3f m_up = Vec3f(0, 1, 0);
    Vec3f m_eye = Vec3f(0, 1, 6.8);
    Vec3f m_lookat = Vec3f(0, 1, 5.8);
    float m_fov = 19.5;
};
