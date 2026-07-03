#ifndef PSNR_EVALUATOR_HH
#define PSNR_EVALUATOR_HH

#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>

#include "image_enhance/lanczos.hh"
#include "utils/utils.hh"
#include "utils/metrics.hh"

inline bool load_image(const std::string& path, Image& img) {
    int w, h, c;
    unsigned char* raw = stbi_load(path.c_str(), &w, &h, &c, 3);
    if (!raw) return false;

    img.width = w;
    img.height = h;
    img.data.resize(w * h);

    for (int i = 0; i < w * h; i++) {
        img.data[i].r = raw[i * 3 + 0];
        img.data[i].g = raw[i * 3 + 1];
        img.data[i].b = raw[i * 3 + 2];
    }

    stbi_image_free(raw);
    return true;
}

#endif