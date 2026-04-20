#ifndef __UTILS_INTERACTIVE_HH
#define __UTILS_INTERACTIVE_HH
#include <iostream>
#include <string>
#include <functional>
// pragma once
#pragma once

namespace interactive
{

#ifndef NO_COLOR
    inline const std::string RESET = "\033[0m";
    inline const std::string BOLD = "\033[1m";
    inline const std::string GREEN = "\033[32m";
    inline const std::string YELLOW = "\033[33m";
    inline const std::string RED = "\033[31m";
    inline const std::string CYAN = "\033[36m";
    inline const std::string BLUE = "\033[34m";
#else
    inline const std::string RESET = "", BOLD = "", GREEN = "", YELLOW = "", RED = "", CYAN = "", BLUE = "";
#endif

    // progress bar
    inline void show_progress(int current, int total, const std::string &label = "")
    {
        const int width = 40;
        float pct = static_cast<float>(current) / std::max(total, 1);
        int pos = static_cast<int>(width * pct);

        std::cout << "\r" << CYAN << "[" << RESET;
        for (int i = 0; i < width; ++i)
        {
            if (i < pos)
                std::cout << GREEN << "O" << RESET;
            else if (i == pos && current < total)
                std::cout << YELLOW << ">" << RESET;
            else
                std::cout << " ";
        }
        std::cout << CYAN << "] " << RESET
                  << static_cast<int>(pct * 100) << "%";
        if (!label.empty())
            std::cout << " " << label;
        std::cout.flush();
        if (current >= total)
            std::cout << "\n";
    }

    inline void log_info(const std::string &msg)
    {
        std::string info_symbol = "[ℹ]";
        std::cout << CYAN << info_symbol << RESET << msg << "\n";
    }
    inline void log_success(const std::string &msg)
    {
        std::string info_symbol = "[✓]";
        std::cout << GREEN << info_symbol << RESET << msg << "\n";
    }
    inline void log_warn(const std::string &msg)
    {
        std::string info_symbol = "[!]";
        std::cout << GREEN << info_symbol << RESET << msg << "\n";
    }
    inline void log_error(const std::string &msg)
    {
        std::string infoSymbol = "[✗] ";
        std::cerr << RED << infoSymbol << RESET << msg << "\n";
    }
} // namespace interative

#endif