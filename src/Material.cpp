#include "Material.h"

#include "stb_image.h"

Texture::Texture(std::string const &filename)
{
    stbi_set_flip_vertically_on_load(true);
    int c;
    auto tdata = stbi_loadf(filename.c_str(), &width, &height, &c, 3);

    data.resize(width * height * 3);
    int n = width * height * c;
    for (int i = 0, j = 0; i < n; i += c)
    {
        data[j ++ ] = tdata[i];
        data[j ++ ] = tdata[i + 1];
        data[j ++ ] = tdata[i + 2];
    }

    stbi_image_free(tdata);
}

Texture::Texture(Vec3f const &color)
{
    data = {color[0], color[1], color[2]};
    width = height = 1;
}

Texture::~Texture()
{
}

Vec3f Texture::get(float u, float v)
{
    u = glm::fract(u);
    v = glm::fract(v);
    int ind = static_cast<int>(v * height) * width + static_cast<int>(u * width);
    ind = std::max(0, std::min(int(data.size()/3-1), ind));
    return Vec3f(data[ind *3 + 0], data[ind * 3 + 1], data[ind * 3 + 2]);
}

