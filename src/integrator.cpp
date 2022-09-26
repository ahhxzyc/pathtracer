#include "integrator.h"
#include "scene.h"
#include "Log.h"
#include <fstream>
#include <iomanip>

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
        if (!is)
            break;

        is->BuildBSDF();
        auto &bsdf = is->bsdf;
        bool perfectDelta = (bsdf.num_components(BSDF_Delta) == bsdf.num_components(BSDF_All));

        // direct path from eye to light
        auto mat = is->primitive->GetMaterial();
        if (bounces == 0 && mat->IsEmissive())
        {
            L += mat->ke;
        }

        // sample from lights
        if (!perfectDelta)
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
                auto weight = power_heuristic(sample.pdf / invPdf, bsdf.Pdf(sample.wi));
                L += beta * currentBeta * sample.Le * weight ;
            }
        }

        // sample BSDF
        auto scatterSample = bsdf.Sample();
        if (scatterSample.pdf == 0.f)
            break;
        Ray ray(is->point, scatterSample.wi);
        auto nextIsec = scene.accel->intersect(ray);
        if (!nextIsec)
            break;

        // accumulate path throughput
        beta *= scatterSample.f * std::abs(glm::dot(is->normal, scatterSample.wi)) / scatterSample.pdf;

        // sample contribution
        auto light = nextIsec->primitive->GetAreaLight();
        if (light && !nextIsec->backface)
        {
            if (scatterSample.isDelta)
            {
                L += beta * light->radiance;
            }
            else
            {
                auto lightPdf = light->Pdf(is->point, nextIsec->point, nextIsec->normal) / float(scene.lights.size());
                auto weight = power_heuristic(scatterSample.pdf, lightPdf);
                L += beta * light->radiance * weight;
            }
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

    //==============================================================//
    //                                                              //
    //                    Direct lighting only                      //
    //                                                              //
    //==============================================================//

    //Color3f L = incidentRadiance_;

    //auto is = scene.accel->intersect(Ray(ray));
    //if (!is || is->backface)
    //    return L;

    //is->BuildBSDF();
    //auto &bsdf = is->bsdf;

    //// sample bsdf
    //auto sample = bsdf.Sample();
    //if (sample.pdf == 0.f)
    //    return L;

    //// direct lighting
    //float cosTheta = glm::dot(is->normal, sample.wi);
    //auto f = bsdf.Eval(sample.wi);
    //L = f * incidentRadiance_ * cosTheta / sample.pdf;

    //return L;


    //==============================================================//
    //                                                              //
    //                    Path tracing with MIS                     //
    //                                                              //
    //==============================================================//

    Color3f L = Color3f(0.f);

    Color3f beta(1.f);
    auto is = scene.accel->intersect(Ray(ray));
    for (int bounces = 0; ; bounces ++)
    {
        if (!is)
        {
            if (bounces == 0)
                L += incidentRadiance_;
            break;
        }

        is->BuildBSDF();
        auto &bsdf = is->bsdf;

        // sample from lights
        {
            // uniformly sample hemisphere
            float cosTheta = 1.f - rand01();
            float sinTheta = std::sqrt(1.f - cosTheta * cosTheta);
            float phi = 2.f * PI * rand01();
            Vec3f wi(sinTheta * std::sin(phi), sinTheta * std::cos(phi), cosTheta);
            wi = CoordinateSystem(is->normal).ToWorld(wi);
            float pdf = 1.f / (2.f * PI);

            // is light visible ? 
            if (!scene.accel->has_intersection(Ray(is->point, wi)))
            {
                auto currentBeta = bsdf.Eval(wi) * glm::dot(is->normal, wi) / pdf;
                auto weight = power_heuristic(pdf, bsdf.Pdf(wi));
                L += beta * currentBeta * incidentRadiance_ * weight;
            }
        }

        // sample BSDF
        auto scatterSample = bsdf.Sample();
        if (scatterSample.pdf == 0.f)
            break;
        Ray ray(is->point, scatterSample.wi);
        auto nextIsec = scene.accel->intersect(ray);

        // accumulate path throughput
        beta *= bsdf.Eval(scatterSample.wi) * glm::dot(is->normal, scatterSample.wi) / scatterSample.pdf;

        // sample hit light ?
        if (!nextIsec)
        {
            auto lightPdf = 1.f / (2.f * PI);
            auto weight = power_heuristic(scatterSample.pdf, lightPdf);
            L += beta * incidentRadiance_ * weight;
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

    return L;
}
