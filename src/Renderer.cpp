#include "Renderer.h"
#include "Triangle.h"
#include "Utils.h"

#include <glm/gtx/norm.hpp>

#include <stb_image_write.h>

#include <iostream>
#include <fstream>

using namespace std;


Renderer::Renderer(Scene *scene) : m_scene(scene)
{
    m_Film = new Film({m_scene->m_width, m_scene->m_height});
}

Renderer::~Renderer()
{
    delete m_Film;
}

auto balance_heuristic(float p1, float p2)
{
    return std::make_tuple(p1/(p1+p2), p2/(p1+p2));
}
auto power_heuristic(float p1, float p2)
{
    p1 *= p1; p2 *= p2;
    return balance_heuristic(p1, p2);
}


void Renderer::render()
{
    auto w = m_scene->m_width;
    auto h = m_scene->m_height;

    for (int rid = 0; rid < m_spp; rid ++)
    {
#pragma omp parallel for
        for (int pix = 0; pix < w * h; pix ++)
        {
            auto wi = pix % w, hi = pix / w;
            auto color = trace01(m_scene->getRay(wi, hi), m_max_depth);
            m_Film->AddSample({wi, hi}, color);
        }
        printf("rid: %d\n", rid);
        if (rid % 2 == 0)
        {
            string name = "output_" + to_string(rid) + ".bmp";
            m_Film->Save(name);
        }
    }
}

Vec3f Renderer::trace01(Ray ray, int depth)
{
    //// find intersection
    //Intersection inter;
    //m_scene->intersect(ray);
    //if (!inter.yes)
    //{
    //    return Vec3f(0,0,0);
    //}

    //Vec3f Lo = Vec3f(0,0,0);
    //if (depth <= 0)
    //    return Lo;

    //// direct lighting, sampling on light sources
    //{
    //    auto [sam, pdf] = sample_light_source();
    //    Ray r(
    //            inter.point,
    //            samp.point - inter.point
    //    );
    //    Intersection is;
    //    m_scene->intersect(r);
    //    if (is.yes && almostZero(samp.point - is.point) && r.dir.dot(is.normal) < 0.f)
    //    {
    //        Vec3f fr = brdf(r.dir, -ray.dir, inter);
    //        float cos = r.dir.dot(inter.normal);
    //        float cos_i = (-r.dir).dot(samp.normal);
    //        Vec3f diff = samp.point - inter.point;
    //        Lo += fr.cwiseProduct(samp.material->ke)
    //                * cos * cos_i / diff.dot(diff) / pdf;
    //    }
    //}

    //// indirect lighting, cosine weighted sampling on hemisphere
    //{
    //    float pdf;
    //    Vec3f dir = sample_hemisphere(inter.normal, pdf);
    //    Ray nray(inter.point, dir);
    //    Vec3f Li = trace01(nray, depth - 1);
    //    Vec3f fr = brdf(nray.dir, -ray.dir, inter);
    //    float cos = nray.dir.dot(inter.normal);
    //    Lo += fr.cwiseProduct(Li) * cos / pdf;
    //}

    //return Lo;

    auto is = m_scene->intersect(ray);

    // Hit background or hit back face
    if (!is.yes || is.is_back)
    {
        return Vec3f(0, 0, 0);
    }
    return is.normal * 0.5f + Vec3f(0.5f);

    //Vec3f Lo(0,0,0);
    //Lo = is.material->ke;

    //float termination_p = 0.2f;
    //if (rand01() < termination_p)
    //{
    //    return Lo;
    //}

    // Perform the 2 sampling strategies on separate parts of the lighting
    //{
    //    // Direct lighting, sampling on light sources
    //    auto [sam, pdf] = sample_light_source();
    //    Ray ri(is.point, sam - is.point);
    //    auto lis = m_scene->intersect(ri);
    //    if (lis.yes && !lis.is_back && almostZero(sam - lis.point))
    //    {
    //        Vec3f fr = is.brdf(ri.dir, -ray.dir);
    //        float cos = abs_dot(ri.dir, is.normal, 0.001f);
    //        float cos_l = abs_dot(-ri.dir, lis.normal, 0.001f);
    //        float R2 = glm::length2(sam - is.point);
    //        Lo += fr * lis.material->ke * cos * cos_l / R2 / pdf;
    //        // ref: Dartmouth slides 14-MIS, one sample case
    //        // https://canvas.dartmouth.edu/courses/35073/files/folder/Slides
    //    }
    //}
    //{
    //    // Indirect lighting, cosine weighted sampling on hemisphere
    //    float pdf;
    //    Vec3f dir = sample_hemisphere(is.normal, pdf);
    //    Ray ri(is.point, dir);
    //    Vec3f Li = trace01(ri, depth - 1);
    //    Vec3f fr = is.brdf(ri.dir, -ray.dir);
    //    float cos = abs_dot(ri.dir, is.normal);
    //    Lo += fr * Li * cos / pdf;
    //}
    //return Lo / (1.f - termination_p);
}

Vec3f Renderer::trace_balanced(Ray ray, int depth)
{
    // find intersection
    Intersection is;
    m_scene->intersect(ray);

    // Hit background or hit back face
    if (!is.yes || is.is_back)
    {
        return Vec3f(0, 0, 0);
    }

    auto Lo = is.material->ke;
    
    // Run out of hits
    if (depth <= 0)
        return Lo;

    // Decide sampling strategy
    float p_sample_light = 0.5f;
    if (rand01() < p_sample_light)
    {
        // Direct lighting, sampling on light sources
        auto [sam, pdf] = sample_light_source();
        Ray ri( is.point, sam - is.point );
        Intersection lis;
        m_scene->intersect(ri);
        if (lis.yes && !lis.is_back && almostZero(sam - lis.point))
        {
            Vec3f fr = is.brdf(ri.dir, -ray.dir);
            float cos = abs_dot(ri.dir, is.normal, 0.001f);
            float cos_l = abs_dot(-ri.dir, lis.normal, 0.001f);
            float R2 = glm::length2(sam - is.point);
            float w_l = p_sample_light * pdf * R2 / cos_l;
            float w_b = (1 - p_sample_light) * cos_hemisphere_pdf(glm::normalize(sam - is.point), is.normal);
            Lo += fr * lis.material->ke * cos / (w_l + w_b);
            // ref: Dartmouth slides 14-MIS, one sample case
            // https://canvas.dartmouth.edu/courses/35073/files/folder/Slides
        }
    }
    else
    {
        // Indirect lighting, cosine weighted sampling on hemisphere
        float pdf;
        Vec3f dir = sample_hemisphere(is.normal, pdf);
        Ray ri(is.point, dir);
        Vec3f Li = trace_balanced(ri, depth - 1);
        Vec3f fr = is.brdf(ri.dir, -ray.dir);
        float cos = abs_dot(ri.dir, is.normal);
        float w_l = 0.f;
        auto p_l = light_pdf(ri);
        if (p_l)
        {
            w_l = p_sample_light * *p_l;
        }
        float w_b = (1 - p_sample_light) * pdf;
        Lo += fr * Li * cos / (w_l + w_b);
    }
    return Lo;
}


// bool Renderer::brdf_importance_sampling(Vec3f normal, Vec3f wo, float ns, Vec3f &res) {
//     Vec3f dir = normal;
//     // for specular cases, sample around mirror reflection direction
//     if (ns > 1.01) {
//         Vec3f proj = wo.dot(normal) * wo;
//         dir = 2 * proj - wo;
//     }
//     // sampling
//     float phi = rand01() * 2 * PI;
//     float theta = acos(pow(rand01(), 1 / (ns + 1)));
//     Vec3f v(
//         sin(theta) * cos(phi),
//         sin(theta) * sin(phi),
//         cos(theta)
//     );
//     // basis of local coordinate system
//     Vec3f front;
//     if (fabs(dir[0]) > fabs(dir[1])) {
//         front = Vec3f(dir[2], 0, -dir[0]).normalized();
//     } else {
//         front = Vec3f(0, dir[2], -dir[1]).normalized();
//     }
//     Vec3f right = front.cross(dir);
//     res = (v[0]*right + v[1]*front + v[2]*dir).normalized();
//     // see if it points below the hemisphere
//     return res.dot(normal) > 0.f;
// }


Vec3f Renderer::sample_hemisphere(Vec3f normal, float &pdf) const {
    // local coordinate
    Vec3f front;
    if (fabs(normal[0]) > fabs(normal[1])) {
        front = glm::normalize(Vec3f(normal[2], 0, -normal[0]));
    } else {
        front = glm::normalize(Vec3f(0, normal[2], -normal[1]));
    }
    Vec3f right = glm::cross(front, normal);
    // cosine weighted sampling
    float phi = rand01() * 2 * PI;
    float theta = 0.5f * acos(1 - 2 * rand01());
    Vec3f v(
        sin(theta) * cos(phi),
        sin(theta) * sin(phi),
        cos(theta)
    );
    Vec3f dir = glm::normalize(v[0]*right + v[1]*front + v[2]*normal);
    pdf = fabs(v[2]) / PI;
    return dir;
}

//void Renderer::save(const string &filepath) const {
//    // convert to uchar
//    vector<unsigned char> colorsUchar;
//    auto fp = reinterpret_cast<const float*>(fb.data());
//    auto cnt = fb.size() * 3;
//    for (int i = 0; i < cnt; i ++ ) {
//        auto f = clamp(0, 1, std::pow(fp[i], 1.f/2.2f));
//        colorsUchar.push_back((unsigned char)(f * 255.99));
//    }
//    // save as bmp file
//    stbi_flip_vertically_on_write(true);
//    int success = stbi_write_bmp(
//            filepath.c_str(),
//            m_scene->m_width,
//            m_scene->m_height, 
//            3, 
//            colorsUchar.data()
//    );
//    if (success) {
//        cout << "image saved: " << filepath << endl;
//    } else {
//        cout << "failed to save image " << filepath << endl;
//    }
//}


std::tuple<Vec3f,float> Renderer::sample_light_source()
{
    float area_sum = 0.f;
    for (int idx : m_scene->m_light_ids)
        area_sum += m_scene->m_tris[idx].area();
    // Pick light source with probability proportional to area
    float r = rand01();
    float t_sum = 0.f;
    for (int idx : m_scene->m_light_ids) {
        const auto &tri = m_scene->m_tris[idx];
        t_sum += tri.area() / area_sum;
        if (r < t_sum) {
            // uniform sampling inside the triangle
            // ref: https://jsfiddle.net/jniac/fmx8bz9y/
            float u = rand01(),  v = rand01();
            if (u + v > 1)
            {
                u = 1 - u;
                v = 1 - v;
            }
            auto P = (1-u-v)*tri.p[0] + u*tri.p[1] + v*tri.p[2];
            float pdf = 1.f / area_sum;
            return {P, pdf};
        }
    }
    return {Vec3f(0,0,0), 0.f};
}

std::optional<float> Renderer::light_pdf(const Ray& ray)
{
    float ret = 0.f;
    Intersection tis;
    m_scene->intersect(ray);
    if (tis.yes && !tis.is_back && tis.tri->mMaterial->is_emissive())
    {
        float area_sum = 0.f;
        for (auto ind : m_scene->m_light_ids)
            area_sum += m_scene->m_tris[ind].area();
        return {1.f / area_sum};
    }
    return {};
}

float Renderer::cos_hemisphere_pdf(const Vec3f& dir, const Vec3f &normal)
{
    float cos = glm::dot(dir, normal);
    return cos * std::sqrt(1.f-cos*cos) / PI;
}
