#pragma once


#include "Triangle.h"
#include "OctTree.h"
#include "Primitive.h"
#include "Aggregate.h"
#include "Camera.h"

#include <vector>
#include <glm/glm.hpp>

class Scene {
public:
    Scene();
    ~Scene();

    auto GetCamera() {return m_Camera; }
    void AddModel(const std::string &dir, const std::string &name);

    void ParseScene(const std::string &dir, const std::string &name);

    void BuildAggregate();

    auto &GetAggregate() const {return *m_Aggregate;}

public:
    std::shared_ptr<Camera> m_Camera;
    std::vector<std::shared_ptr<Primitive>> m_Primitives;
    std::shared_ptr<Aggregate> m_Aggregate;
    std::vector<std::shared_ptr<Light>> m_Lights;
    std::unordered_map<std::string, glm::vec3> m_LightRadiance;
};
