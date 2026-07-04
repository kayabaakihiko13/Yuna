#define NOMINMAX
#include <windows.h>
#include <CommCtrl.h>

#pragma comment(lib,"comctl32.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "include/utils/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/utils/stb_image_write.h"
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#include "gui/window.hh"
#include "resource.hh"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_PROGRESS_CLASS };
    ::InitCommonControlsEx(&icc);

    try {
        yuna::gui::Window window(hInstance, nCmdShow);
        return window.Run();
    } catch (const std::exception& e) {
        ::MessageBoxA(nullptr, e.what(), "Fatal Error", MB_ICONERROR);
        return -1;
    }
}