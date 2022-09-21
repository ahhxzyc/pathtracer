#pragma once

#include "common.h"
#include <memory>
#include <string>
#include <mutex>
#include <vector>


struct Pixel
{
    Vec3f rgb;
    float weight;
    Pixel() : rgb(0.f), weight(0.f) {}
};

class Film
{
public:
    Film(const Size2i& size);
    void            save(const std::string &filePath);
    Pixel&          pixel_at(const Point2i& location);
    void            add_sample(const Point2i& location, const Vec3f &rgb);
    const Color3b*  framebuffer_ub() const;
public:
    Size2i size;
private:
    std::unique_ptr<Pixel[]> m_Pixels;
    std::unique_ptr<std::vector<Color3b>> m_ColorsUchar;
    std::mutex m_Mutex;
};


class Camera
{
public:
    Camera(const Size2i &size);

    Ray spawn_ray(const Point2i loc) const;

    std::shared_ptr<Film> film;
    Vec3f m_up = Vec3f(0, 1, 0);
    Vec3f m_eye = Vec3f(0, 1, 6.8);
    Vec3f m_lookat = Vec3f(0, 1, 5.8);
    float m_fov = 19.5;
};
