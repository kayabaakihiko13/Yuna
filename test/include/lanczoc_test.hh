#ifndef __LANCZOS_TEST_HH
#define __LANCZOS_TEST_HH

#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>

#include "utils/constants.hh"
#include "utils/utils.hh"
#include "image_enchace/lanczoc.hh"

namespace lanczos_test
{

    // Helper: bandingkan dua pixel dengan toleransi
    bool pixels_equal(const Pixel &a, const Pixel &b, uint8_t tolerance = 2)
    {
        return std::abs(static_cast<int>(a.r) - static_cast<int>(b.r)) <= tolerance &&
               std::abs(static_cast<int>(a.g) - static_cast<int>(b.g)) <= tolerance &&
               std::abs(static_cast<int>(a.b) - static_cast<int>(b.b)) <= tolerance;
    }

    // Test 1: Basic upscale 2x
    void test_basic_upscale_2x()
    {
        std::cout << "Running test: basic_upscale_2x... ";

        Image src;
        src.width = 2;
        src.height = 2;
        src.data.resize(4);
        src.at(0, 0) = {255, 0, 0};
        src.at(1, 0) = {0, 255, 0};
        src.at(0, 1) = {0, 0, 255};
        src.at(1, 1) = {255, 255, 255};

        Image dst = resizeLanczos(src, 2.0f);

        assert(dst.width == 4 && dst.height == 4);
        assert(dst.data.size() == 16);

        std::cout << "✓ PASSED\n";
    }

    // Test 2: Downscale 0.5x
    void test_downscale_half()
    {
        std::cout << "Running test: downscale_half... ";

        Image src;
        src.width = 10;
        src.height = 10;
        src.data.resize(100, {128, 128, 128});

        Image dst = resizeLanczos(src, 0.5f);

        assert(dst.width == 5 && dst.height == 5);
        assert(dst.data.size() == 25);

        std::cout << "✓ PASSED\n";
    }

    // Test 3: Edge case - scale 1.0 (no change)
    void test_scale_identity()
    {
        std::cout << "Running test: scale_identity... ";

        Image src;
        src.width = 5;
        src.height = 5;
        src.data.resize(25);
        for (int i = 0; i < 25; ++i)
        {
            src.data[i] = {100, 150, 200};
        }

        Image dst = resizeLanczos(src, 1.0f);

        assert(dst.width == 5 && dst.height == 5);
        // Pixel harus mirip (Lanczos mungkin ada sedikit rounding)
        assert(pixels_equal(dst.at(2, 2), {100, 150, 200}, 5));

        std::cout << "✓ PASSED\n";
    }

    // Test 4: RGB gradient preservation
    void test_rgb_gradient()
    {
        std::cout << "Running test: rgb_gradient... ";

        Image src;
        src.width = 4;
        src.height = 4;
        src.data.resize(16);

        for (int y = 0; y < 4; ++y)
        {
            for (int x = 0; x < 4; ++x)
            {
                src.at(x, y) = {
                    static_cast<uint8_t>(x * 64),
                    static_cast<uint8_t>(y * 64),
                    128};
            }
        }

        Image dst = resizeLanczos(src, 2.0f);

        assert(dst.width == 8 && dst.height == 8);
        // Cek corner pixels
        assert(dst.at(0, 0).r < 50);  // Should be dark red
        assert(dst.at(7, 7).r > 200); // Should be bright red
        assert(dst.at(7, 7).g > 200); // Should be bright green

        std::cout << "✓ PASSED\n";
    }

    // Test 5: Non-square image
    void test_non_square_image()
    {
        std::cout << "Running test: non_square_image... ";

        Image src;
        src.width = 8;
        src.height = 4;
        src.data.resize(32, {255, 128, 0});

        Image dst = resizeLanczos(src, 2.0f);

        assert(dst.width == 16 && dst.height == 8);
        assert(dst.data.size() == 128);

        std::cout << "✓ PASSED\n";
    }

    // Run all tests
    int run_all_tests()
    {
        std::cout << "===========================================\n";
        std::cout << "  Lanczos Upscaler - Test Suite\n";
        std::cout << "===========================================\n\n";

        try
        {
            test_basic_upscale_2x();
            test_downscale_half();
            test_scale_identity();
            test_rgb_gradient();
            test_non_square_image();

            std::cout << "\n===========================================\n";
            std::cout << "  🎉 ALL TESTS PASSED!\n";
            std::cout << "===========================================\n";
            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "\n❌ TEST FAILED: " << e.what() << "\n";
            return 1;
        }
    }

} // namespace lanczos_test

#endif // __LANCZOS_TEST_HH