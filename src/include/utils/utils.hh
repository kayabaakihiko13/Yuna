#ifndef UTILS_HH
#define UTILS_HH

#include "constants.hh"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace image_utils
{

struct Pixel
{
    uint8_t r, g, b;
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
};

inline float sinc(float x)
{
    if (x == 0.0f)
        return 1.0f;
    float pix = static_cast<float>(PI) * x;
    return std::sin(pix) / pix;
}

} // namespace image_utils

using image_utils::Pixel;
using image_utils::Image;

#endif
