#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <cmath>
#include <functional> // ← Untuk std::function

// STB Image
#define STB_IMAGE_IMPLEMENTATION
#pragma GCC diagno
#include "include/utils/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/utils/stb_image_write.h"

// Local headers
#include "image_enchace/lanczoc.hh"
#include "utils/constants.hh"
#include "utils/interactive.hh"
#include "utils/format_file_image.hh"
#include "gui/cli.hh"

// using Progress callback type
using ProgressCallback = std::function<void(int, int)>;

bool process_image()
{
    // intial prompt gui
    cli::Args args = cli::prompt();
    if (!args.is_valid)
    {
        return false;
    }
    // detect and validation user input format
    format_processing::ImageFormat input_fmt = format_processing::detect_format(args.input_path);
    if (input_fmt == format_processing::ImageFormat::UNKNOWN)
    {
        interactive::log_warn("Format input tidak dikenali, mencoba load sebagai gambar...");
    }
    format_processing::ImageFormat output_fmt = format_processing::detect_format(args.output_path);
    if (output_fmt == format_processing::ImageFormat::UNKNOWN)
    {
        // Default ke PNG jika ekstensi tidak jelas
        interactive::log_warn("Ekstensi output tidak dikenali, menggunakan .png");
        args.output_path = format_processing::ensure_extension(args.output_path, format_processing::ImageFormat::PNG);
        output_fmt = format_processing::ImageFormat::PNG;
    }
    // show up info conversion
    format_processing::print_conversion_info(args.input_path, args.output_path);
    // intial load image
    int w = 0, h = 0, ch = 0;
    std::string path = args.input_path;
    for (char &c : path)
        if (c == '\\')
            c = '/';
    unsigned char *data = stbi_load(path.c_str(), &w, &h, &ch, 3);
    if (!data)
    {
        interactive::log_error("Gagal Load:" + args.input_path);
        std::cerr << "Datail:" << (stbi_failure_reason() ? stbi_failure_reason() : "Unknown") << "\n";
        return false;
    }
    interactive::log_success("Loaded: " + std::to_string(w) + "x" + std::to_string(h) + " px");

    // convert raw data to Image Struct
    Image src{w, h, {}};
    src.data.resize(w * h);
    for (int i = 0; i < w * h; ++i)
    {
        src.data[i] = {data[i * 3], data[i * 3 + 1], data[i * 3 + 2]};
    }
    // free memory variabel on src
    stbi_image_free(data);
    if (args.verbose)
    {
        std::cout << interactive::BOLD << "Source" << interactive::RESET << "\n";
        std::cout << "    " << src.width << "*" << src.height << "px\n"
                  << std::endl;
    }
    // processing image
    interactive::log_info("Processing Lanzoos Upscale ...");
    auto t1 = std::chrono::high_resolution_clock::now();
    Image result = resizeLanczos(src, args.scale_factor,
                                 [verbose = args.verbose](int done, int total)
                                 {
                                     if (verbose)
                                         interactive::show_progress(done, total, "Resampling");
                                 });

    auto t2 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    interactive::log_success("Done: " + std::to_string(result.width) + "x" + std::to_string(result.height) + " px");
    std::cout << "⏱️  Time: " << ms << " ms\n\n";

    // saving and print output
    std::vector<unsigned char> out(result.width * result.height * 3);
    for (int i = 0; i < result.width * result.height; ++i)
    {
        out[i * 3] = result.data[i].r;
        out[i * 3 + 1] = result.data[i].g;
        out[i * 3 + 2] = result.data[i].b;
    }
    std::string out_lower = args.output_path;
    for (char &c : out_lower)
        c = std::tolower(static_cast<unsigned char>(c));
    bool ok = false;
    switch (output_fmt)
    {
    case format_processing::ImageFormat::PNG:
        ok = stbi_write_png(args.output_path.c_str(), result.width, result.height, 3,
                            out.data(), result.width * 3);
        break;
    case format_processing::ImageFormat::JPG:
        ok = stbi_write_jpg(args.output_path.c_str(), result.width, result.height, 3,
                            out.data(), 90); // Quality 90
        break;
    case format_processing::ImageFormat::BMP:
        ok = stbi_write_bmp(args.output_path.c_str(), result.width, result.height, 3,
                            out.data());
        break;
    default:
        // Fallback ke PNG
        interactive::log_warn("Format tidak didukung, fallback ke PNG");
        ok = stbi_write_png(args.output_path.c_str(), result.width, result.height, 3,
                            out.data(), result.width * 3);
    }

    if (!ok)
    {
        interactive::log_error("Gagal save: " + args.output_path);
        return false;
    }
    interactive::log_success("Saved: " + args.output_path + " [" + format_processing::to_string(output_fmt) + "]");
    std::cout << interactive::GREEN << "Success!" << interactive::RESET << "\n";
    return true;
    //
}

// in this code
int main()
{
    std::cout << interactive::CYAN << interactive::BOLD
              << "\n Welcome to Lanczos Upscaler!"
              << interactive::RESET << std::endl
              << std::endl;
    while (true)
    {
        bool success = process_image();

        if (!success)
        {
            std::cout << interactive::YELLOW << "\nProcess failed or cancelled."
                      << interactive::RESET << "\n";
        }

        // section for break app or continue using
        std::cout << "\n"
                  << interactive::BLUE << "*" << interactive::RESET
                  << " Process another image? [Y/n]: ";
        std::string confirm;
        std::getline(std::cin, confirm);

        // Jika user ketik 'n' atau 'no', exit loop
        if (confirm == "n" || confirm == "N" || confirm == "no" || confirm == "No")
        {
            std::cout << interactive::GREEN << "\n Thank you! Goodbye."
                      << interactive::RESET << "\n";
            break;
        }

        // Jika 'y' atau kosong, lanjut loop
        std::cout << "\n"
                  << std::string(50, '-') << "\n\n";
    }
    return 0;
}
