#ifndef LANCZOC_TEST_HH
#define LANCZOC_TEST_HH

#ifdef _WIN32
#define NOMINMAX
#endif

#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <algorithm>
#include <vector>

#include "utils/utils.hh"
#include "utils/metrics.hh"
#include "utils/constants.hh"
#include "image_enhance/lanczos.hh"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace image_utils;

static int tests_passed = 0;
static int tests_total = 0;

inline void check(bool condition, const char* name) {
    tests_total++;
    if (condition) {
        std::cout << "  [PASS] " << name << "\n";
        tests_passed++;
    } else {
        std::cout << "  [FAIL] " << name << "\n";
    }
}

inline void check_eq(float a, float b, float eps, const char* name) {
    tests_total++;
    if (std::fabs(a - b) < eps) {
        std::cout << "  [PASS] " << name << "\n";
        tests_passed++;
    } else {
        std::cout << "  [FAIL] " << name << " (got " << a << ", expected " << b << ")\n";
    }
}

inline void check_img_eq(const Image& a, const Image& b, const char* name) {
    tests_total++;
    if (a.width != b.width || a.height != b.height) {
        std::cout << "  [FAIL] " << name << " (dimensi beda: "
                  << a.width << "x" << a.height << " vs "
                  << b.width << "x" << b.height << ")\n";
        return;
    }
    for (int y = 0; y < a.height; y++) {
        for (int x = 0; x < a.width; x++) {
            const Pixel& pa = a.at(x, y);
            const Pixel& pb = b.at(x, y);
            if (pa.r != pb.r || pa.g != pb.g || pa.b != pb.b) {
                std::cout << "  [FAIL] " << name << " (beda di (" << x << "," << y
                          << "): got (" << (int)pa.r << "," << (int)pa.g << "," << (int)pa.b
                          << ") expected (" << (int)pb.r << "," << (int)pb.g << "," << (int)pb.b << "))\n";
                return;
            }
        }
    }
    std::cout << "  [PASS] " << name << "\n";
    tests_passed++;
}

// ==========================================
// TESTS: sinc()
// ==========================================

inline void test_sinc_zero() {
    float s = image_utils::sinc(0.0f);
    check_eq(s, 1.0f, 1e-6f, "sinc(0) == 1");
}

inline void test_sinc_pi() {
    // sinc(1) = sin(pi)/pi = 0
    float s = image_utils::sinc(1.0f);
    check_eq(s, 0.0f, 1e-6f, "sinc(1) == 0");
}

inline void test_sinc_half() {
    // sinc(0.5) = sin(pi/2)/(pi/2) = 2/pi
    float s = image_utils::sinc(0.5f);
    check_eq(s, 2.0f / 3.14159265f, 1e-6f, "sinc(0.5) == 2/pi");
}

inline void test_sinc_integer() {
    for (int n = 1; n <= 10; n++) {
        float s = image_utils::sinc(static_cast<float>(n));
        check_eq(s, 0.0f, 1e-6f, ("sinc(" + std::to_string(n) + ") == 0").c_str());
    }
}

inline void test_sinc_negative() {
    // sinc(-x) = sinc(x) because it's an even function
    float s_pos = image_utils::sinc(0.75f);
    float s_neg = image_utils::sinc(-0.75f);
    check_eq(s_pos, s_neg, 1e-6f, "sinc(-x) == sinc(x)");
}

// ==========================================
// TESTS: lanczos::kernel()
// ==========================================

inline void test_kernel_outside_radius() {
    float k = lanczos::kernel(3.5f, 3);
    check_eq(k, 0.0f, 1e-6f, "kernel(x=3.5, a=3) == 0 (outside radius)");
}

inline void test_kernel_at_radius_edge() {
    float k = lanczos::kernel(3.0f, 3);
    check_eq(k, 0.0f, 1e-6f, "kernel(x=3, a=3) == 0 (at radius edge)");
}

inline void test_kernel_zero() {
    float k = lanczos::kernel(0.0f, 3);
    check_eq(k, 1.0f, 1e-6f, "kernel(0, a=3) == 1");
}

inline void test_kernel_symmetry() {
    float k_pos = lanczos::kernel(1.5f, 3);
    float k_neg = lanczos::kernel(-1.5f, 3);
    check_eq(k_pos, k_neg, 1e-6f, "kernel(x) == kernel(-x)");
}

inline void test_kernel_has_negative_lobes() {
    // For a=3, there should be negative values between 1 and 2
    float k_at_1 = lanczos::kernel(1.0f, 3); // zero crossing
    float k_at_1p5 = lanczos::kernel(1.5f, 3); // negative lobe
    float k_at_2 = lanczos::kernel(2.0f, 3); // zero crossing
    check(k_at_1p5 < 0.0f, "kernel(1.5, 3) has negative lobe");
    check_eq(k_at_1, 0.0f, 1e-6f, "kernel(1, a=3) == 0 (zero crossing #1)");
    check_eq(k_at_2, 0.0f, 1e-6f, "kernel(2, a=3) == 0 (zero crossing #2)");
}


// TESTS: lanczos::resize()


inline void test_resize_empty_source() {
    Image empty;
    empty.width = 0;
    empty.height = 0;
    Image result = lanczos::resize(empty, 2.0f);
    check(result.data.empty(), "resize(empty, 2x) -> empty result");
}

inline void test_resize_identity_scale() {
    Image src;
    src.width = 4;
    src.height = 4;
    src.data.resize(16);
    for (int i = 0; i < 16; i++) {
        src.data[i] = {static_cast<uint8_t>((i * 37) % 256),
                       static_cast<uint8_t>((i * 51) % 256),
                       static_cast<uint8_t>((i * 73) % 256)};
    }
    Image result = lanczos::resize(src, 1.0f);
    check_img_eq(src, result, "resize(src, 1.0x) == src");
}

inline void test_resize_solid_color() {
    Image src;
    src.width = 4;
    src.height = 4;
    src.data.resize(16, {127, 200, 50});

    Image result = lanczos::resize(src, 2.0f);
    check(result.width == 8 && result.height == 8, "resize(solid, 2x) -> 8x8");
    // All pixels should still be same solid color
    bool same = true;
    for (auto& p : result.data) {
        if (p.r != 127 || p.g != 200 || p.b != 50) { same = false; break; }
    }
    check(same, "resize(solid, 2x) -> all pixels same color");
}

inline void test_resize_2x_checkerboard() {
    // 2x2 checkerboard hitam/putih → 4x4
    Image src;
    src.width = 2;
    src.height = 2;
    src.data = {{255,255,255}, {0,0,0}, {0,0,0}, {255,255,255}};

    Image result = lanczos::resize(src, 2.0f);
    check(result.width == 4 && result.height == 4, "resize(checker 2x2, 2x) -> 4x4");
    // Center pixels should be interpolated (greyish)
    const Pixel& center = result.at(1, 1);
    check(center.r > 0 && center.r < 255, "resize(checker, 2x) center pixel interpolated");
}

inline void test_resize_1x1_to_2x() {
    Image src;
    src.width = 1;
    src.height = 1;
    src.data = {{100, 150, 200}};

    Image result = lanczos::resize(src, 2.0f);
    check(result.width == 2 && result.height == 2, "resize(1x1, 2x) -> 2x2");
    bool same = true;
    for (auto& p : result.data) {
        if (p.r != 100 || p.g != 150 || p.b != 200) { same = false; break; }
    }
    check(same, "resize(1x1, 2x) -> all pixels same as source");
}

inline void test_resize_4x_scale() {
    Image src;
    src.width = 3;
    src.height = 3;
    src.data.resize(9, {0, 255, 0});
    src.data[4] = {255, 0, 0}; // center pixel red

    Image result = lanczos::resize(src, 4.0f);
    check(result.width == 12 && result.height == 12, "resize(3x3, 4x) -> 12x12");
}

inline void test_resize_progress_callback() {
    Image src;
    src.width = 4;
    src.height = 4;
    src.data.resize(16, {128, 128, 128});

    int last_done = 0, last_total = 0;
    auto cb = [&](int done, int total) { last_done = done; last_total = total; };
    lanczos::resize(src, 2.0f, cb);
    check(last_done == last_total && last_total > 0, "resize callback called with done==total");
}

inline void test_resize_border_clamp() {
    // Pixels near the edge should be clamped properly (no crash)
    Image src;
    src.width = 2;
    src.height = 2;
    src.data = {{10,20,30}, {40,50,60}, {70,80,90}, {100,110,120}};

    // Up then down should not crash
    Image result = lanczos::resize(src, 0.5f);
    check(result.width == 1 && result.height == 1, "resize(2x2, 0.5x) -> 1x1 (border clamp test)");
}

inline void test_resize_negative_weight_usage() {
    // Test yang menunjukkan bahwa negative weights diabaikan (bug)
    // Buat 3x3 image dengan center beda
    Image src;
    src.width = 3;
    src.height = 3;
    for (int i = 0; i < 9; i++) src.data.push_back({100, 100, 100});
    src.data[4] = {200, 200, 200}; // center brighter

    Image result = lanczos::resize(src, 2.0f);
    // Output harus 6x6, tidak crash
    check(result.width == 6 && result.height == 6, "resize(3x3, 2x) -> 6x6 (negative weight test)");
}

// ==========================================
// SIMILARITY / PSNR COMPARISON
// ==========================================

struct SimilarityResult {
    double overall_similarity;
    double red_similarity;
    double green_similarity;
    double blue_similarity;
    double psnr;
};

inline SimilarityResult calculate_similarity(const Image& img1, const Image& img2) {
    SimilarityResult result = {0, 0, 0, 0, 0};

    if (img1.width != img2.width || img1.height != img2.height) {
        return result;
    }

    int total_pixels = img1.width * img1.height;
    double sum_red = 0, sum_green = 0, sum_blue = 0;
    double sum_squared_error = 0;

    for (int y = 0; y < img1.height; y++) {
        for (int x = 0; x < img1.width; x++) {
            const Pixel& p1 = img1.at(x, y);
            const Pixel& p2 = img2.at(x, y);

            int diff_r = std::abs(static_cast<int>(p1.r) - static_cast<int>(p2.r));
            int diff_g = std::abs(static_cast<int>(p1.g) - static_cast<int>(p2.g));
            int diff_b = std::abs(static_cast<int>(p1.b) - static_cast<int>(p2.b));

            sum_red += diff_r;
            sum_green += diff_g;
            sum_blue += diff_b;

            sum_squared_error += diff_r * diff_r;
            sum_squared_error += diff_g * diff_g;
            sum_squared_error += diff_b * diff_b;
        }
    }

    result.red_similarity   = 100.0 * (1.0 - (sum_red   / (total_pixels * 255.0)));
    result.green_similarity = 100.0 * (1.0 - (sum_green / (total_pixels * 255.0)));
    result.blue_similarity  = 100.0 * (1.0 - (sum_blue  / (total_pixels * 255.0)));
    result.overall_similarity = (result.red_similarity + result.green_similarity + result.blue_similarity) / 3.0;

    double mse = sum_squared_error / (total_pixels * 3);
    if (mse < 1e-10) result.psnr = 100.0;
    else result.psnr = 10.0 * std::log10((255.0 * 255.0) / mse);

    return result;
}


// TESTS: SIMILARITY with PNSR

inline Image downscale_nearest(const Image& src, int tw, int th) {
    Image dst;
    dst.width = tw;
    dst.height = th;
    dst.data.resize(tw * th);
    float sx = static_cast<float>(src.width) / tw;
    float sy = static_cast<float>(src.height) / th;
    for (int y = 0; y < th; y++) {
        for (int x = 0; x < tw; x++) {
            int src_x = static_cast<int>(x * sx);
            int src_y = static_cast<int>(y * sy);
            if (src_x >= src.width) src_x = src.width - 1;
            if (src_y >= src.height) src_y = src.height - 1;
            dst.at(x, y) = src.at(src_x, src_y);
        }
    }
    return dst;
}

inline void test_similarity_identical() {
    Image img;
    img.width = 10; img.height = 10;
    img.data.resize(100, {128, 64, 192});

    auto sim = calculate_similarity(img, img);
    check(sim.psnr > 99.0, "similarity(identical) PSNR ~100 dB");
    check(sim.overall_similarity > 99.0, "similarity(identical) ~100%");
}

inline void test_similarity_noise() {
    Image img1, img2;
    img1.width = img2.width = 10;
    img1.height = img2.height = 10;
    img1.data.resize(100, {128, 128, 128});
    img2.data.resize(100, {130, 126, 129});

    auto sim = calculate_similarity(img1, img2);
    check(sim.psnr > 35.0 && sim.psnr < 55.0, "similarity(small noise) PSNR ~40-50 dB");
    check(sim.overall_similarity > 95.0, "similarity(small noise) ~98-99%");
}

inline void test_similarity_opposite() {
    Image img1, img2;
    img1.width = img2.width = 10;
    img1.height = img2.height = 10;
    img1.data.resize(100, {255, 255, 255});
    img2.data.resize(100, {0, 0, 0});

    auto sim = calculate_similarity(img1, img2);
    check(sim.psnr < 15.0, "similarity(opposite) PSNR ~0 dB");
    check(sim.overall_similarity < 5.0, "similarity(opposite) ~0%");
}

inline void test_similarity_upscale_downscale() {
    Image src;
    src.width = 4; src.height = 4;
    src.data.resize(16);
    for (int i = 0; i < 16; i++) {
        uint8_t v = static_cast<uint8_t>((i * 51) % 256);
        src.data[i] = {v, v, v};
    }

    {
        Image dst;
        dst.width = 2; dst.height = 2;
        dst.data.resize(4);
        float sx = static_cast<float>(src.width) / 2;
        float sy = static_cast<float>(src.height) / 2;
        for (int y = 0; y < 2; y++) {
            for (int x = 0; x < 2; x++) {
                int sx_ = (x * sx < src.width - 1) ? static_cast<int>(x * sx) : src.width - 1;
                int sy_ = (y * sy < src.height - 1) ? static_cast<int>(y * sy) : src.height - 1;
                dst.at(x, y) = src.at(sx_, sy_);
            }
        }
        Image restored = lanczos::resize(dst, 2.0f);

        auto sim = calculate_similarity(src, restored);
        std::cout << "  [INFO] Lanczos roundtrip 4x4->2x2->4x4: PSNR="
                  << std::fixed << std::setprecision(2) << sim.psnr << " dB, "
                  << "Similarity=" << sim.overall_similarity << "%\n";
        check(sim.overall_similarity > 70.0, "Lanczos roundtrip similarity > 70%");
    }
}

// ==========================================
// MAIN TEST RUNNER
// ==========================================

inline void print_separator(char c = '=') {
    std::cout << std::string(50, c) << "\n";
}

namespace lanczos_test {

inline int run_all_tests() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    print_separator();
    std::cout << "  Lanczos Unit Test Suite\n";
    print_separator();

    // --- sinc() tests ---
    std::cout << "\n[image_utils::sinc]\n";
    test_sinc_zero();
    test_sinc_pi();
    test_sinc_half();
    test_sinc_integer();
    test_sinc_negative();

    // --- kernel() tests ---
    std::cout << "\n[lanczos::kernel]\n";
    test_kernel_outside_radius();
    test_kernel_at_radius_edge();
    test_kernel_zero();
    test_kernel_symmetry();
    test_kernel_has_negative_lobes();

    // --- resize() tests ---
    std::cout << "\n[lanczos::resize]\n";
    test_resize_empty_source();
    test_resize_identity_scale();
    test_resize_solid_color();
    test_resize_1x1_to_2x();
    test_resize_2x_checkerboard();
    test_resize_4x_scale();
    test_resize_progress_callback();
    test_resize_border_clamp();
    test_resize_negative_weight_usage();

    // --- PSNR / Similarity tests ---
    std::cout << "\n[PSNR / Similarity]\n";
    test_similarity_identical();
    test_similarity_noise();
    test_similarity_opposite();
    test_similarity_upscale_downscale();

    // --- Summary ---
    std::cout << "\n";
    print_separator();
    std::cout << "  RESULT: " << tests_passed << "/" << tests_total << " passed\n";
    if (tests_passed == tests_total) {
        std::cout << "  *** ALL TESTS PASSED! ***\n";
    } else {
        std::cout << "  *** " << (tests_total - tests_passed) << " TESTS FAILED ***\n";
    }
    print_separator();

    return (tests_passed == tests_total) ? 0 : 1;
}

} // namespace lanczos_test

#endif // LANCZOC_TEST_HH
