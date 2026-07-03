#ifdef _WIN32
#define NOMINMAX
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "lanczoc_test.hh"

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    return lanczos_test::run_all_tests();
}