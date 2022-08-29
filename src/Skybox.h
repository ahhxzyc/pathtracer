#pragma once

#include "Types.h"
#include "Ray.h"
#include <string>
using namespace std;

class Skybox {
public:
    Skybox(const string &filepath);
    ~Skybox() = default;

    // return color of the sky box at intersection point
    Vec3f get_color(Ray ray);

public:
    bool m_valid = false;
    float *m_data = nullptr;
    int m_width = 0;
    int m_height = 0;
};