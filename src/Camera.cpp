#include "Camera.h"
#include "Film.h"
#include "Utils.h"

Camera::Camera(const Size2i &size) : film(std::make_shared<Film>(size))
{

}

Ray Camera::GetRay(const Point2i loc) const
{
    float h = std::tan(m_fov * PI / 180.f * 0.5f) * 2.f;
    auto size = film->Size();
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
