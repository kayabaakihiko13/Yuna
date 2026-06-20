#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <cmath>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// STB Image
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "include/utils/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/utils/stb_image_write.h"

// Local headers
#include "image_enhance/lanczos.hh"
#include "image_enhance/pde_super_resolution.hh"
#include "utils/constants.hh"
#include "utils/interactive.hh"
#include "utils/format_file_image.hh"
#include "gui/cli.hh"

static Image load_image(const std::string &path, int &w, int &h)
{
    int ch = 0;
    std::string normalized = path;
    for (char &c : normalized)
        if (c == '\\')
            c = '/';
    unsigned char *data = stbi_load(normalized.c_str(), &w, &h, &ch, 3);
    if (!data)
    {
        return {0, 0, {}};
    }
    Image img{w, h, {}};
    img.data.resize(w * h);
    for (int i = 0; i < w * h; ++i)
    {
        img.data[i] = {data[i * 3], data[i * 3 + 1], data[i * 3 + 2]};
    }
    stbi_image_free(data);
    return img;
}

static bool save_image(const std::string &path, const Image &img,
                       format_processing::ImageFormat fmt)
{
    std::vector<unsigned char> out(img.width * img.height * 3);
    for (int i = 0; i < img.width * img.height; ++i)
    {
        out[i * 3]     = img.data[i].r;
        out[i * 3 + 1] = img.data[i].g;
        out[i * 3 + 2] = img.data[i].b;
    }

    switch (fmt)
    {
    case format_processing::ImageFormat::PNG:
        return stbi_write_png(path.c_str(), img.width, img.height, 3,
                              out.data(), img.width * 3);
    case format_processing::ImageFormat::JPG:
        return stbi_write_jpg(path.c_str(), img.width, img.height, 3,
                              out.data(), 90);
    case format_processing::ImageFormat::BMP:
        return stbi_write_bmp(path.c_str(), img.width, img.height, 3,
                              out.data());
    default:
        interactive::log_warn("Format tidak didukung, fallback ke PNG");
        return stbi_write_png(path.c_str(), img.width, img.height, 3,
                              out.data(), img.width * 3);
    }
}

bool process_image()
{
    cli::Args args = cli::prompt();
    if (!args.is_valid)
    {
        return false;
    }

    format_processing::ImageFormat input_fmt = format_processing::detect_format(args.input_path);
    if (input_fmt == format_processing::ImageFormat::UNKNOWN)
    {
        interactive::log_warn("Format input tidak dikenali, mencoba load sebagai gambar...");
    }

    format_processing::ImageFormat output_fmt = format_processing::detect_format(args.output_path);
    if (output_fmt == format_processing::ImageFormat::UNKNOWN)
    {
        interactive::log_warn("Ekstensi output tidak dikenali, menggunakan format pilihan");
        output_fmt = args.format_ext;
        args.output_path = format_processing::ensure_extension(args.output_path, output_fmt);
    }

    format_processing::print_conversion_info(args.input_path, args.output_path);

    int w = 0, h = 0;
    Image src = load_image(args.input_path, w, h);
    if (src.data.empty())
    {
        interactive::log_error("Gagal Load: " + args.input_path);
        std::cerr << "Detail: " << (stbi_failure_reason() ? stbi_failure_reason() : "Unknown") << "\n";
        return false;
    }

    interactive::log_success("Loaded: " + std::to_string(w) + "x" + std::to_string(h) + " px");

    if (args.verbose)
    {
        std::cout << interactive::BOLD << "Source" << interactive::RESET << "\n";
        std::cout << "    " << src.width << "*" << src.height << "px\n" << std::endl;
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    interactive::log_info("Processing Lanczos Upscale ...");
    Image result = lanczos::resize(src, args.scale_factor,
                                   [verbose = args.verbose](int done, int total)
                                   {
                                       if (verbose)
                                           interactive::show_progress(done, total, "Resampling");
                                   });

    if (args.pde_enchance)
    {
        interactive::log_info("Applying PDE post-processing (Diffusion + Shock Filter)...");
        result = PDE_SR::enhance(result);
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    interactive::log_success("Done: " + std::to_string(result.width) + "x" + std::to_string(result.height) + " px");
    std::cout << "Time: " << ms << " ms\n\n";

    bool ok = save_image(args.output_path, result, output_fmt);
    if (!ok)
    {
        interactive::log_error("Gagal save: " + args.output_path);
        return false;
    }

    interactive::log_success("Saved: " + args.output_path + " [" + format_processing::to_string(output_fmt) + "]");
    std::cout << interactive::GREEN << "Success!" << interactive::RESET << "\n";
    return true;
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
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

        std::cout << "\n"
                  << interactive::BLUE << "*" << interactive::RESET
                  << " Process another image? [Y/n]: ";
        std::string confirm;
        std::getline(std::cin, confirm);

        if (confirm == "n" || confirm == "N" || confirm == "no" || confirm == "No")
        {
            std::cout << interactive::GREEN << "\n Thank you! Goodbye."
                      << interactive::RESET << "\n";
            break;
        }

        std::cout << "\n" << std::string(50, '-') << "\n\n";
    }
    return 0;
}
