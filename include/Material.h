#pragma once

#include "Types.h"
#include <glm/gtx/norm.hpp>

#include <string>
#include <vector>
#include <memory>

class Texture
{
public:
    Texture(std::string const &filename);
    Texture(Vec3f const &color);
    ~Texture();
    
    Vec3f get(float u, float v);

private:
    std::vector<float> data;
    int width, height;
};

struct Material
{
    std::shared_ptr<Texture> kd_map;
    Vec3f ks;
    Vec3f ke;
    float shininess;
    float ior;

    bool IsEmissive() const
    {
        return std::abs(glm::length2(ke)) > 0.01f;
    }
};