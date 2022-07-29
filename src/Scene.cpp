#include "Scene.h"
#include "Utils.h"

#include <iostream>


#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Scene::Scene(int w, int h) : m_width(w), m_height(h) {
    m_front = (m_lookat - m_eye).normalized();
    m_right = m_front.cross(m_up);
}

Scene::~Scene() {

}



void Scene::init_octtree() {
    m_octtree = new OctTree(m_tris);
}



void Scene::addModel(const string &filepath) {
    std::string inputfile = filepath;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "E:/vscodedev/ptracer/res/cornell-box";

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }
    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Loop over shapes
    vector<Vec3f> verts;
    vector<Vec3f> norms;
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                verts.emplace_back(
                        attrib.vertices[3*idx.vertex_index+0],
                        attrib.vertices[3*idx.vertex_index+1],
                        attrib.vertices[3*idx.vertex_index+2]);
                norms.emplace_back(
                        attrib.normals[3*idx.vertex_index+0],
                        attrib.normals[3*idx.vertex_index+1],
                        attrib.normals[3*idx.vertex_index+2]);
            }

            // create triangle
            Triangle tri({
                    verts[verts.size() - 3],
                    verts[verts.size() - 2],
                    verts[verts.size() - 1]
            });
            tri.n[0] = norms[norms.size() - 3];
            tri.n[1] = norms[norms.size() - 2];
            tri.n[2] = norms[norms.size() - 1];
            // bind material
            Material &mat = tri.mMaterial;
            auto &tmat = materials[shapes[s].mesh.material_ids[f]];
            mat.ka = Vec3f(tmat.ambient[0], tmat.ambient[1], tmat.ambient[2]);
            mat.kd = Vec3f(tmat.diffuse[0], tmat.diffuse[1], tmat.diffuse[2]);
            mat.ks = Vec3f(tmat.specular[0], tmat.specular[1], tmat.specular[2]);
            mat.ke = Vec3f(tmat.emission[0], tmat.emission[1], tmat.emission[2]);

            m_tris.emplace_back(tri);
            if (tri.mMaterial.ke.norm() > 0.01f)
                m_light_ids.push_back(m_tris.size() - 1);
            index_offset += fv;
        }
    }
}



void Scene::addSkybox(const string &filepath) {
    m_skybox = new Skybox(filepath);
    if (!m_skybox->m_valid) {
        delete m_skybox;
        m_skybox = nullptr;
    }
}


void Scene::intersect(Ray ray, Intersection &inter) {
    int cnt = 0;
    m_octtree->intersect(ray, inter, cnt);
}

Ray Scene::getRay(int x, int y) {
    // angle range
    float rt = m_fov * PI / 180.f;
    float d = rt / m_height;
    float rp = d * m_width;
    // pixel location
    float theta = ((float) y / m_height - 0.5f) * rt;
    float phi = ((float) x / m_width - 0.5f) * rp;
    // random ray
    theta += (rand01() - 0.5f) * d;
    phi += (rand01() - 0.5f) * d;
    Vec3f dir =
            m_right * cos(theta) * sin(phi)
            + m_up * sin(theta)
            + m_front * cos(theta) * cos(phi);
    return Ray(m_eye, dir);
}