#ifndef __CLI_HH
#define __CLI_HH

#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include "utils/constants.hh"
#include "utils/interactive.hh"

namespace cli
{
    struct Args
    {
        float scale_factor = 2.0f;
        bool verbose = false;
        bool is_valid = false;
        std::string input_path;
        std::string output_path = "output_lanczos.png";
    };

    inline bool is_valid_scale(float scale)
    {
        for (int i = 0; i < LANCZOS_RADIUS; ++i)
        {
            if (std::abs(SCALES[i] - scale) < 0.0001f)
                return true;
        }
        return false;
    }

    inline std::string get_scale_options()
    {
        std::ostringstream oss;
        for (int i = 0; i < LANCZOS_RADIUS; ++i)
        {
            oss << SCALES[i];
            if (i < LANCZOS_RADIUS - 1)
                oss << ", ";
        }
        return oss.str();
    }

    inline Args prompt()
    {
        Args args;
        std::string input;

        // Header
        std::cout << "\n"
                  << interactive::BOLD << interactive::CYAN
                  << "Lanczos Image Upscaler" << interactive::RESET << "\n";
        std::cout << std::string(50, '=') << "\n\n";

        // ── 1. Input Path (FIX: tambah continue!) ──
        while (true)
        {
            std::cout << interactive::BLUE << "*" << interactive::RESET
                      << " Input image path: ";
            std::getline(std::cin, input);

            if (input.empty())
            {
                interactive::log_warn("Path tidak boleh kosong. Coba lagi.");
                continue; // ← FIX: ulang loop, jangan break!
            }
            args.input_path = input;
            break;
        }

        // ── 2. Scale Factor ──
        std::cout << "\n"
                  << interactive::BLUE << "📐" << interactive::RESET
                  << " Available scales: " << interactive::GREEN
                  << get_scale_options() << interactive::RESET << "\n";

        while (true)
        {
            std::cout << "   Choose scale [2/4/8, default=2]: ";
            std::getline(std::cin, input);

            if (input.empty())
            {
                args.scale_factor = 2.0f;
                break;
            }
            try
            {
                float val = std::stof(input);
                if (is_valid_scale(val))
                {
                    args.scale_factor = val;
                    break;
                }
                else
                {
                    interactive::log_warn("Scale '" + input + "' tidak didukung.");
                }
            }
            catch (...)
            {
                interactive::log_warn("Input tidak valid. Masukkan angka (2/4/8).");
            }
        }

        // ── 3. Output Path ──
        std::cout << "\n"
                  << interactive::BLUE << "💾" << interactive::RESET
                  << " Output path [default: output_lanczos.png]: ";
        std::getline(std::cin, input);
        if (!input.empty())
        {
            args.output_path = input;
        }

        // ── 4. Verbose Mode ──
        std::cout << "\n"
                  << interactive::BLUE << " Enable verbose output? [y/N]: "
                  << interactive::RESET;
        std::getline(std::cin, input);
        args.verbose = (input == "y" || input == "Y" || input == "yes");

        // ── 5. Summary & Confirm ──
        std::cout << "\n"
                  << interactive::BOLD << "Configuration Summary:" << interactive::RESET << "\n";
        std::cout << "   * Input:  " << interactive::GREEN << args.input_path << interactive::RESET << "\n";
        std::cout << "   * Scale:  " << interactive::GREEN << args.scale_factor << "x" << interactive::RESET << "\n";
        std::cout << "   * Output: " << interactive::GREEN << args.output_path << interactive::RESET << "\n";
        if (args.verbose)
            std::cout << "   * Mode:   " << interactive::GREEN << "Verbose" << interactive::RESET << "\n";

        std::cout << "\n"
                  << interactive::BOLD << ">" << interactive::RESET
                  << interactive::YELLOW << "> Start processing? [Y/n]: " << interactive::RESET;
        std::getline(std::cin, input);

        if (input.empty() || input == "y" || input == "Y")
        {
            args.is_valid = true;
            std::cout << "\n"
                      << interactive::GREEN << "Starting..." << interactive::RESET << "\n\n";
        }
        else
        {
            std::cout << interactive::YELLOW << "Cancelled by user." << interactive::RESET << "\n";
            args.is_valid = false;
        }

        return args;
    }

} // namespace cli

#endif