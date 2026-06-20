#ifndef CONSTANTS_HH
#define CONSTANTS_HH

#include <cstddef>

constexpr int LANCZOS_RADIUS = 3;
constexpr float SCALES[] = {2.0f, 4.0f, 8.0f};
inline constexpr size_t SCALES_COUNT = sizeof(SCALES) / sizeof(SCALES[0]);

constexpr double PI = 3.14159265358979323846;

#endif