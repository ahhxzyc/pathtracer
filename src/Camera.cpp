#include "camera.h"
#include "Log.h"
#include <stb_image_write.h>

Film::Film(const Size2i &size) : size(size)
{
    m_Pixels        = std::unique_ptr<Pixel[]>(new Pixel[size.x * size.y]);
    m_ColorsUchar   = 
        std::unique_ptr<std::vector<Color3b>>(new std::vector<Color3b>(size.x * size.y));
}

void Film::save(const std::string &filePath)
{
    auto cb = framebuffer_ub();
    stbi_flip_vertically_on_write(true);
    int success = stbi_write_bmp(filePath.c_str(), size.x, size.y, 3, cb);
    if (success)
    {
        LOG_INFO("image saved: {}", filePath);
    }
    else
    {
        LOG_INFO("failed to save image: {}", filePath);
    }
}

Pixel &Film::pixel_at(const Point2i &location)
{
    int ind = location.y * size.x + location.x;
    return m_Pixels[ind];
}

void Film::add_sample(const Point2i &location, const Vec3f &rgb)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto &pixel = pixel_at(location);
    pixel.rgb += rgb;
    pixel.weight += 1.f;
}

const Color3b *Film::framebuffer_ub() const
{
    float gamma = 1.f / 2.2f;
    for (int i = 0; i < size.x * size.y; i ++)
    {
        auto rgb = m_Pixels[i].rgb / m_Pixels[i].weight;
        rgb = glm::pow(glm::clamp(rgb, Vec3f(0), Vec3f(1)), Vec3f(gamma));
        (*m_ColorsUchar)[i] = rgb * 255.99f;
    }
    return m_ColorsUchar->data();
}


Camera::Camera(const Size2i &size) : 
    film(std::make_shared<Film>(size))
{

}

Ray Camera::spawn_ray(const Point2i loc) const
{
    float h = std::tan(m_fov * PI / 180.f * 0.5f) * 2.f;
    auto size = film->size;
    float asp = static_cast<float>(size.x) / size.y;
    Vec3f front = glm::normalize(m_lookat - m_eye);
    Vec3f right = glm::normalize(glm::cross(front, m_up));
    Vec3f ver = m_up * h;
    Vec3f hor = right * h * asp;
    float u = (static_cast<float>(loc.x) + rand01()) / size.x;
    float v = (static_cast<float>(loc.y) + rand01()) / size.y;
    Vec3f tar = m_eye + front + (u - 0.5f) * hor + (v - 0.5f) * ver;
    return Ray(m_eye, glm::normalize(tar - m_eye));
}
