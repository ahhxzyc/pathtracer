#include "Renderer.h"
#include "Triangle.h"
#include "Utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <iostream>
using namespace std;


Renderer::Renderer(Scene *scene) : m_scene(scene) {
    // create frame buffer
    fb.resize(scene->m_width * scene->m_height);
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
            auto color = trace(m_scene->getRay(wi, hi), max_depth);
            fb[pix] = (fb[pix] * rid + color) / static_cast<float>(rid + 1);
        }
        printf("rid: %d\n", rid);
        if (1)
        {
            string name = "output_" + to_string(rid) + ".bmp";
            save(name);
        }
    }
}

Vec3f Renderer::trace(Ray ray, int depth)
{
    // find intersection
    Intersection inter;
    m_scene->intersect(ray, inter);
    if (!inter.yes)
    {
        //Skybox *ps = m_scene->m_skybox;
        //if (ps)
        //    return ps->get_color(ray);
        //else
        //    return Vec3f(0, 0, 0);
        return Vec3f(0);
    }
    auto Le = inter.material.ke;
    if (!depth)
        return Le;

    // direct lighting, sampling on light sources
    Vec3f l_dir(0,0,0);
    {
        float pdf;
        Intersection p_inter;
        p_inter = sample_light_source(pdf);
        Ray r(
                inter.point,
                p_inter.point - inter.point
        );
        Intersection t_inter;
        m_scene->intersect(r, t_inter);
        if (t_inter.yes && almostZero(p_inter.point - t_inter.point)) {
            // integrate on light source
            Vec3f fr = brdf(r.dir, -ray.dir, inter);
            float cos = r.dir.dot(inter.normal);
            float cos_i = (-r.dir).dot(p_inter.normal);
            Vec3f diff = p_inter.point - inter.point;
            l_dir = 
                    p_inter.material.ke
                    .cwiseProduct(fr)
                    * cos * cos_i / diff.dot(diff)
                    / pdf;
        }
    }

    // indirect lighting, sampling on hemisphere
    Vec3f l_indir(0,0,0);
    {
        // cosine weighted sampling on hemisphere
        // so pdf is actually useless
        float pdf;
        Vec3f dir = sample_hemisphere(inter.normal, pdf);
        Ray nray(inter.point, dir);
        Vec3f l_i = trace(nray, depth + 1);
        Vec3f fr = brdf(nray.dir, -ray.dir, inter);
        float cos = nray.dir.dot(inter.normal);
        l_indir = 
                l_i.cwiseProduct(fr) 
                * cos
                / pdf;
    }
    
    return Le + l_dir + l_indir;
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
        front = Vec3f(normal[2], 0, -normal[0]).normalized();
    } else {
        front = Vec3f(0, normal[2], -normal[1]).normalized();
    }
    Vec3f right = front.cross(normal);
    // cosine weighted sampling
    float phi = rand01() * 2 * PI;
    float theta = 0.5f * acos(1 - 2 * rand01());
    Vec3f v(
        sin(theta) * cos(phi),
        sin(theta) * sin(phi),
        cos(theta)
    );
    Vec3f dir = (v[0]*right + v[1]*front + v[2]*normal).normalized();
    pdf = fabs(v[2]) / PI;
    return dir;
}


Vec3f Renderer::brdf(Vec3f wi, Vec3f wo, Intersection inter) const {
    if (wi.dot(inter.normal) < 0.f || wo.dot(inter.normal) < 0.f)
        return Vec3f(0,0,0);
    return inter.material.kd / PI;
}

void Renderer::save(const string &filepath) const {
    // convert to uchar
    vector<unsigned char> colorsUchar;
    auto fp = reinterpret_cast<const float*>(fb.data());
    auto cnt = fb.size() * 3;
    for (int i = 0; i < cnt; i ++ ) {
        auto f = clamp(0, 1, std::pow(fp[i], 1.f/2.2f));
        colorsUchar.push_back((unsigned char)(f * 255));
    }
    // save as bmp file
    stbi_flip_vertically_on_write(true);
    int success = stbi_write_bmp(
            filepath.c_str(),
            m_scene->m_width,
            m_scene->m_height, 
            3, 
            colorsUchar.data()
    );
    if (success) {
        cout << "image saved: " << filepath << endl;
    } else {
        cout << "failed to save image " << filepath << endl;
    }
}


Intersection Renderer::sample_light_source(float &pdf) {
    float area_sum = 0.f;
    for (int idx : m_scene->m_light_ids) {
        const Triangle &tri = m_scene->m_tris[idx];
        area_sum += tri.area();
    }
    // pick a light source
    float r = rand01();
    float t_sum = 0.f;
    Intersection p;
    pdf = 0.f;
    for (int idx : m_scene->m_light_ids) {
        const Triangle &tri = m_scene->m_tris[idx];
        t_sum += tri.area() / area_sum;
        if (r < t_sum) {
            // uniform sampling inside the triangle
            // ref: https://math.stackexchange.com/questions/18686/uniform-random-point-in-triangle-in-3d
            float u = sqrtf(rand01());
            float v = rand01();
            p.point = (1-u)*tri.p[0]
                    + u*(1-v)*tri.p[1]
                    + v*u*tri.p[2];
            p.material = tri.mMaterial;
            p.normal = tri.normal();
            pdf = 1 / area_sum;
            break;
        }
    }
    return p;
}