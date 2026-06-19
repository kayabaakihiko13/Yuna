#define STB_IMAGE_IMPLEMENTATION
#include "../src/include/utils/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../src/include/utils/stb_image_write.h"

#include "../src/include/image_enhance/lanczos.hh"
#include "../src/include/utils/constants.hh"
#include "../src/include/utils/utils.hh"

#include <iostream>
#include <chrono>
#include <string>
#include <vector>

int main(int argc, char *argv[])
{
    std::string input_path = "../docs/images/65056cf873ff6c1ac9240af3bfe4d8c6.jpg";
    std::string output_path = "../docs/images/output_lanczos.jpg";
    float scale_factor = 4.0f;

    if (argc > 1)
        input_path = argv[1];
    if (argc > 2)
        scale_factor = std::stof(argv[2]);
    if (argc > 3)
        output_path = argv[3];

    std::cout << "Lanczos Upscaler\n";
    std::cout << "==================\n";
    std::cout << "Input : " << input_path << "\n";
    std::cout << "Scale : " << scale_factor << "x\n";
    std::cout << "Output: " << output_path << "\n\n";

    int width, height, channels;
    unsigned char *img_data = stbi_load(input_path.c_str(), &width, &height, &channels, 3);
    if (!img_data)
    {
        std::cerr << "ERROR: Gagal load gambar '" << input_path << "'\n";
        std::cerr << "   Detail: " << stbi_failure_reason() << "\n";
        return 1;
    }
    std::cout << "Loaded: " << width << "x" << height << " px\n";

    Image src;
    src.width = width;
    src.height = height;
    src.data.resize(width * height);
    for (int i = 0; i < width * height; ++i)
    {
        src.data[i] = {img_data[i * 3], img_data[i * 3 + 1], img_data[i * 3 + 2]};
    }
    stbi_image_free(img_data);

    std::cout << "Processing Lanczos resampling...\n";
    auto start = std::chrono::high_resolution_clock::now();
    Image result = lanczos::resize(src, scale_factor);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Done! Result: " << result.width << "x" << result.height << " px\n";
    std::cout << "Time: " << duration.count() << " ms\n\n";

    std::vector<unsigned char> out_data(result.width * result.height * 3);
    for (int i = 0; i < result.width * result.height; ++i)
    {
        out_data[i * 3]     = result.data[i].r;
        out_data[i * 3 + 1] = result.data[i].g;
        out_data[i * 3 + 2] = result.data[i].b;
    }

    if (!stbi_write_png(output_path.c_str(), result.width, result.height, 3,
                        out_data.data(), result.width * 3))
    {
        std::cerr << "ERROR: Gagal save ke '" << output_path << "'\n";
        return 1;
    }

    std::cout << "Saved: " << output_path << "\n";
    std::cout << "Success!\n";
    return 0;
}