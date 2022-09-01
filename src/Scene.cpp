#include "Scene.h"
#include "Utils.h"

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

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();
    
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
        // Loop over faces(polygon)
        size_t index_offset = 0;
        auto num_faces = shapes[s].mesh.num_face_vertices.size();
        m_Primitives.reserve(m_Primitives.size() + num_faces);

        for (size_t f = 0; f < num_faces; f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            auto &ptr = m_Primitives.emplace_back();
            ptr = std::make_shared<Triangle>();
            auto &tri = *std::static_pointer_cast<Triangle>(ptr);
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
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

            // bind material
            auto mid = shapes[s].mesh.material_ids[f];
            tri.mMaterial = mats[mid];

            //if (tri.mMaterial->is_emissive())
            //    m_light_ids.push_back(m_Primitives.size() - 1);
            index_offset += fv;
        }
    }
}

void Scene::BuildAggregate()
{
    m_Aggregate = std::make_shared<ListAggregate>(m_Primitives);
}
