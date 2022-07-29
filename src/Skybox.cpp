#include "Skybox.h"

#include "hdrloader.h"
#include <iostream>


Skybox::Skybox(const string &filepath) {
    HDRLoaderResult result;
    bool ret = HDRLoader::load(filepath.c_str(), result);
    if (ret) {
        m_valid = true;
        m_data = result.cols;
        m_width = result.width;
        m_height = result.height;
        cout << "loaded sky box " << filepath << " ";
        cout << "(" << m_width << "," << m_height << ")\n";
    } else {
        cout << "failed to load sky box " << filepath << endl;
    }
}



Vec3f Skybox::get_color(Ray ray) {
    float x = ray.dir[0];
    float y = ray.dir[1];
    float z = ray.dir[2];
    float theta = acos(y);
    float phi = 
            z > 0.f ?
            acos(x / sin(theta)) :
            2*PI - acos(x / sin(theta));
    int xi = phi / (2 * PI) * m_width;
    int yi = theta / PI * m_height;
    xi = max(0, min(xi, m_width - 1));
    yi = max(0, min(yi, m_height - 1));
    int idx = yi * m_width + xi;
    return Vec3f(
        m_data[idx * 3 + 0],
        m_data[idx * 3 + 1],
        m_data[idx * 3 + 2]
    );
}