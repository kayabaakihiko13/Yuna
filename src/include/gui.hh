#ifndef GUI_HH
#define GUI_HH

#include <iostream>
#include <vector>
inline void cmd_clear()
{
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void cmd_help()
{
    std::cout << "Hello, Welcome to my CLI,Avaiable Commands\n"
              << "Help or (h) - Show this message\n"
              << "Exit or (x) - comamnd cli to close program\n";
}

void cmd_greet()
{
    std::cout << "Hello user,this is beta test";
}

#endif