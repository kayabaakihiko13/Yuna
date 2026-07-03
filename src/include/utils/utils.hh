#ifndef UTILS_HH
#define UTILS_HH

#include "constants.hh"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <string>
#include "stb_image.h"
#include "stb_image_write.h"
#include "format_file_image.hh"
#include "interactive.hh"

namespace image_utils
{

struct Pixel
{
    uint8_t r, g, b;
    // menambahkan akses channel pada gambar
    uint8_t& operator[](int i) {
        return (i == 0) ? r : (i == 1) ? g : b;
    }
    const uint8_t& operator[](int i) const {
        return (i == 0) ? r : (i == 1) ? g : b;
    }
};

struct Image
{
    int width;
    int height;
    std::vector<Pixel> data;

    Pixel &at(int x, int y)
    {
        return data[y * width + x];
    }

    const Pixel &at(int x, int y) const
    {
        return data[y * width + x];
    }

    // adding helper for clamp koordinat
    const Pixel& at_clamped(int x,int y) const{
        x = std::max(0, std::min(width - 1, x));
        y = std::max(0, std::min(height - 1, y));
        return data[y * width + x];
    }
};

inline float sinc(float x)
{
    if (x == 0.0f)
        return 1.0f;
    float pix = static_cast<float>(PI) * x;
    return std::sin(pix) / pix;
}

inline Image load_image(const std::string &path, int &w, int &h)
{
    int ch = 0;
    std::string normalized = path;
    for (char &c : normalized)
        if (c == '\\')
            c = '/';
    unsigned char *data = stbi_load(normalized.c_str(), &w, &h, &ch, 3);
    if (!data)
    {
        return {0, 0, {}};
    }
    Image img{w, h, {}};
    img.data.resize(w * h);
    for (int i = 0; i < w * h; ++i)
    {
        img.data[i] = {data[i * 3], data[i * 3 + 1], data[i * 3 + 2]};
    }
    stbi_image_free(data);
    return img;
}

inline bool save_image(const std::string &path, const Image &img,
                       format_processing::ImageFormat fmt)
{
    std::vector<unsigned char> out(img.width * img.height * 3);
    for (int i = 0; i < img.width * img.height; ++i)
    {
        out[i * 3]     = img.data[i].r;
        out[i * 3 + 1] = img.data[i].g;
        out[i * 3 + 2] = img.data[i].b;
    }

    switch (fmt)
    {
    case format_processing::ImageFormat::PNG:
        return stbi_write_png(path.c_str(), img.width, img.height, 3,
                              out.data(), img.width * 3);
    case format_processing::ImageFormat::JPG:
        return stbi_write_jpg(path.c_str(), img.width, img.height, 3,
                              out.data(), 90);
    case format_processing::ImageFormat::BMP:
        return stbi_write_bmp(path.c_str(), img.width, img.height, 3,
                              out.data());
    default:
        interactive::log_warn("Format tidak didukung, fallback ke PNG");
        return stbi_write_png(path.c_str(), img.width, img.height, 3,
                              out.data(), img.width * 3);
    }
}

} // namespace image_utils

using image_utils::Pixel;
using image_utils::Image;

#endif
