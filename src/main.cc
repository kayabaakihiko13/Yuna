#include <iostream>
#include <string_view>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>

/**
 * import internal header file
 *
 */
#include "include/gui.hh"

int main(int argc, char *argv[])
{
    std::cout << "Simple C++ CLI v1.0\n Type 'help' for commands.\n\n";
    std::string line;
    bool running = true;
    // while loop
    while (running)
    {
        std::cout << "> ";
        std::getline(std::cin, line);
        if (line.empty())
            continue;
        // split user input into command + argument
        std::istringstream iss(line);
        std::vector<std::string> args;
        std::string cmd;
        iss >> cmd;
        std::string arg;

        while (iss >> arg)
            args.push_back(arg);
        // route command
        if (cmd == "help")
            cmd_help();
        else if (cmd == "greet")
            cmd_greet();

        else if (cmd == "exit")
        {
            running = false;
            cmd_clear();
        }
        else
            std::cerr << "Unknown command:" << cmd << "\n";
    }
    return 0;
}