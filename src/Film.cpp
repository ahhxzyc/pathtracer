#include "Film.h"

#include <stb_image_write.h>
#include <vector>
#include <iostream>

Film::Film(const Size2i &size) : m_Size(size)
{
    m_Pixels = std::unique_ptr<Pixel[]>(new Pixel[size.x * size.y]);
}

void Film::Save(const std::string &filePath)
{
    std::vector<unsigned char> colorsUchar(m_Size.x * m_Size.y * 3);
    float gamma = 1.f / 2.2f;
    for (int i = 0; i < m_Size.x * m_Size.y; i ++ )
    {
        auto rgb = m_Pixels[i].rgb / m_Pixels[i].weight;
        rgb = glm::pow(glm::clamp(rgb, Vec3f(0), Vec3f(1)), Vec3f(gamma));
        colorsUchar[i * 3 + 0] = rgb.r * 255.99f;
        colorsUchar[i * 3 + 1] = rgb.g * 255.99f;
        colorsUchar[i * 3 + 2] = rgb.b * 255.99f;
    }
    // save as bmp file
    stbi_flip_vertically_on_write(true);
    int success = stbi_write_bmp( filePath.c_str(), m_Size.x, m_Size.y, 3, colorsUchar.data() );
    if (success)
    {
        std::cout << "image saved: " << filePath << std::endl;
    }
    else
    {
        std::cout << "failed to save image " << filePath << std::endl;
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
