#ifndef GUI_HH
#define GUI_HH

#include <iostream>
#include <cstdlib>

inline void cmd_clear()
{
    // Skip clear jika di CI environment
    if (std::getenv("CI") != nullptr)
    {
        return;
    }

#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

inline void cmd_help()
{
    std::cout << "Available Commands:\n"
              << "  help/h     - Show this message\n"
              << "  clear/cls  - Clear screen\n"
              << "  greet      - Say hello\n"
              << "  exit/x     - Close program\n";
}

inline void cmd_greet()
{
    std::cout << "Hello, user! This is beta test.\n";
}

#endif