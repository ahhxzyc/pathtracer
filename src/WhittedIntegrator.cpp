#include "WhittedIntegrator.h"
#include "Utils.h"


WhittedIntegrator::WhittedIntegrator(const std::shared_ptr<Camera> &camera)
    : Integrator(camera)
{

}

Color3f WhittedIntegrator::Li(const Ray &ray, const Scene &scene, int depth)
{
    auto is = scene.GetAggregate().Intersect(ray, 0.001f, std::numeric_limits<float>::max());

    Color3f Lo = Color3f(0.f);
    if (!is || is->backface)
    {
        return Vec3f(0, 0, 0);
    }

    // Directly hits an area light
    auto areaLight = is->primitive->GetAreaLight();
    if (areaLight)
    {
        Lo += areaLight->L();
    }

    // Construct surface BSDF
    is->BuildBSDF();
    auto &bsdf = is->bsdf;

    // Add contribution of direct lighting
    for (const auto &light : scene.lights)
    {
        auto sample = light->Sample(is->point);
        float tmax = glm::length(sample.point - is->point) - 0.0001f;
        if (!scene.GetAggregate().ExistIntersection({ is->point, sample.direction }, 0.001f, tmax))
        {
            Lo += sample.radiance * bsdf.Eval({},{}) * abs_dot(sample.direction, is->normal, 0.f) / sample.pdf;
            return Lo;
        }
    }

    // Add indirect lighting coming in specular reflection direction
    if (depth > 0)
    {
        auto reflectDir = ray.dir + is->normal * glm::dot(-ray.dir, is->normal) * 2.f;
        auto L = Li({ is->point, reflectDir }, scene, depth - 1);
        Lo += L * bsdf.Eval({}, {}) * abs_dot(reflectDir, is->normal);
    }
    
    return Lo;
}
