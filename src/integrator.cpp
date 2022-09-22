#include "integrator.h"
#include "scene.h"
#include "Log.h"

void Integrator::render(const Scene &scene)
{
    auto &camera = scene.camera;
    auto size = camera->film->size;
#if !defined(DEBUG) & !defined(_DEBUG)
#pragma omp parallel for
#endif
    for (int pix = 0; pix < size.x * size.y; pix ++)
    {
        auto wi = pix % size.x, hi = pix / size.x;
        auto color = radiance(camera->spawn_ray({ wi, hi }), scene, 6);
        camera->film->add_sample({ wi, hi }, color);
    }
}


Color3f PathIntegrator::radiance(const Ray& ray, const Scene &scene, int depth /*= 0*/)
{
    Color3f L = Color3f(0.f);

    Color3f beta(1.f);
    auto is = scene.accel->intersect(Ray(ray));
    for (int bounces = 0; ; bounces ++ )
    {
        if (!is || is->backface)
            break;
        is->BuildBSDF();
        auto &bsdf = is->bsdf;

        // direct path from eye to light
        auto mat = is->primitive->GetMaterial();
        if (bounces == 0 && mat->IsEmissive())
        {
            L += mat->ke;
        }

        // sample from lights
        {
            // uniformly choose a light
            auto u = rand01();
            int n = scene.lights.size();
            auto index = std::min(static_cast<int>(u * n), n - 1);
            auto invPdf = static_cast<float>(n);
            auto &light = *scene.lights[index];

            // sample the light
            auto sample = light.Sample(*is);
            if (sample.pdf != 0.f && !scene.accel->has_intersection(Ray::between(is->point, sample.point)))
            {
                auto currentBeta = bsdf.Eval(sample.wi) * glm::dot(is->normal, sample.wi) / sample.pdf * invPdf;
                auto weight = power_heuristic(sample.pdf, bsdf.Pdf(sample.wi));
                L += beta * currentBeta * sample.Le * weight ;
            }
        }

        // sample BSDF
        auto scatterSample = bsdf.Sample();
        if (scatterSample.pdf == 0.f)
            break;
        Ray ray(is->point, scatterSample.wi);
        auto nextIsec = scene.accel->intersect(ray);
        if (!nextIsec || nextIsec->backface)
            break;

        // accumulate path throughput
        beta *= bsdf.Eval(scatterSample.wi) * glm::dot(is->normal, scatterSample.wi) / scatterSample.pdf;

        // sample contribution
        auto light = nextIsec->primitive->GetAreaLight();
        if (light)
        {
            auto lightPdf = light->Pdf(is->point, nextIsec->point, nextIsec->normal) / float(scene.lights.size());
            auto weight = power_heuristic(scatterSample.pdf, lightPdf);
            L += beta * light->Radiance(ray) * weight;
        }

        is = nextIsec;

        // Russian roulette
        if (bounces > 3)
        {
            auto q = std::min(std::max(std::max(beta.x, beta.y), beta.z), 0.95f);
            if (rand01() > q)
                break;
            beta /= q;
        }
    }

    if (std::isnan(L.x) || std::isnan(L.y) || std::isnan(L.z))
    {
        LOG_INFO("nan");
    }

    return L;
}


float balance_heuristic(float pdf1, float pdf2)
{
    auto sum = pdf1 + pdf2;
    return sum == 0.f ? 0.f : pdf1 / sum;
}

float power_heuristic(float pdf1, float pdf2)
{
    auto sum = pdf1*pdf1 + pdf2*pdf2;
    return sum == 0.f ? 0.f : pdf1 * pdf1 / sum;
}

WhiteFurnaceIntegrator::WhiteFurnaceIntegrator(const Color3f &rad)
    : incidentRadiance_(rad)
{

}

Color3f WhiteFurnaceIntegrator::radiance(const Ray &ray, const Scene &scene, int depth /*= 0*/)
{
    Color3f L = incidentRadiance_;

    auto is = scene.accel->intersect(Ray(ray));
    if (!is || is->backface)
        return L;

    is->BuildBSDF();
    auto &bsdf = is->bsdf;

    // sample bsdf
    auto sample = bsdf.Sample();
    if (sample.pdf == 0.f)
        return L;

    // direct lighting
    float cosTheta = glm::dot(is->normal, sample.wi);
    auto f = bsdf.Eval(sample.wi);
    L = f * incidentRadiance_ * cosTheta / sample.pdf;

    return L;
}
