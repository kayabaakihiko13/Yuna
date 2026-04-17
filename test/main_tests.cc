// test/lanczos_tests.cc
#include "lanczoc_test.hh"
#include <iostream>
#include <cassert>

int main()
{
    // Buat gambar dummy 4x4 RGB
    Image src;
    src.width = 4;
    src.height = 4;
    src.data.resize(16);

    // Isi dengan pola checkerboard sederhana
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            bool is_black = (x + y) % 2 == 0;
            src.at(x, y) = is_black ? Pixel{0, 0, 0} : Pixel{255, 255, 255};
        }
    }

    // Test upscale 2x
    Image dst = resizeLanczos(src, 2.0f);

    // Verifikasi dimensi
    assert(dst.width == 8 && dst.height == 8);

    std::cout << "✓ Lanczos test passed: 4x4 → 8x8\n";
    std::cout << "✓ Center pixel: R=" << +dst.at(4, 4).r
              << " G=" << +dst.at(4, 4).g
              << " B=" << +dst.at(4, 4).b << "\n";

    return 0;
}