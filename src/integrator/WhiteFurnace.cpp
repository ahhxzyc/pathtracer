#include "integrator/WhiteFurnace.h"
#include "Utils.h"


WhiteFurnaceIntegrator::WhiteFurnaceIntegrator(const std::shared_ptr<Camera> &camera)
    :Integrator(camera)
{

}

Color3f WhiteFurnaceIntegrator::Li(const Ray &ray, const Scene &scene, int depth /*= 0*/)
{
    Color3f L = Color3f(0.f);

    auto is = scene.GetAggregate().Intersect(ray, 0.001f, std::numeric_limits<float>::max());
    if (is && !is->backface)
    {
        is->BuildBSDF();
        is->bsdf.unify_reflectance();

        auto sample = is->bsdf.Sample();
        L += is->bsdf.Eval(sample.wi) * abs_dot(is->normal, sample.wi) / sample.pdf;
    }
    return L;
}
