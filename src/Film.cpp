#include "Film.h"
#include "Log.h"
#include <stb_image_write.h>
#include <vector>
#include <iostream>

Film::Film(const Size2i &size) : m_Size(size)
{
    m_Pixels        = std::unique_ptr<Pixel[]>(new Pixel[size.x * size.y]);
    m_ColorsUchar   = 
        std::unique_ptr<std::vector<Color3b>>(new std::vector<Color3b>(size.x * size.y));
}

void Film::Save(const std::string &filePath)
{
    auto cb = GetColorsUchar();
    stbi_flip_vertically_on_write(true);
    int success = stbi_write_bmp( filePath.c_str(), m_Size.x, m_Size.y, 3, cb );
    if (success)
    {
        LOG_INFO("image saved: {}", filePath);
    }
    else
    {
        LOG_INFO("failed to save image: {}", filePath);
    }
}

Pixel &Film::GetPixel(const Point2i &location)
{
    int ind = location.y * m_Size.x + location.x;
    return m_Pixels[ind];
}

void Film::AddSample(const Point2i &location, const Vec3f &rgb)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto &pixel = GetPixel(location);
    pixel.rgb += rgb;
    pixel.weight += 1.f;
}

const Color3b *Film::GetColorsUchar() const
{
    float gamma = 1.f / 2.2f;
    for (int i = 0; i < m_Size.x * m_Size.y; i ++)
    {
        auto rgb = m_Pixels[i].rgb / m_Pixels[i].weight;
        rgb = glm::pow(glm::clamp(rgb, Vec3f(0), Vec3f(1)), Vec3f(gamma));
        (*m_ColorsUchar)[i] = rgb * 255.99f;
    }
    return m_ColorsUchar->data();
}
