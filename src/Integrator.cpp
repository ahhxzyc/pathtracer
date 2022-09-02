#include "Integrator.h"

Integrator::Integrator(const std::shared_ptr<Camera> &camera) : m_Camera(camera)
{

}

void Integrator::Render(const Scene &scene)
{
    auto size = m_Camera->film->Size();
    int spp = 5000;
    for (int rid = 0; rid < spp; rid ++)
    {
#pragma omp parallel for
        for (int pix = 0; pix < size.x * size.y; pix ++)
        {
            auto wi = pix % size.x, hi = pix / size.y;
            auto color = Li(m_Camera->GetRay({ wi, hi }), scene, 6);
            m_Camera->film->AddSample({ wi, hi }, color);
        }
        printf("rid: %d\n", rid);
        if (rid % 2 == 0)
        {
            string name = "output_" + to_string(rid) + ".bmp";
            m_Camera->film->Save(name);
        }
    }
}
