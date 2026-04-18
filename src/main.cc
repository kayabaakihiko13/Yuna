#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <cmath>
#include <cctype>

// STB Image
#define STB_IMAGE_IMPLEMENTATION
#include "include/utils/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/utils/stb_image_write.h"

// Local headers
#include "image_enchace/lanczoc.hh"
#include "utils/constants.hh"
#include "utils/utils.hh"

void print_usage(const char *prog_name)
{
    std::cout << "\n🚀 Lanczos Upscaler\n"
              << "Usage: " << prog_name << " [options]\n\n"
              << "Options:\n"
              << "  -i, --input <path>     Input image path (required)\n"
              << "  -o, --output <path>    Output image path [default: output_lanczos.png]\n"
              << "  -s, --scale <value>    Scale: 2.0, 4.0, 8.0 [default: 4.0]\n"
              << "  -v, --verbose          Enable verbose output\n"
              << "  -h, --help             Show this help\n\n"
              << "Or positional: " << prog_name << " <input> [scale] [output]\n\n"
              << "Examples:\n"
              << "  " << prog_name << " -i input.jpg -o out.png -s 4.0 -v\n"
              << "  " << prog_name << " input.jpg 4.0 output.png\n\n";
}

bool is_valid_scale(float s)
{
    const float scales[] = {2.0f, 4.0f, 8.0f};
    for (float valid : scales)
    {
        if (std::abs(s - valid) < 0.001f)
            return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    // === Default Parameters ===
    std::string input_path = "../docs/images/65056cf873ff6c1ac9240af3bfe4d8c6.jpg";
    std::string output_path = "output_lanczos.png";
    float scale_factor = 4.0f;
    bool verbose = false;

    // === Parse CLI (Flags + Positional) ===
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            print_usage(argv[0]);
            return 0;
        }
        else if (arg == "-v" || arg == "--verbose")
        {
            verbose = true;
        }
        else if ((arg == "-i" || arg == "--input") && i + 1 < argc)
        {
            input_path = argv[++i];
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc)
        {
            output_path = argv[++i];
        }
        else if ((arg == "-s" || arg == "--scale") && i + 1 < argc)
        {
            try
            {
                scale_factor = std::stof(argv[++i]);
                if (!is_valid_scale(scale_factor))
                {
                    std::cerr << "Error: Scale harus 2.0, 4.0, atau 8.0\n";
                    print_usage(argv[0]);
                    return 1;
                }
            }
            catch (...)
            {
                std::cerr << "Error: Scale harus angka\n";
                return 1;
            }
        }
        else if (arg.find('-') != 0)
        {
            // Positional fallback
            if (input_path == "../docs/images/65056cf873ff6c1ac9240af3bfe4d8c6.jpg")
            {
                input_path = arg;
            }
            else if (output_path == "output_lanczos.png")
            {
                output_path = arg;
            }
        }
    }

    // === Validate ===
    if (input_path.empty())
    {
        std::cerr << "❌ Error: Input path is required\n";
        print_usage(argv[0]);
        return 1;
    }

    // === Info Output ===
    std::cout << "Lanczos Upscale\n";
    std::cout << "==================\n";
    std::cout << "Input : " << input_path << "\n";
    std::cout << "Scale : " << scale_factor << "x\n";
    std::cout << "Output: " << output_path << "\n";
    if (verbose)
        std::cout << "Mode  : Verbose\n";
    std::cout << "\n";
    std::cout.flush();

    // === 1. LOAD GAMBAR ===
    int width = 0, height = 0, channels = 0;

    std::string safe_path = input_path;
    for (char &c : safe_path)
        if (c == '\\')
            c = '/';

    unsigned char *image_data = stbi_load(safe_path.c_str(), &width, &height, &channels, 3);

    if (!image_data)
    {
        std::cerr << "Error: Gagal Load Gambar: " << input_path << "\n";
        std::cerr << "Detail: " << (stbi_failure_reason() ? stbi_failure_reason() : "Unknown") << "\n";
        std::cerr.flush();
        return 1;
    }

    std::cout << "Loaded: " << width << "x" << height << " px\n";

    // === 2. Conversion to struct Image ===
    Image src;
    src.width = width;
    src.height = height;
    src.data.resize(width * height);

    for (int i = 0; i < width * height; ++i)
    {
        src.data[i] = {
            image_data[i * 3],
            image_data[i * 3 + 1],
            image_data[i * 3 + 2]};
    }
    stbi_image_free(image_data);

    // === 3. Processing Resize Lanczos ===
    if (verbose)
        std::cout << "Processing Lanczos ReSampling...\n";
    std::cout.flush();

    auto start = std::chrono::high_resolution_clock::now();
    Image result = resizeLanczos(src, scale_factor);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Done! Result: " << result.width << "x" << result.height << " px\n";
    std::cout << "Time: " << duration.count() << " ms\n\n";

    // === 4. Conversion result to array ===
    std::vector<unsigned char> saving_array(result.width * result.height * 3);
    for (int i = 0; i < result.width * result.height; ++i)
    {
        saving_array[i * 3] = result.data[i].r;
        saving_array[i * 3 + 1] = result.data[i].g;
        saving_array[i * 3 + 2] = result.data[i].b;
    }

    // === 5. Save Image ===
    bool save_success = false;
    std::string lower_output = output_path;
    for (char &c : lower_output)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (lower_output.find(".png") != std::string::npos)
    {
        save_success = stbi_write_png(output_path.c_str(), result.width, result.height, 3,
                                      saving_array.data(), result.width * 3);
    }
    else
    {
        save_success = stbi_write_jpg(output_path.c_str(), result.width, result.height, 3,
                                      saving_array.data(), 90);
    }

    if (!save_success)
    {
        std::cerr << "ERROR: Gagal save ke '" << output_path << "'\n";
        std::cerr.flush();
        return 1;
    }

    std::cout << "Output Saved on: " << output_path << "\n";
    std::cout << "Success!\n";

    return 0;
}