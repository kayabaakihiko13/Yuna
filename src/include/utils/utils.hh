#ifndef __UTILS_HH
#define __UTILS_HH
#include "constants.hh"
#include <vector>
#include <filesystem>
#include <cmath>
// utilities for define image and Pixel data type

struct Pixel
{
    uint8_t r, g, b;
};
struct Image
{
    int width;
    int height;
    std::vector<Pixel> data;

    // by passing
    Pixel &at(int x, int y)
    {
        return data[y * width + x];
    }
    const Pixel &at(int x, int y) const
    {
        return data[y * width + x];
    }
};

/*
 * Math helper
 */

// this for Sinc function algorihm
float sinc(float x)
{
    if (x == 0.0)
        return 1.0;
    return std::sin(M_PI * x) / (M_PI * x);
}

#endif