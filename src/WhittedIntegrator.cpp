#include "WhittedIntegrator.h"



WhittedIntegrator::WhittedIntegrator(const std::shared_ptr<Camera> &camera)
    : Integrator(camera)
{

}

Color3f WhittedIntegrator::Li(const Ray &ray, const Scene &scene)
{
    auto is = scene.intersect(ray);

    // Hit background or hit back face
    if (!is.yes || is.backface)
    {
        return Vec3f(0, 0, 0);
    }

    return is.normal * 0.5f + Vec3f(0.5f);
}
