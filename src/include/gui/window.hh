#pragma once
#ifndef YUNA_GUI_WINDOW_HH
#define YUNA_GUI_WINDOW_HH

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>
#include <thread>
#include <atomic>
#include <string>
#include <chrono>

#include "resource.hh"
#include "image_enhance/lanczos.hh"
#include "image_enhance/pde_super_resolution.hh"
#include "utils/format_file_image.hh"
#include "utils/stb_image.h"
#include "utils/stb_image_write.h"

namespace yuna::gui {

// Pesan custom: dikirim dari worker thread ke UI thread
constexpr UINT WM_YUNA_PROGRESS = WM_APP + 1; // wParam=done, lParam=total
constexpr UINT WM_YUNA_DONE     = WM_APP + 2; // wParam=success(bool)

class Window {
public:
    Window(HINSTANCE hInstance, int nCmdShow)
        : hInstance_(hInstance) {
        RegisterWindowClass();
        Create();
        ::ShowWindow(hwnd_, nCmdShow);
        ::UpdateWindow(hwnd_);
    }

    ~Window() {
        if (worker_.joinable()) worker_.join();
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    int Run() {
        MSG msg{};
        while (::GetMessage(&msg, nullptr, 0, 0) > 0) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        return static_cast<int>(msg.wParam);
    }

private:
    static constexpr wchar_t kClassName[] = L"YunaMainWindowClass";

    void RegisterWindowClass() {
        WNDCLASSEXW wc{};
        wc.cbSize        = sizeof(WNDCLASSEXW);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = &Window::WndProcStatic;
        wc.hInstance     = hInstance_;
        wc.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = kClassName;
        wc.hIcon         = ::LoadIconW(hInstance_, MAKEINTRESOURCEW(YunaIcon));
        wc.hIconSm       = wc.hIcon;
        if (!::RegisterClassExW(&wc)) throw std::runtime_error("RegisterClassExW gagal");
    }

    void Create() {
        hwnd_ = ::CreateWindowExW(
            0, kClassName, L"Yuna - Image Upscaler",
            WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, 480, 320,
            nullptr, nullptr, hInstance_, this);
        if (!hwnd_) throw std::runtime_error("CreateWindowExW gagal");
    }

    void CreateControls() {
        auto label = [&](const wchar_t* text, int x, int y, int w) {
            ::CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE,
                               x, y, w, 18, hwnd_, nullptr, hInstance_, nullptr);
        };

        label(L"Input file:", 10, 12, 100);
        hEditInput_ = ::CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY,
            10, 32, 340, 24, hwnd_, (HMENU)IDC_EDIT_INPUT, hInstance_, nullptr);
        hBtnBrowseIn_ = ::CreateWindowExW(0, L"BUTTON", L"Browse...",
            WS_CHILD | WS_VISIBLE, 360, 32, 90, 24,
            hwnd_, (HMENU)IDC_BTN_BROWSE_IN, hInstance_, nullptr);

        label(L"Output file:", 10, 64, 100);
        hEditOutput_ = ::CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            10, 84, 340, 24, hwnd_, (HMENU)IDC_EDIT_OUTPUT, hInstance_, nullptr);
        hBtnBrowseOut_ = ::CreateWindowExW(0, L"BUTTON", L"Save as...",
            WS_CHILD | WS_VISIBLE, 360, 84, 90, 24,
            hwnd_, (HMENU)IDC_BTN_BROWSE_OUT, hInstance_, nullptr);

        label(L"Scale factor:", 10, 118, 100);
        HWND hRadio2 = CreateWindowExW(
            0,L"Button", L"2x",
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
            120, 116, 50, 24, hwnd_, 
            (HMENU)IDC_RADIO_SCALE_2, hInstance_, nullptr
        );
        HWND hRadio4 = CreateWindowExW(
        0, L"BUTTON", L"4x",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        180, 116, 50, 24, hwnd_, (HMENU)IDC_RADIO_SCALE_4, hInstance_, nullptr);

        HWND hRadio8 = CreateWindowExW(
        0, L"BUTTON", L"8x",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        240, 116, 50, 24, hwnd_, (HMENU)IDC_RADIO_SCALE_8, hInstance_, nullptr);

        // default setup with 2x scale
        CheckRadioButton(hwnd_,IDC_RADIO_SCALE_2,IDC_RADIO_SCALE_8,IDC_RADIO_SCALE_2);

        hChkPde_ = ::CreateWindowExW(0, L"BUTTON", L"PDE post-processing (Diffusion + Shock Filter)",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 150, 350, 22, hwnd_, (HMENU)IDC_CHK_PDE, hInstance_, nullptr);

        hBtnProcess_ = ::CreateWindowExW(0, L"BUTTON", L"Process",
            WS_CHILD | WS_VISIBLE | WS_DISABLED,
            10, 184, 120, 30, hwnd_, (HMENU)IDC_BTN_PROCESS, hInstance_, nullptr);

        hProgress_ = ::CreateWindowExW(0, PROGRESS_CLASSW, nullptr,
            WS_CHILD | WS_VISIBLE, 10, 226, 440, 20,
            hwnd_, (HMENU)IDC_PROGRESS, hInstance_, nullptr);
        ::SendMessageW(hProgress_, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

        hStatus_ = ::CreateWindowExW(0, L"STATIC", L"Pilih file gambar untuk mulai.",
            WS_CHILD | WS_VISIBLE, 10, 254, 440, 40,
            hwnd_, (HMENU)IDC_STATUS, hInstance_, nullptr);
    }
    int GetCheckedRadioButton(HWND hDlg, int idFirst, int idLast) {
        for (int id = idFirst; id <= idLast; ++id) {
            if (IsDlgButtonChecked(hDlg, id) == BST_CHECKED)
                return id;
        }
        return 0;
    }
    void BrowseInput() {
        wchar_t buf[MAX_PATH] = L"";
        OPENFILENAMEW ofn{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner   = hwnd_;
        ofn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0";
        ofn.lpstrFile   = buf;
        ofn.nMaxFile    = MAX_PATH;
        ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (::GetOpenFileNameW(&ofn)) {
            ::SetWindowTextW(hEditInput_, buf);
            inputPath_ = WideToUtf8(buf);

            // auto-suggest nama output
            std::wstring outSuggest = buf;
            auto dot = outSuggest.find_last_of(L'.');
            if (dot != std::wstring::npos) outSuggest.insert(dot, L"_upscaled");
            ::SetWindowTextW(hEditOutput_, outSuggest.c_str());

            ::EnableWindow(hBtnProcess_, TRUE);
            SetStatus(L"File dipilih. Atur scale factor lalu klik Process.");
        }
    }

    void BrowseOutput() {
        wchar_t buf[MAX_PATH] = L"";
        OPENFILENAMEW ofn{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner   = hwnd_;
        ofn.lpstrFilter = L"PNG\0*.png\0JPEG\0*.jpg\0BMP\0*.bmp\0All Files\0*.*\0";
        ofn.lpstrFile   = buf;
        ofn.nMaxFile    = MAX_PATH;
        ofn.Flags       = OFN_OVERWRITEPROMPT;
        ofn.lpstrDefExt = L"png";

        if (::GetSaveFileNameW(&ofn)) {
            ::SetWindowTextW(hEditOutput_, buf);
        }
    }
    int GetSelectedScale(HWND hwnd)
    {
        int id = GetCheckedRadioButton(hwnd, IDC_RADIO_SCALE_2, IDC_RADIO_SCALE_8);
        switch (id)
        {
        case IDC_RADIO_SCALE_2: return 2;
        case IDC_RADIO_SCALE_4: return 4;
        case IDC_RADIO_SCALE_8: return 8;
        default: return 2; // fallback
        }
    }
    static std::string WideToUtf8(const std::wstring& w) {
        if (w.empty()) return {};
        int size = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string out(size - 1, '\0');
        ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, out.data(), size, nullptr, nullptr);
        return out;
    }

    static std::wstring Utf8ToWide(const std::string& s) {
        if (s.empty()) return {};
        int size = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        std::wstring out(size - 1, L'\0');
        ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.data(), size);
        return out;
    }

    void SetStatus(const std::wstring& text) {
        ::SetWindowTextW(hStatus_, text.c_str());
    }

    // Dipanggil di UI thread saat tombol Process ditekan
    void StartProcessing() {
        if (processing_) return;

        wchar_t inBuf[MAX_PATH], outBuf[MAX_PATH], scaleBuf[64];
        ::GetWindowTextW(hEditInput_, inBuf, MAX_PATH);
        ::GetWindowTextW(hEditOutput_, outBuf, MAX_PATH);

        std::string inputPath  = WideToUtf8(inBuf);
        std::string outputPath = WideToUtf8(outBuf);
        int scale = GetSelectedScale(hwnd_); // ini berfungsi untuk radio button
        bool usePde = (::SendMessageW(hChkPde_, BM_GETCHECK, 0, 0) == BST_CHECKED);

        if (inputPath.empty() || outputPath.empty()) {
            SetStatus(L"Input/output path tidak boleh kosong.");
            return;
        }

        processing_ = true;
        ::EnableWindow(hBtnProcess_, FALSE);
        ::EnableWindow(hBtnBrowseIn_, FALSE);
        ::EnableWindow(hBtnBrowseOut_, FALSE);
        ::SendMessageW(hProgress_, PBM_SETPOS, 0, 0);
        SetStatus(L"Memproses...");

        if (worker_.joinable()) worker_.join();
        worker_ = std::thread(&Window::WorkerRun, this, inputPath, outputPath, scale, usePde);
    }

    void WorkerRun(std::string inputPath, std::string outputPath,
                   double scale, bool usePde) {
        bool ok = false;
        try {
            format_processing::ImageFormat outFmt =
                format_processing::detect_format(outputPath);
            if (outFmt == format_processing::ImageFormat::UNKNOWN) {
                outFmt = format_processing::ImageFormat::PNG; // default fallback
                outputPath = format_processing::ensure_extension(outputPath, outFmt);
            }

            int w = 0, h = 0;
            Image src = image_utils::load_image(inputPath, w, h);
            if (src.data.empty()) {
                PostMessageW(hwnd_, WM_YUNA_DONE, FALSE, 0);
                return;
            }

            Image result = lanczos::resize(src, scale,
                [this](int done, int total) {
                    ::PostMessageW(hwnd_, WM_YUNA_PROGRESS,
                                   static_cast<WPARAM>(done),
                                   static_cast<LPARAM>(total));
                });

            if (usePde) {
                result = PDE_SR::enhance(result);
            }

            ok = image_utils::save_image(outputPath, result, outFmt);
        } catch (...) {
            ok = false;
        }
        ::PostMessageW(hwnd_, WM_YUNA_DONE, static_cast<WPARAM>(ok), 0);
    }

    void OnProgress(int done, int total) {
        if (total <= 0) return;
        int percent = static_cast<int>((100.0 * done) / total);
        ::SendMessageW(hProgress_, PBM_SETPOS, percent, 0);
        SetStatus(L"Memproses... " + std::to_wstring(percent) + L"%");
    }

    void OnDone(bool success) {
        processing_ = false;
        ::EnableWindow(hBtnProcess_, TRUE);
        ::EnableWindow(hBtnBrowseIn_, TRUE);
        ::EnableWindow(hBtnBrowseOut_, TRUE);
        ::SendMessageW(hProgress_, PBM_SETPOS, success ? 100 : 0, 0);
        SetStatus(success ? L"Selesai! Gambar berhasil disimpan."
                           : L"Gagal memproses gambar.");
    }

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_CREATE:
                CreateControls();
                return 0;

            case WM_COMMAND: {
                switch (LOWORD(wParam)) {
                    case IDC_BTN_BROWSE_IN:  BrowseInput();  break;
                    case IDC_BTN_BROWSE_OUT: BrowseOutput(); break;
                    case IDC_BTN_PROCESS:    StartProcessing(); break;
                }
                return 0;
            }

            case WM_YUNA_PROGRESS:
                OnProgress(static_cast<int>(wParam), static_cast<int>(lParam));
                return 0;

            case WM_YUNA_DONE:
                OnDone(wParam != 0);
                return 0;

            case WM_DESTROY:
                ::PostQuitMessage(0);
                return 0;

            default:
                return ::DefWindowProcW(hwnd_, msg, wParam, lParam);
        }
    }

    static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Window* self = nullptr;
        if (msg == WM_NCCREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            self = static_cast<Window*>(cs->lpCreateParams);
            ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->hwnd_ = hwnd;
        } else {
            self = reinterpret_cast<Window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }
        return self ? self->HandleMessage(msg, wParam, lParam)
                    : ::DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    // control id
    static constexpr int IDC_EDIT_INPUT     = 1001;
    static constexpr int IDC_BTN_BROWSE_IN  = 1002;
    static constexpr int IDC_EDIT_OUTPUT    = 1003;
    static constexpr int IDC_BTN_BROWSE_OUT = 1004;
    static constexpr int IDC_EDIT_SCALE     = 1005;
    static constexpr int IDC_CHK_PDE        = 1006;
    static constexpr int IDC_BTN_PROCESS    = 1007;
    static constexpr int IDC_PROGRESS       = 1008;
    static constexpr int IDC_STATUS         = 1009;
    static constexpr int IDC_RADIO_SCALE_2  = 1010;
    static constexpr int IDC_RADIO_SCALE_4  = 1011;
    static constexpr int IDC_RADIO_SCALE_8  = 1012;

    HINSTANCE hInstance_{};
    HWND hwnd_{};
    HWND hEditInput_{}, hBtnBrowseIn_{};
    HWND hEditOutput_{}, hBtnBrowseOut_{};
    HWND hChkPde_{};
    HWND hBtnProcess_{}, hProgress_{}, hStatus_{};

    std::string inputPath_;
    std::thread worker_;
    std::atomic<bool> processing_{false};
};

} // namespace yuna::gui

#endif // YUNA_GUI_WINDOW_HH