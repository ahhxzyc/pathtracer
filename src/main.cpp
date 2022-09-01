#include "Types.h"
#include "Utils.h"
#include "Scene.h"
#include "Camera.h"
#include "WhittedIntegrator.h"

#include <iostream>

using namespace std;

int main()
{
    Scene scene;
    scene.addModel("E:/vscodedev/ptracer/res/cornell-box/cornell-box.obj");
    scene.BuildAggregate();
    
    auto camera = std::make_shared<Camera>(Size2i{ 800, 800 });
    auto integrator = std::make_shared<WhittedIntegrator>(camera);

    integrator->Render(scene);

    return 0;
}