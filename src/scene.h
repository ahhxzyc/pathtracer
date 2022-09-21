#pragma once

#include "common.h"
#include "primitive.h"
#include "accel.h"
#include "camera.h"

#include <vector>

class Scene {
public:
    void add_model(const std::string &dir, const std::string &name);
    void parse(const std::string &dir, const std::string &name);
    void build_accel();
public:
    std::shared_ptr<Camera>             camera;
    std::shared_ptr<Accel>              accel;
    std::vector<std::shared_ptr<Light>> lights;
private:
    std::vector<std::shared_ptr<Primitive>>     primitives_;
    std::unordered_map<std::string, glm::vec3>  lightRadiance_;
};
