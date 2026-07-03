#ifndef UTILS_METRICS
#define UTILS_METRICS

#include <vector>
#include <cmath>

#include "utils/utils.hh"
#include "utils/constants.hh"

namespace image_utils{



inline float calculate_psnr(const Image& imgOrigin,const Image& imgResult){
    if(imgOrigin.width != imgResult.width || imgOrigin.height != imgResult.height){
        return -1.0;
    }
    int n = imgOrigin.width * imgOrigin.height;
    float sum_squared_error = 0.0f;
    for(int y =0;y<imgOrigin.height; y++){
        for(int x = 0;x<imgOrigin.width; x++){
            const Pixel& p1 = imgOrigin.at(x,y);
            const Pixel& p2 = imgResult.at(x,y);
            // calculate error untuk setiap channel
            for(int c = 0;c<3;c++){
                float diff = p1[c] - p2[c];
                sum_squared_error += diff* diff;
            }
        }
    }
    float mse = sum_squared_error / (n*3);
    if(mse<1e-10) return 100.0;
    return 10.0 * std::log10((SSIM_L *SSIM_L)/ mse);
};


// calculate gausian kernel

}

#endif