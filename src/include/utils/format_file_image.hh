#ifndef __UTILS_FORMAT_FILE_IMAGE_HH
#define __UTILS_FORMAT_FILE_IMAGE_HH

#pragma once
#include <string>
#include <algorithm>
#include "utils/interactive.hh"
namespace format_processing
{
    enum class ImageFormat
    {
        PNG,
        JPG,
        BMP,
        UNKNOWN
    };
    inline ImageFormat detect_format(const std::string &path)
    {
        std::string lower = path;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });

        if (lower.find(".png") != std::string::npos)
            return ImageFormat::PNG;
        if (lower.find(".jpg") != std::string::npos ||
            lower.find(".jpeg") != std::string::npos)
            return ImageFormat::JPG;
        if (lower.find(".bmp") != std::string::npos)
            return ImageFormat::BMP;

        return ImageFormat::UNKNOWN;
    }

    // converting result from ImageFormat enum to string
    inline std::string to_string(ImageFormat fmt)
    {
        switch (fmt)
        {
        case ImageFormat::PNG:
            return "PNG";
        case ImageFormat::JPG:
            return "JPG";
        case ImageFormat::BMP:
            return "BMP";
        default:
            return "UNKNOWN";
        }
    }

    // generate output path with correted format file
    inline std::string ensure_extension(const std::string &path, ImageFormat target)
    {
        std::string result = path;
        ImageFormat current = detect_format(path);

        if (current != target)
        {
            size_t dot = result.find_last_of(".");
            if (dot != std::string::npos)
            {
                result = result.substr(0, dot);
            }
            switch (target)
            {
            case ImageFormat::PNG:
                result += ".png";
                break;
            case ImageFormat::JPG:
                result += ".jpg";
                break;
            case ImageFormat::BMP:
                result += ".bmp";
                break;
            default:
                break;
            }
        }
        return result;
    }

    // make convertion formating for users
    inline void print_conversion_info(const std::string &input_path,
                                      const std::string &output_path)
    {
        ImageFormat in_fmt = detect_format(input_path);
        ImageFormat out_fmt = detect_format(output_path);

        if (in_fmt != out_fmt && in_fmt != ImageFormat::UNKNOWN && out_fmt != ImageFormat::UNKNOWN)
        {
            std::cout << interactive::CYAN << "*" << interactive::RESET
                      << "Format conversion: "
                      << interactive::YELLOW << to_string(in_fmt) << interactive::RESET
                      << " → "
                      << interactive::GREEN << to_string(out_fmt) << interactive::RESET
                      << "\n";
        }
    }

}

#endif // format_file_image