#ifndef __LANCZOC_HH
#define __LANCZOC_HH

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

#include "utils/constants.hh"
#include "utils/utils.hh"

// implement kernel lanczoc
float lanczos(float x, int a)
{
    if (x < -a || x > a)
        return 0.0f;
    return sinc(x) * sinc(x / a);
}

// bounding image with camp value
int clamp(int val, int min_val, int max_val)
{
    return std::max(min_val, std::min(max_val, val));
}

// Di lanczoc.hh, tambahkan:
#include <functional>
using ProgressCallback = std::function<void(int, int)>;

Image resizeLanczos(const Image &src, float scale_factor, ProgressCallback cb = nullptr)
{
    int dst_width = static_cast<int>(std::round(src.width * scale_factor));
    int dst_height = static_cast<int>(std::round(src.height * scale_factor));

    Image dst;
    dst.width = dst_width;
    dst.height = dst_height;
    dst.data.resize(dst_width * dst_height);

    float x_ratio = static_cast<float>(src.width) / dst_width;
    float y_ratio = static_cast<float>(src.height) / dst_height;

    int total = dst_width * dst_height;
    int processed = 0;

    for (int y = 0; y < dst_height; ++y)
    {
        for (int x = 0; x < dst_width; ++x)
        {
            float src_x = (x + 0.5) * x_ratio - 0.5;
            float src_y = (y + 0.5) * y_ratio - 0.5;

            float r_val = 0.0f, g_val = 0.0f, b_val = 0.0f;
            float weight_sum = 0.0;

            int x_start = static_cast<int>(std::floor(src_x)) - LANCZOS_RADIUS + 1;
            int x_end = static_cast<int>(std::ceil(src_x)) + LANCZOS_RADIUS;
            int y_start = static_cast<int>(std::floor(src_y)) - LANCZOS_RADIUS + 1;
            int y_end = static_cast<int>(std::ceil(src_y)) + LANCZOS_RADIUS;

            for (int ky = y_start; ky <= y_end; ++ky)
            {
                for (int kx = x_start; kx <= x_end; ++kx)
                {
                    float wx = lanczos(static_cast<float>(kx) - src_x, LANCZOS_RADIUS);
                    float wy = lanczos(static_cast<float>(ky) - src_y, LANCZOS_RADIUS);
                    float weight = wx * wy;

                    if (weight > 0.0f)
                    {
                        int cx = clamp(kx, 0, src.width - 1);
                        int cy = clamp(ky, 0, src.height - 1);

                        const Pixel &p = src.at(cx, cy);

                        r_val += static_cast<float>(p.r) * weight;
                        g_val += static_cast<float>(p.g) * weight;
                        b_val += static_cast<float>(p.b) * weight;
                        weight_sum += weight;
                    }
                }
            }

            if (weight_sum > 0.0f)
            {
                float inv_sum = 1.0f / weight_sum;
                dst.at(x, y).r = static_cast<uint8_t>(std::round(r_val * inv_sum));
                dst.at(x, y).g = static_cast<uint8_t>(std::round(g_val * inv_sum));
                dst.at(x, y).b = static_cast<uint8_t>(std::round(b_val * inv_sum));
            }
            else
            {
                dst.at(x, y) = {0, 0, 0};
            }
            processed++;
            if (cb && processed % (total / 100 + 1) == 0)
            {
                cb(processed, total);
            }
        }
    }
    return dst;
}
#endif // __LANCZOS_HH