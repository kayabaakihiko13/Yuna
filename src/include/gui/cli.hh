#ifndef __CLI_HH
#define __CLI_HH

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "utils/constants.hh"

namespace cli
{
    struct args
    {
        float scale_factor = 2.0f; // default 2.0
        bool help = false;
        bool verbose = false;
        std::string input_path;
        std::string output_path;
    };

    inline bool is_valid_scale(float scale)
    {
        for (int i = 0; i < SCALE_COUNT; ++i)
        {
            if (std::abs(SCALES[i] - scale) < 0.0001f)
            {
                return true;
            }
        }
        return false;
    }

    inline std::string get_scale_options()
    {
        std::ostringstream oss; // saving input user
        for (int i = 0; i < SCALE_COUNT; ++i)
        {
            oss << SCALES[i];
            if (i < SCALE_COUNT - 1)
                oss << ", ";
        }
        return oss.str();
    }

    inline args parse(int argc, char *argv[])
    {
        args a;

        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            // Help flag
            if (arg == "-h" || arg == "--help")
            {
                a.help = true;
            }
            // Verbose flag
            else if (arg == "-v" || arg == "--verbose")
            {
                a.verbose = true;
            }
            // Input path
            else if ((arg == "-i" || arg == "--input") && i + 1 < argc)
            {
                a.input_path = argv[++i];
            }
            // Output path (user-defined)
            else if ((arg == "-o" || arg == "--output") && i + 1 < argc)
            {
                a.output_path = argv[++i];
            }
            // Scale factor (restricted to 2.0, 4.0, 8.0)
            else if ((arg == "-s" || arg == "--scale") && i + 1 < argc)
            {
                try
                {
                    float val = std::stof(argv[++i]);
                    if (is_valid_scale(val))
                    {
                        a.scale_factor = val;
                    }
                    else
                    {
                        std::cerr << "❌ Error: Scale '" << val << "' tidak didukung.\n";
                        std::cerr << "   Pilihan yang valid: " << get_scale_options() << "\n";
                        a.help = true;
                    }
                }
                catch (...)
                {
                    std::cerr << "❌ Error: Invalid scale value\n";
                    a.help = true;
                }
            }
            // Unknown option
            else if (arg.find('-') == 0)
            {
                std::cerr << "⚠️  Unknown option: " << arg << "\n";
                a.help = true;
            }
            // Positional argument as input
            else if (a.input_path.empty())
            {
                a.input_path = arg;
            }
        }

        return a;
    }

    inline void print_help(const char *prog_name)
    {
        std::cout << "\n🖼️  Lanczos Image Upscaler\n"
                     "Usage: "
                  << prog_name << " [options]\n\n"
                                  "Options:\n"
                                  "  -i, --input <path>     Input image path (required)\n"
                                  "  -o, --output <path>    Output image path [default: ./output.jpg]\n"
                                  "  -s, --scale <value>    Scale factor: "
                  << get_scale_options() << " [default: 2.0]\n"
                                            "  -v, --verbose          Enable verbose output\n"
                                            "  -h, --help             Show this help message\n\n"
                                            "Examples:\n"
                                            "  "
                  << prog_name << " -i input.jpg -o out.png -s 4.0\n"
                                  "  "
                  << prog_name << " --input photo.jpg --scale 8.0 -v\n"
                                  "  "
                  << prog_name << " in.jpg -s 2.0  # positional input\n\n";
    }

    inline bool validate(const args &a)
    {
        if (a.input_path.empty())
        {
            std::cerr << "❌ Error: Input path is required\n";
            return false;
        }
        // Scale sudah divalidasi saat parse, tapi double-check aja
        if (!is_valid_scale(a.scale_factor))
        {
            std::cerr << "❌ Error: Invalid scale factor. Pilih: " << get_scale_options() << "\n";
            return false;
        }
        return true;
    }

} // namespace cli
#endif