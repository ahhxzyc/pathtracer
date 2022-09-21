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
    auto currentRay = ray;
    for (int bounces = 0; ; bounces ++ )
    {
        auto is = scene.accel->intersect(currentRay);
        if (!is || is->backface)
            break;
        is->BuildBSDF();

        // direct path from eye to light
        auto mat = is->primitive->GetMaterial();
        if (bounces == 0 && mat->IsEmissive())
        {
            L += mat->ke;
        }

        // estimate direct lighting
        L += beta * estimate_direct_group_stochastic(scene, *is, -ray.dir);

        // Sample for direction of next ray, accumulate path throughput
        auto sample = is->bsdf.Sample();
        currentRay = Ray(is->point, sample.wi);
        beta *= is->bsdf.Eval(sample.wi) * abs_dot(is->normal, sample.wi) / sample.pdf;

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


Color3f estimate_direct_all(const Scene& scene, Intersection &is, const Vec3f& wo)
{
    Color3f L(0.f);
    for (auto &light : scene.lights)
    {
        L += estimate_direct_single(scene, is, wo, *light);
    }
    return L;
}

Color3f estimate_direct_stochastic(const Scene &scene, Intersection &is, const Vec3f& wo)
{
    auto u = rand01();
    int n = scene.lights.size();
    auto index = std::min(static_cast<int>(u * n), n - 1);
    auto invPdf = static_cast<float>(n);
    return estimate_direct_single(scene, is, wo, *scene.lights[index]) * invPdf;
}

Color3f estimate_direct_single(const Scene &scene, Intersection &is, const Vec3f &wo, const Light& light)
{
    Color3f L(0.f);
    
    float lightWeight = 0.f;
    float bsdfWeight = 0.f;

    // sample light
    {
        auto sample = light.Sample(is.point);
        float tmax = glm::length(sample.point - is.point) - 0.0001f;
        if (!scene.accel->has_intersection({ is.point, sample.wi }))
        {
            auto transport = is.bsdf.Eval(sample.wi) * abs_dot(is.normal, sample.wi);
            auto weight = balance_heuristic(sample.pdf, is.bsdf.Pdf(sample.wi));
            //auto weight = 1.f;
            L += transport * sample.Le * weight / sample.pdf;

            lightWeight = weight;
        }
    }

    // sample BSDF
    {
        auto sample = is.bsdf.Sample();
        auto lightPdf = light.Pdf(is, sample.wi);
        Ray ray(is.point, sample.wi);
        auto lightIs = scene.accel->intersect(ray);
        if (lightIs && !lightIs->backface && lightIs->primitive->GetAreaLight() == &light)
        {
            auto transport = is.bsdf.Eval(sample.wi) * abs_dot(is.normal, sample.wi);
            auto weight = balance_heuristic(sample.pdf, lightPdf);
            L += transport * light.Radiance(ray) * weight / sample.pdf;

            bsdfWeight = weight;
        }
    }

    //auto sum = lightWeight + bsdfWeight;
    //if (sum > 0.001f)
    //{
    //    L = Color3f(1,0,0) + lightWeight/sum*Color3f(-1,1,0);
    //}

    return L;
}

Color3f estimate_direct_group_stochastic(const Scene &scene, Intersection &is, const Vec3f &wo)
{
    Color3f L(0.f);

    float lightWeight = 0.f;
    float bsdfWeight = 0.f;

    // sample from lights
    {
        // uniformly choose a light
        auto u = rand01();
        int n = scene.lights.size();
        auto index = std::min(static_cast<int>(u * n), n - 1);
        auto invPdf = static_cast<float>(n);
        auto &light = *scene.lights[index];

        // sample the light
        auto sample = light.Sample(is.point);
        float tmax = glm::length(sample.point - is.point) - 0.0001f;
        if (!scene.accel->has_intersection(Ray::between(is.point, sample.point)))
        {
            auto transport = is.bsdf.Eval(sample.wi) * abs_dot(is.normal, sample.wi);
            auto weight = balance_heuristic(sample.pdf, is.bsdf.Pdf(sample.wi));
            L += transport * sample.Le * weight / sample.pdf * invPdf;

            lightWeight = weight;
        }
    }

    // sample BSDF
    {
        // shoot a ray into the scene
        auto sample = is.bsdf.Sample();
        Ray ray(is.point, sample.wi);
        auto lightIs = scene.accel->intersect(ray);
        if (lightIs && !lightIs->backface && lightIs->primitive->GetAreaLight())
        {
            auto &light = *lightIs->primitive->GetAreaLight();
            auto transport = is.bsdf.Eval(sample.wi) * abs_dot(is.normal, sample.wi);
            auto lightPdf = light.Pdf(is, sample.wi);
            auto weight = balance_heuristic(sample.pdf, lightPdf);
            L += transport * light.Radiance(ray) * weight / sample.pdf;

            bsdfWeight = weight;
        }
    }

    return L;
}

float balance_heuristic(float pdf1, float pdf2)
{
    auto sum = pdf1 + pdf2;
    return sum > 0.001f ? pdf1 / sum : 0.f;
}
