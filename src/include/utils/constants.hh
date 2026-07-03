#ifndef CONSTANTS_HH
#define CONSTANTS_HH

#include <cstddef>

constexpr int LANCZOS_RADIUS = 3;
constexpr float SCALES[] = {2.0f, 4.0f, 8.0f};
inline constexpr size_t SCALES_COUNT = sizeof(SCALES) / sizeof(SCALES[0]);

constexpr double PI = 3.14159265358979323846;

// Konstanta SSIM untuk citra 8-bit
const double SSIM_L  = 255.0;
const double SSIM_K1 = 0.01;
const double SSIM_K2 = 0.03;
const double SSIM_C1 = (SSIM_K1 * SSIM_L) * (SSIM_K1 * SSIM_L);  // 6.5025
const double SSIM_C2 = (SSIM_K2 * SSIM_L) * (SSIM_K2 * SSIM_L);  // 58.5225

#endif