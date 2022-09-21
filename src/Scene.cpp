#include "scene.h"
#include "common.h"
#include "Log.h"
#include "camera.h"
#include "material.h"

#include <iostream>
#include <tiny_obj_loader.h>
#include <pugixml.hpp>

void Scene::add_model(const std::string &dir, const std::string &name)
{
    std::string inputfile = dir + "/" + name + ".obj";
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = dir;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(inputfile, reader_config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }
    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    // Get geometry data from loader
    auto &attrib = reader.GetAttrib();
    auto &shapes = reader.GetShapes();
    auto &materials = reader.GetMaterials();

    // Register the materials
    std::vector<std::shared_ptr<Material>> mats;
    for (auto &tmat : materials)
    {
        mats.push_back(std::make_shared<Material>());
        auto &mat = mats.back();

        // diffuse map
        if (tmat.diffuse_texname.empty())
            mat->kdMap = std::make_shared<Texture>(Vec3f(tmat.diffuse[0], tmat.diffuse[1], tmat.diffuse[2]));
        else
            mat->kdMap = std::make_shared<Texture>(reader_config.mtl_search_path + "/" + tmat.diffuse_texname);

        // specular ks
        mat->ks = Vec3f(tmat.specular[0], tmat.specular[1], tmat.specular[2]);

        // specular shininess
        mat->shininess = tmat.shininess;

        // light radiance
        auto it = lightRadiance_.find(tmat.name);
        if (it != lightRadiance_.end())
        {
            mat->ke = it->second;
        }
    }

    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces and save to the scene
        size_t index_offset = 0;
        auto num_faces = shapes[s].mesh.num_face_vertices.size();
        primitives_.reserve(primitives_.size() + num_faces);

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
            primitives_.push_back( std::make_shared<Triangle>(material) );
            auto &tri = *std::static_pointer_cast<Triangle>(primitives_.back());

            // Register an area light if there is one
            if (material->IsEmissive())
            {
                auto light = std::make_shared<AreaLight>(tri, material->ke);
                tri.SetAreaLight(light);
                lights.push_back(light);
            }

            // Save vertex information
            int fv = shapes[s].mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; v++)
            {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                tri.p[v].x = attrib.vertices[3 * idx.vertex_index + 0];
                tri.p[v].y = attrib.vertices[3 * idx.vertex_index + 1];
                tri.p[v].z = attrib.vertices[3 * idx.vertex_index + 2];

                tri.n[v].x = attrib.normals[3 * idx.vertex_index + 0];
                tri.n[v].y = attrib.normals[3 * idx.vertex_index + 1];
                tri.n[v].z = attrib.normals[3 * idx.vertex_index + 2];

                tri.uv[v].x = attrib.texcoords[2 * idx.texcoord_index + 0];
                tri.uv[v].y = attrib.texcoords[2 * idx.texcoord_index + 1];
            }

            index_offset += fv;
        }
    }
}

void Scene::parse(const std::string &dir, const std::string &name)
{
    pugi::xml_document doc;
    auto xmlPath = dir + '/' + name + ".xml";
    pugi::xml_parse_result result = doc.load_file(xmlPath.c_str());
    if (!result)
        return ;

    auto split = [](const std::string &str, const std::string &delimiters = " ") -> std::vector<std::string>
    {
        std::vector<std::string> ret;
        auto lastPos = str.find_first_not_of(delimiters, 0);
        auto pos = str.find_first_of(delimiters, lastPos);
        while (lastPos != std::string::npos || pos != std::string::npos)
        {
            if (pos == std::string::npos)
                pos = str.size();
            ret.emplace_back(str.begin() + lastPos, str.begin() + pos);
            lastPos = str.find_first_not_of(delimiters, pos);
            pos = str.find_first_of(delimiters, lastPos);
        }
        return ret;
    };
    auto stringToVec = [&split](const std::string &str) -> glm::vec3
    {
        auto splitvec = split(str, ", ");
        if (splitvec.size() != 3)
        {
            LOG_ERROR("Error parsing scene file: {} is not a vec3", str);
            return glm::vec3();
        }
        return glm::vec3(std::stof(splitvec[0]), std::stof(splitvec[1]), std::stof(splitvec[2]));
    };

    for (auto light : doc.children("light"))
    {
        auto name = light.attribute("mtlname").as_string();
        auto rad = light.attribute("radiance").as_string();
        LOG_INFO("light material {} has radiance : {}", name, rad);
        lightRadiance_[name] = stringToVec(rad);
    }

    auto camAttrs = doc.child("camera");
    auto width = camAttrs.child("width").attribute("value").as_int();
    auto height = camAttrs.child("height").attribute("value").as_int();
    camera = std::make_shared<Camera>(Size2i{ width, height });
    camera->m_eye = stringToVec(camAttrs.child("eye").attribute("value").as_string());
    camera->m_lookat = stringToVec(camAttrs.child("lookat").attribute("value").as_string());
    camera->m_up = stringToVec(camAttrs.child("up").attribute("value").as_string());
    camera->m_fov = camAttrs.child("fovy").attribute("value").as_float();

    add_model(dir, name);
    LOG_INFO("Number of lights in the scene: {}", lights.size());
}

void Scene::build_accel()
{
    accel = std::make_shared<BVHAccel>(primitives_);
}
