// test/lanczos_tests.cc
#include "lanczoc_test.hh"
#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
// Set console output ke UTF-8 (Windows only)
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    // Run all tests
    return lanczos_test::run_all_tests();
}