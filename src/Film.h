#pragma once

#include "Types.h"
#include <memory>
#include <string>
#include <mutex>

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
    void    Save(const std::string &filePath);
    Pixel&  GetPixel(const Point2i& location);
    void    AddSample(const Point2i& location, const Vec3f &rgb);

    auto Size() const {return m_Size;}
private:
    Size2i m_Size;
    std::unique_ptr<Pixel[]> m_Pixels;
    std::mutex m_Mutex;
};