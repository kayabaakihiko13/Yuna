
#ifndef PDE_ENCHANCE_HH
#define PDE_ENCHANCE_HH

#include <cmath>
#include <vector>
#include <algorithm>
#include <functional>
#include "utils/constants.hh"
#include "utils/utils.hh"
#include "image_enhance/lanczos.hh"

using ProgressCallback = std::function<void(int,int)>;

namespace PDE_SR
{
    // float point image representation
    struct FloatImage
    {
        int w,h;
        std::vector<float> r,g,b;
    };

    inline FloatImage image_to_float(const Image& img){
        FloatImage f;
        f.w = img.width; f.h = img.height;
        f.r.resize(f.w * f.h); f.g.resize(f.w * f.h); f.b.resize(f.w * f.h);
        for (int i = 0; i < f.w * f.h; ++i) {
            f.r[i] = static_cast<float>(img.data[i].r);
            f.g[i] = static_cast<float>(img.data[i].g);
            f.b[i] = static_cast<float>(img.data[i].b);
        }
        return f;
    }

    inline Image float_to_image(const FloatImage& f){
        Image img;
        img.width = f.w; img.height = f.h;
        img.data.resize(f.w * f.h);
        for (int i = 0; i < f.w * f.h; ++i) {
            img.data[i].r = static_cast<uint8_t>(std::clamp(std::round(f.r[i]), 0.0f, 255.0f));
            img.data[i].g = static_cast<uint8_t>(std::clamp(std::round(f.g[i]), 0.0f, 255.0f));
            img.data[i].b = static_cast<uint8_t>(std::clamp(std::round(f.b[i]), 0.0f, 255.0f));
        }
        return img;
    }
    //  Anisotropic diffusion 
    inline void anisotropic_diffusion_channel(std::vector<float>&u,
                                              int w,int h,float K,float dt,int iter){
         std::vector<float> u_next(w * h);
        for (int t = 0; t < iter; ++t) {
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    int idx = y * w + x;
                    
                    // Finite differences dengan boundary clamp (replicating)
                    int yn = std::max(y - 1, 0);
                    int ys = std::min(y + 1, h - 1);
                    int xe = std::min(x + 1, w - 1);
                    int xw = std::max(x - 1, 0);
                    
                    float grad_N = u[yn * w + x] - u[idx];
                    float grad_S = u[ys * w + x] - u[idx];
                    float grad_E = u[y * w + xe] - u[idx];
                    float grad_W = u[y * w + xw] - u[idx];
                    
                    float cN = 1.0f / (1.0f + (grad_N * grad_N) / (K * K));
                    float cS = 1.0f / (1.0f + (grad_S * grad_S) / (K * K));
                    float cE = 1.0f / (1.0f + (grad_E * grad_E) / (K * K));
                    float cW = 1.0f / (1.0f + (grad_W * grad_W) / (K * K));
                    
                    float divergence = cN * grad_N + cS * grad_S
                                    + cE * grad_E + cW * grad_W;
                    
                    u_next[idx] = u[idx] + dt * divergence;
                    }
                }
            }
            u.swap(u_next);
    }
    inline Image anisotropic_diffusion(const Image& input,
                                    float K = 30.0f,
                                    float dt = 0.2f,
                                    int iterations = 15) {
        FloatImage f = image_to_float(input);
        anisotropic_diffusion_channel(f.r, f.w, f.h, K, dt, iterations);
        anisotropic_diffusion_channel(f.g, f.w, f.h, K, dt, iterations);
        anisotropic_diffusion_channel(f.b, f.w, f.h, K, dt, iterations);
        return float_to_image(f);
    }

    // shock filter
    inline void shock_filter_channel(std::vector<float>& u,int w,int h,
                                    float dt,int iter){
        std::vector<float> u_next(w*h);
        const float eps= 1e-6f;
        // ini bagian agak berat
        for(int t= 0; t< iter;++t){
            for(int y = 0;y<h; ++y){
                for(int x = 0;x<w; ++x){
                    int idx = y*w+x;
                    // boundry clamp
                    int yn = std::max(y-1,0);
                    int ys = std::min(y + 1, h - 1);
                    int xw = std::max(x - 1, 0);
                    int xe = std::min(x + 1, w - 1);

                    // turunan pertama
                    float ux = (u[y*w+xe] - u[y*w+xw])/ 2.0f;
                    float uy = (u[ys*w+x] - u[yn*w+x]) /2.0f;
                    float grad_mag = std::sqrt(ux*ux + uy*uy+eps);

                    // second derivative
                    float uxx = u[y*w+xe] -2.0f * u[idx]+u[y*w +xw];
                    float uyy = u[ys * w + x] - 2.0f * u[idx] + u[yn * w + x];
                    float uxy = (u[ys * w + xe] - u[ys * w + xw]
                           - u[yn * w + xe] + u[yn * w + xw]) / 4.0f;
                    // second derivative for gradien direction
                    float u_nn = (ux * ux * uxx + 2.0f * ux * uy * uxy
                            + uy * uy * uyy) / (grad_mag * grad_mag + eps);
                    // logic for atur shock dilatation
                    float sign_u_nn = (u_nn > eps) ? 1.0f: (u_nn < -eps) ? -1.0f : 0.0f;
                    u_next[idx] = u[idx] - dt*sign_u_nn * grad_mag;
    
                }
            }
            u.swap(u_next);
        }
    }
    inline Image shock_filter(const Image& input,float dt = 0.1f,
                              int iterations = 8){
        FloatImage f= image_to_float(input);
        shock_filter_channel(f.r,f.w,f.h,dt,iterations);
        shock_filter_channel(f.g,f.w,f.h,dt,iterations);
        shock_filter_channel(f.b,f.w,f.h,dt,iterations);
        return float_to_image(f);
    }
    // Post-processing pipeline (applied AFTER Lanczos upscaling)
    inline Image enhance(const Image& src,
                      bool use_diffusion = true,
                      bool use_shock = true,
                      float K = 30.0f,
                      float diff_dt = 0.2f,
                      int diff_iter = 15,
                      float shock_dt = 0.1f,
                      int shock_iter = 8) {
    
    Image result = src;
    
    // Step 1: Anisotropic Diffusion (hapus ringing artifacts)
    if (use_diffusion) {
        result = anisotropic_diffusion(result, K, diff_dt, diff_iter);
    }
    
    // Step 2: Shock Filter (tajamkan edge)
    if (use_shock) {
        result = shock_filter(result, shock_dt, shock_iter);
    }
    
    return result;
    }
} // namespace PDE_SR




#endif


