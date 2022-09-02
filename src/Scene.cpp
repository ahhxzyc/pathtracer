#include "Scene.h"
#include "Utils.h"
#include "Log.h"

#include <iostream>


#include <tiny_obj_loader.h>

Scene::Scene()  {
}

Scene::~Scene() {

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

    // Get geometry data from loader
    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();
    
    // Register the materials
    std::vector<std::shared_ptr<Material>> mats;
    for (auto &tmat : materials)
    {
        mats.push_back(std::make_shared<Material>());
        auto &mat = mats.back();
        if (tmat.diffuse_texname.empty())
            mat->kd_map = std::make_shared<Texture>(Vec3f(tmat.diffuse[0], tmat.diffuse[1], tmat.diffuse[2]));
        else
            mat->kd_map = std::make_shared<Texture>(reader_config.mtl_search_path + "/" + tmat.diffuse_texname);
        mat->ke = Vec3f(tmat.emission[0], tmat.emission[1], tmat.emission[2]);
    }

    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces and save to the scene
        size_t index_offset = 0;
        auto num_faces = shapes[s].mesh.num_face_vertices.size();
        m_Primitives.reserve(m_Primitives.size() + num_faces);

        for (size_t f = 0; f < num_faces; f++)
        {
            // Get the material of the face
            auto mid = shapes[s].mesh.material_ids[f];
            if (mid < 0 || mid >= mats.size())
            {
                LOG_ERROR("Invalid material ID");
                break;
            }
            auto &material = mats[mid];

            // Create the face
            auto &ptr = m_Primitives.emplace_back();
            ptr = std::make_shared<Triangle>(material);
            auto &tri = *std::static_pointer_cast<Triangle>(ptr);

            // Register an area light if there is one
            if (material->IsEmissive())
            {
                LOG_INFO("Found an emissive face");
                auto light = std::make_shared<AreaLight>(tri, material->ke);
                tri.SetAreaLight(light);
                lights.push_back(light);
            }

            // Save vertex information
            int fv = shapes[s].mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                tri.p[v].x = attrib.vertices[3 * idx.vertex_index + 0];
                tri.p[v].y = attrib.vertices[3 * idx.vertex_index + 1];
                tri.p[v].z = attrib.vertices[3 * idx.vertex_index + 2];

                tri.n[v].x = attrib.normals[3 * idx.vertex_index + 0];
                tri.n[v].y = attrib.normals[3 * idx.vertex_index + 1];
                tri.n[v].z = attrib.normals[3 * idx.vertex_index + 2];

                tri.t[v].x = attrib.texcoords[2 * idx.texcoord_index + 0];
                tri.t[v].y = attrib.texcoords[2 * idx.texcoord_index + 1];
            }

            index_offset += fv;
        }
    }
}

void Scene::BuildAggregate()
{
    m_Aggregate = std::make_shared<BVHAggregate>(m_Primitives);
}
