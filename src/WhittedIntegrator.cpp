#include "WhittedIntegrator.h"



WhittedIntegrator::WhittedIntegrator(const std::shared_ptr<Camera> &camera)
    : Integrator(camera)
{

}

Color3f WhittedIntegrator::Li(const Ray &ray, const Scene &scene)
{
    Color3f Lo = Color3f(0.f);

    auto is = scene.GetAggregate().Intersect(ray, 0.001f, std::numeric_limits<float>::max());

    // Hit background or hit back face
    if (!is || is->backface)
    {
        return Vec3f(0, 0, 0);
    }

    return is->normal * 0.5f + Vec3f(0.5);
}
