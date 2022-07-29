#include "Types.h"
#include "Utils.h"
#include "Renderer.h"
#include "hdrloader.h"

#include <iostream>

using namespace std;

int main()
{
    Scene scene(800, 800);
    scene.addModel("E:/vscodedev/ptracer/res/cornell-box/cornell-box.obj");
    // scene.addSkybox("../res/environment.hdr");
    scene.init_octtree();

    Renderer renderer(&scene);
    renderer.render();

    return 0;
}