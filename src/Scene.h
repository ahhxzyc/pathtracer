#pragma once


#include "Triangle.h"
#include "OctTree.h"
#include "Primitive.h"
#include "Aggregate.h"

#include <vector>

using namespace std;

class Scene {
public:
    Scene();
    ~Scene();

    // load a model from file
    void addModel(const string &filepath);

    void BuildAggregate();

    auto &GetAggregate() const {return *m_Aggregate;}

public:
    std::vector<std::shared_ptr<Primitive>> m_Primitives;
    std::shared_ptr<Aggregate> m_Aggregate;
    vector<int> m_light_ids;
};
