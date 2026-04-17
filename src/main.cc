#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

// Helper: trim whitespace
std::string trim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// Helper: to lowercase
std::string to_lower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

int main()
{
    std::cout << "Simple C++ CLI v1.0\nType 'help' for commands.\n\n";

    std::string line;
    bool running = true;

    while (running)
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        line = trim(line);
        if (line.empty())
            continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        cmd = to_lower(cmd);

        std::vector<std::string> args;
        std::string arg;
        while (iss >> arg)
        {
            args.push_back(arg);
        }

        // Route commands
        if (cmd == "help" || cmd == "h")
        {
            cmd_help();
        }
        else if (cmd == "clear" || cmd == "cls")
        {
            cmd_clear();
        }
        else if (cmd == "greet")
        {
            cmd_greet();
        }
        else if (cmd == "exit" || cmd == "x" || cmd == "quit")
        {
            std::cout << "Goodbye!\n";
            running = false;
        }
        else
        {
            std::cerr << "Unknown command: " << cmd << "\n";
            std::cerr << "Type 'help' for available commands.\n";
        }
    }

    return 0;
}
