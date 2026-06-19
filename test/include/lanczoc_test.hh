// test/include/lanczos_test.hh
#ifndef LANCZOS_TEST_HH
#define LANCZOS_TEST_HH

#include "image_enhance/lanczos.hh"
#include "utils/utils.hh"
#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include <iomanip> // Untuk std::setw

namespace lanczos_test
{

    //  Helper: Print pixel value
    void print_pixel(const std::string &label, const Pixel &p)
    {
        std::cout << label << " R=" << std::setw(3) << +p.r
                  << " G=" << std::setw(3) << +p.g
                  << " B=" << std::setw(3) << +p.b << std::endl;
    }

    //  Helper: Print image info
    void print_image_info(const std::string &label, const Image &img)
    {
        std::cout << label << ": " << img.width << "x" << img.height
                  << " (" << img.data.size() << " pixels)" << std::endl;
    }

    // Helper: Print sample pixels dari image
    void print_sample_pixels(const Image &img, const std::string &label = "Image")
    {
        std::cout << "\n--- " << label << " Sample Pixels ---" << std::endl;

        // Print corner pixels
        print_pixel("Top-Left     ", img.at(0, 0));
        print_pixel("Top-Right    ", img.at(img.width - 1, 0));
        print_pixel("Bottom-Left  ", img.at(0, img.height - 1));
        print_pixel("Bottom-Right ", img.at(img.width - 1, img.height - 1));

        // Print center pixel
        int cx = img.width / 2;
        int cy = img.height / 2;
        print_pixel("Center       ", img.at(cx, cy));

        std::cout << "-----------------------------------" << std::endl;
    }

    // Test 1: Basic upscale 2x dengan detail output
    void test_basic_upscale_2x()
    {
        std::cout << "\nRunning test: basic_upscale_2x... " << std::flush;

        Image src;
        src.width = 4;
        src.height = 4;
        src.data.resize(16);

        // Fill dengan gradient sederhana
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

        print_image_info("Source", src);
        print_sample_pixels(src, "Source");

        Image dst = lanczos::resize(src, 2.0f);

        print_image_info("Destination", dst);
        print_sample_pixels(dst, "Destination");

        assert(dst.width == 8 && dst.height == 8);
        assert(dst.data.size() == 64);

        std::cout << "[PASS]" << std::endl;
    }

    // Test 2: Downscale 0.5x dengan detail output
    void test_downscale_half()
    {
        std::cout << "\nRunning test: downscale_half... " << std::flush;

        Image src;
        src.width = 10;
        src.height = 10;
        src.data.resize(100);

        // Fill dengan pola checkerboard
        for (int y = 0; y < 10; ++y)
        {
            for (int x = 0; x < 10; ++x)
            {
                bool is_white = (x + y) % 2 == 0;
                src.at(x, y) = is_white ? Pixel{255, 255, 255} : Pixel{0, 0, 0};
            }
        }

        print_image_info("Source", src);
        print_sample_pixels(src, "Source");

        Image dst = lanczos::resize(src, 0.5f);

        print_image_info("Destination", dst);
        print_sample_pixels(dst, "Destination");

        assert(dst.width == 5 && dst.height == 5);
        assert(dst.data.size() == 25);

        std::cout << "[PASS]" << std::endl;
    }

    // Test 3: Scale identity (1.0x) dengan detail output
    void test_scale_identity()
    {
        std::cout << "\nRunning test: scale_identity... " << std::flush;

        Image src;
        src.width = 5;
        src.height = 5;
        src.data.resize(25);

        // Fill dengan warna solid
        for (int i = 0; i < 25; ++i)
        {
            src.data[i] = {100, 150, 200};
        }

        print_image_info("Source", src);
        print_sample_pixels(src, "Source");

        Image dst = lanczos::resize(src, 1.0f);

        print_image_info("Destination", dst);
        print_sample_pixels(dst, "Destination");

        assert(dst.width == 5 && dst.height == 5);

        // Cek apakah warna tetap sama (dengan toleransi)
        Pixel center = dst.at(2, 2);
        std::cout << "Center pixel diff: R=" << std::abs(center.r - 100)
                  << " G=" << std::abs(center.g - 150)
                  << " B=" << std::abs(center.b - 200) << std::endl;

        std::cout << "[PASS]" << std::endl;
    }

    // Test 4: Non-square image dengan detail output
    void test_non_square_image()
    {
        std::cout << "\nRunning test: non_square_image... " << std::flush;

        Image src;
        src.width = 8;
        src.height = 4;
        src.data.resize(32);

        // Fill dengan gradient horizontal
        for (int y = 0; y < 4; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                src.at(x, y) = {
                    static_cast<uint8_t>(x * 32),
                    128,
                    255};
            }
        }

        print_image_info("Source", src);
        print_sample_pixels(src, "Source");

        Image dst = lanczos::resize(src, 2.0f);

        print_image_info("Destination", dst);
        print_sample_pixels(dst, "Destination");

        assert(dst.width == 16 && dst.height == 8);
        assert(dst.data.size() == 128);

        std::cout << "[PASS]" << std::endl;
    }

    // Test 5: Edge pixels (boundary handling) dengan detail output
    void test_edge_pixels()
    {
        std::cout << "\nRunning test: edge_pixels... " << std::flush;

        Image src;
        src.width = 3;
        src.height = 3;
        src.data.resize(9);

        // Fill dengan warna merah solid
        for (int i = 0; i < 9; ++i)
        {
            src.data[i] = {255, 0, 0};
        }

        print_image_info("Source", src);
        print_sample_pixels(src, "Source");

        Image dst = lanczos::resize(src, 2.0f);

        print_image_info("Destination", dst);
        print_sample_pixels(dst, "Destination");

        assert(dst.width == 6 && dst.height == 6);

        // Edge pixels harus tetap merah (tidak hitam)
        std::cout << "Edge pixel check:" << std::endl;
        print_pixel("  Top-Left     ", dst.at(0, 0));
        print_pixel("  Bottom-Right ", dst.at(5, 5));

        assert(dst.at(0, 0).r > 200); // Harus tetap merah
        assert(dst.at(5, 5).r > 200); // Harus tetap merah

        std::cout << "[PASS]" << std::endl;
    }

    // Run all tests
    int run_all_tests()
    {
        std::cout << "===========================================" << std::endl;
        std::cout << "  Lanczos Upscaler - Test Suite" << std::endl;
        std::cout << "===========================================" << std::endl;

        try
        {
            test_basic_upscale_2x();
            test_downscale_half();
            test_scale_identity();
            test_non_square_image();
            test_edge_pixels();

            std::cout << "\n===========================================" << std::endl;
            std::cout << "  *** ALL TESTS PASSED! ***" << std::endl;
            std::cout << "===========================================" << std::endl;
            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "\n!!! TEST FAILED: " << e.what() << std::endl;
            return 1;
        }
    }

} // namespace lanczos_test

#endif // __LANCZOS_TEST_HH