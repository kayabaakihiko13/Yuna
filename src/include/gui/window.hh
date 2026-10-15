#pragma once
#ifndef YUNA_GUI_WINDOW_HH
#define YUNA_GUI_WINDOW_HH

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <thread>
#include <atomic>
#include <string>
#include <stdexcept>

#include "resource.hh"
#include "image_enhance/lanczos.hh"
#include "image_enhance/pde_super_resolution.hh"
#include "utils/format_file_image.hh"

namespace yuna::gui
{

    constexpr UINT WM_YUNA_PROGRESS = WM_APP + 1;
    constexpr UINT WM_YUNA_DONE = WM_APP + 2;

    class Window
    {
    public:
        Window(HINSTANCE hInstance, int nCmdShow)
            : hInstance_(hInstance)
        {
            INITCOMMONCONTROLSEX icce{};
            icce.dwSize = sizeof(icce);
            icce.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;
            ::InitCommonControlsEx(&icce);

            RegisterWindowClass();
            Create();
            EnableModernEffects();

            ::ShowWindow(hwnd_, nCmdShow);
            ::UpdateWindow(hwnd_);
        }

        ~Window()
        {
            if (worker_.joinable())
                worker_.join();
            if (hFont_)
                ::DeleteObject(hFont_);
            if (hFontBold_)
                ::DeleteObject(hFontBold_);
            if (hBgBrush_)
                ::DeleteObject(hBgBrush_);
            if (hEditBrush_)
                ::DeleteObject(hEditBrush_);
        }

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        int Run()
        {
            MSG msg{};
            BOOL ret;
            while ((ret = ::GetMessage(&msg, nullptr, 0, 0)) != 0)
            {
                if (ret == -1)
                    return -1;
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
            return static_cast<int>(msg.wParam);
        }

    private:
        static constexpr wchar_t kClassName[] = L"YunaMainWindowClass";

        // Ukuran CLIENT area (dalam pixel @ 96 DPI, akan di-scale otomatis)
        static constexpr int kClientWidth = 720;
        static constexpr int kClientHeight = 340; //

        int Scale(int value) const
        {
            return MulDiv(value, dpi_, 96);
        }

        void RegisterWindowClass()
        {
            WNDCLASSEXW wc{};
            wc.cbSize = sizeof(WNDCLASSEXW);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = &Window::WndProcStatic;
            wc.hInstance = hInstance_;
            wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
            wc.hbrBackground = nullptr; // Di-handle manual di WM_ERASEBKGND
            wc.lpszClassName = kClassName;
            wc.hIcon = ::LoadIconW(hInstance_, MAKEINTRESOURCEW(YunaIcon));
            wc.hIconSm = wc.hIcon;
            if (!::RegisterClassExW(&wc))
                throw std::runtime_error("RegisterClassExW gagal");
        }

        void Create()
        {
            RECT rect = {0, 0, Scale(kClientWidth), Scale(kClientHeight)};
            DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
            ::AdjustWindowRectEx(&rect, style, FALSE, 0);

            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            hwnd_ = ::CreateWindowExW(
                0, kClassName, L"Yuna - Image Upscaler",
                style,
                CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                nullptr, nullptr, hInstance_, this);
            if (!hwnd_)
                throw std::runtime_error("CreateWindowExW gagal");

            dpi_ = ::GetDpiForWindow(hwnd_);
        }

        void EnableModernEffects()
        {
            // Rounded corners (Windows 11). Dark mode DIMATIKAN agar tampilan putih.
            BOOL darkMode = FALSE;
            ::DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

            DWM_WINDOW_CORNER_PREFERENCE cornerPref = DWMWCP_ROUND;
            ::DwmSetWindowAttribute(hwnd_, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPref, sizeof(cornerPref));
        }

        void CreateControls()
        {
            // ===== Tema Light Modern =====
            bgColor_ = RGB(255, 255, 255);
            textColor_ = RGB(32, 32, 32);
            hBgBrush_ = ::CreateSolidBrush(bgColor_);
            hEditBrush_ = ::CreateSolidBrush(RGB(255, 255, 255));

            // ===== Font: coba Segoe UI Variable (Win11), fallback Segoe UI =====
            const wchar_t *fontName = L"Segoe UI Variable Text";
            HFONT testFont = ::CreateFontW(-Scale(14), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontName);
            if (!testFont)
            {
                fontName = L"Segoe UI";
            }
            else
            {
                ::DeleteObject(testFont);
            }

            hFont_ = ::CreateFontW(-Scale(14), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontName);
            hFontBold_ = ::CreateFontW(-Scale(14), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontName);

            auto ApplyFont = [this](HWND hwnd)
            {
                if (hwnd)
                    ::SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(hFont_), TRUE);
                return hwnd;
            };
            auto ApplyFontBold = [this](HWND hwnd)
            {
                if (hwnd)
                    ::SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(hFontBold_), TRUE);
                return hwnd;
            };

            // ===== Layout Constants (semua ter-scale via Scale()) =====
            const int PAD = Scale(28);
            const int LABEL_W = Scale(110);
            const int EDIT_W = Scale(420);
            const int BTN_W = Scale(115);
            const int ROW_H = Scale(34);
            const int GAP = Scale(18);
            const int ROW_GAP = Scale(15);
            const int CONTENT_W = Scale(kClientWidth) - 2 * PAD;

            int x0 = PAD;
            int y = PAD;

            // ================= Row 1: Input File =================
            HWND lblIn = ::CreateWindowExW(0, L"STATIC", L"Input File",
                                           WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                                           x0, y, LABEL_W, ROW_H, hwnd_, nullptr, hInstance_, nullptr);
            ApplyFontBold(lblIn);

            hEditInput_ = ApplyFont(::CreateWindowExW(0, L"EDIT", L"",
                                                      WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                                      x0 + LABEL_W + GAP, y, EDIT_W, ROW_H,
                                                      hwnd_, (HMENU)IDC_EDIT_INPUT, hInstance_, nullptr));
            ::SendMessageW(hEditInput_, EM_SETCUEBANNER, TRUE,
                           reinterpret_cast<LPARAM>(L"Belum ada file gambar dipilih..."));

            hBtnBrowseIn_ = ApplyFont(::CreateWindowExW(0, L"BUTTON", L"&Browse...",
                                                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                                        x0 + LABEL_W + GAP + EDIT_W + GAP, y, BTN_W, ROW_H,
                                                        hwnd_, (HMENU)IDC_BTN_BROWSE_IN, hInstance_, nullptr));

            // ================= Row 2: Save As =================
            y += ROW_H + ROW_GAP;
            HWND lblOut = ::CreateWindowExW(0, L"STATIC", L"Save As",
                                            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                                            x0, y, LABEL_W, ROW_H, hwnd_, nullptr, hInstance_, nullptr);
            ApplyFontBold(lblOut);

            hEditOutput_ = ApplyFont(::CreateWindowExW(0, L"EDIT", L"",
                                                       WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                                       x0 + LABEL_W + GAP, y, EDIT_W, ROW_H,
                                                       hwnd_, (HMENU)IDC_EDIT_OUTPUT, hInstance_, nullptr));
            ::SendMessageW(hEditOutput_, EM_SETCUEBANNER, TRUE,
                           reinterpret_cast<LPARAM>(L"Akan terisi otomatis setelah pilih input..."));

            hBtnBrowseOut_ = ApplyFont(::CreateWindowExW(0, L"BUTTON", L"&Save as...",
                                                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                                         x0 + LABEL_W + GAP + EDIT_W + GAP, y, BTN_W, ROW_H,
                                                         hwnd_, (HMENU)IDC_BTN_BROWSE_OUT, hInstance_, nullptr));

            // ================= Row 3: Scale Factor =================
            y += ROW_H + ROW_GAP;
            HWND lblScale = ::CreateWindowExW(0, L"STATIC", L"Scale factor",
                                              WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                                              x0, y, LABEL_W, ROW_H, hwnd_, nullptr, hInstance_, nullptr);
            ApplyFontBold(lblScale);

            int radioX = x0 + LABEL_W + GAP;
            int radioW = Scale(80);
            ApplyFont(::CreateWindowExW(0, L"BUTTON", L"2x",
                                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
                                        radioX, y, radioW, ROW_H, hwnd_, (HMENU)IDC_RADIO_SCALE_2, hInstance_, nullptr));

            ApplyFont(::CreateWindowExW(0, L"BUTTON", L"4x",
                                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                                        radioX + radioW + GAP, y, radioW, ROW_H,
                                        hwnd_, (HMENU)IDC_RADIO_SCALE_4, hInstance_, nullptr));

            ApplyFont(::CreateWindowExW(0, L"BUTTON", L"8x",
                                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                                        radioX + 2 * (radioW + GAP), y, radioW, ROW_H,
                                        hwnd_, (HMENU)IDC_RADIO_SCALE_8, hInstance_, nullptr));

            ::CheckRadioButton(hwnd_, IDC_RADIO_SCALE_2, IDC_RADIO_SCALE_8, IDC_RADIO_SCALE_2);

            // ================= Row 4: PDE Checkbox =================
            y += ROW_H + ROW_GAP;
            hChkPde_ = ApplyFont(::CreateWindowExW(0, L"BUTTON", L"PDE post Processing",
                                                   WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                                                   x0, y, Scale(260), Scale(28),
                                                   hwnd_, (HMENU)IDC_CHK_PDE, hInstance_, nullptr));

            // ================= Row 5: Process Button (centered) =================
            y += Scale(24) + ROW_GAP;
            const int PROC_W = Scale(170);
            const int PROC_H = Scale(36);
            int procX = (Scale(kClientWidth) - PROC_W) / 2;
            hBtnProcess_ = ApplyFont(::CreateWindowExW(0, L"BUTTON", L"&Process Image",
                                                       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_DEFPUSHBUTTON | WS_DISABLED,
                                                       procX, y, PROC_W, PROC_H,
                                                       hwnd_, (HMENU)IDC_BTN_PROCESS, hInstance_, nullptr));

            // ================= Row 6: Progress Bar =================
            y += PROC_H + ROW_GAP;
            hProgress_ = ::CreateWindowExW(0, PROGRESS_CLASSW, nullptr,
                                           WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                                           x0, y, CONTENT_W, Scale(24),
                                           hwnd_, (HMENU)IDC_PROGRESS, hInstance_, nullptr);
            ::SendMessageW(hProgress_, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            ::SendMessageW(hProgress_, PBM_SETPOS, 0, 0);
        }

        int GetCheckedRadioButton(HWND hDlg, int idFirst, int idLast)
        {
            for (int id = idFirst; id <= idLast; ++id)
                if (::IsDlgButtonChecked(hDlg, id) == BST_CHECKED)
                    return id;
            return 0;
        }

        void BrowseInput()
        {
            wchar_t buf[MAX_PATH] = L"";
            OPENFILENAMEW ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd_;
            ofn.hInstance = hInstance_;
            ofn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0";
            ofn.lpstrFile = buf;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                        OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
            ofn.lpstrInitialDir = nullptr;
            ofn.lpstrTitle = L"Pilih Gambar Input";

            if (::GetOpenFileNameW(&ofn))
            {
                ::SetWindowTextW(hEditInput_, buf);

                std::wstring outSuggest = buf;
                auto dot = outSuggest.find_last_of(L'.');
                if (dot != std::wstring::npos)
                    outSuggest.insert(dot, L"_upscaled");
                ::SetWindowTextW(hEditOutput_, outSuggest.c_str());

                ::EnableWindow(hBtnProcess_, TRUE);
                SetStatus(L"File dipilih. Atur scale factor lalu klik Process.");
            }
            else
            {
                DWORD err = ::CommDlgExtendedError();
                if (err)
                {
                    wchar_t msg[256];
                    swprintf_s(msg, 256, L"File dialog error: %lu", err);
                    ::MessageBoxW(hwnd_, msg, L"Error", MB_OK | MB_ICONERROR);
                }
            }
        }

        void BrowseOutput()
        {
            wchar_t buf[MAX_PATH] = L"";
            OPENFILENAMEW ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd_;
            ofn.hInstance = hInstance_;
            ofn.lpstrFilter = L"PNG\0*.png\0JPEG\0*.jpg\0BMP\0*.bmp\0All Files\0*.*\0";
            ofn.lpstrFile = buf;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
            ofn.lpstrDefExt = L"png";
            ofn.lpstrInitialDir = nullptr;
            ofn.lpstrTitle = L"Simpan Hasil Upscaling";

            if (::GetSaveFileNameW(&ofn))
                ::SetWindowTextW(hEditOutput_, buf);
            else
            {
                DWORD err = ::CommDlgExtendedError();
                if (err)
                {
                    wchar_t msg[256];
                    swprintf_s(msg, 256, L"Save dialog error: %lu", err);
                    ::MessageBoxW(hwnd_, msg, L"Error", MB_OK | MB_ICONERROR);
                }
            }
        }

        int GetSelectedScale(HWND hwnd)
        {
            int id = GetCheckedRadioButton(hwnd, IDC_RADIO_SCALE_2, IDC_RADIO_SCALE_8);
            switch (id)
            {
            case IDC_RADIO_SCALE_4:
                return 4;
            case IDC_RADIO_SCALE_8:
                return 8;
            default:
                return 2;
            }
        }

        static std::string WideToUtf8(const std::wstring &w)
        {
            if (w.empty())
                return {};
            int size = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string out(size - 1, '\0');
            ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, out.data(), size, nullptr, nullptr);
            return out;
        }

        static std::wstring Utf8ToWide(const std::string &s)
        {
            if (s.empty())
                return {};
            int size = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
            std::wstring out(size - 1, L'\0');
            ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.data(), size);
            return out;
        }

        void SetStatus(const std::wstring &text)
        {
            if (!hStatus_)
                return; // 🔧 safety guard
            ::SetWindowTextW(hStatus_, text.c_str());
            ::InvalidateRect(hStatus_, nullptr, TRUE);
            ::UpdateWindow(hStatus_);
        }

        void StartProcessing()
        {
            if (processing_)
                return;

            wchar_t inBuf[MAX_PATH], outBuf[MAX_PATH];
            ::GetWindowTextW(hEditInput_, inBuf, MAX_PATH);
            ::GetWindowTextW(hEditOutput_, outBuf, MAX_PATH);

            std::string inputPath = WideToUtf8(inBuf);
            std::string outputPath = WideToUtf8(outBuf);
            int scale = GetSelectedScale(hwnd_);
            bool usePde = (::SendMessageW(hChkPde_, BM_GETCHECK, 0, 0) == BST_CHECKED);

            if (inputPath.empty() || outputPath.empty())
            {
                SetStatus(L"Input/output path tidak boleh kosong.");
                return;
            }

            processing_ = true;
            ::EnableWindow(hBtnProcess_, FALSE);
            ::EnableWindow(hBtnBrowseIn_, FALSE);
            ::EnableWindow(hBtnBrowseOut_, FALSE);
            ::SendMessageW(hProgress_, PBM_SETPOS, 0, 0);
            SetStatus(L"Memproses...");

            if (worker_.joinable())
                worker_.join();

            worker_ = std::thread([this, inputPath, outputPath, scale, usePde]()
                                  { WorkerRun(inputPath, outputPath, scale, usePde); });
        }

        void WorkerRun(std::string inputPath, std::string outputPath, double scale, bool usePde)
        {
            bool ok = false;
            try
            {
                format_processing::ImageFormat outFmt = format_processing::detect_format(outputPath);
                if (outFmt == format_processing::ImageFormat::UNKNOWN)
                {
                    outFmt = format_processing::ImageFormat::PNG;
                    outputPath = format_processing::ensure_extension(outputPath, outFmt);
                }

                int w = 0, h = 0;
                Image src = image_utils::load_image(inputPath, w, h);
                if (src.data.empty())
                {
                    ::PostMessageW(hwnd_, WM_YUNA_DONE, FALSE, 0);
                    return;
                }

                Image result = lanczos::resize(src, scale,
                                               [this](int done, int total)
                                               {
                                                   ::PostMessageW(hwnd_, WM_YUNA_PROGRESS,
                                                                  static_cast<WPARAM>(done),
                                                                  static_cast<LPARAM>(total));
                                               });

                if (usePde)
                    result = PDE_SR::enhance(result);
                ok = image_utils::save_image(outputPath, result, outFmt);
            }
            catch (...)
            {
                ok = false;
            }

            ::PostMessageW(hwnd_, WM_YUNA_DONE, static_cast<WPARAM>(ok), 0);
        }

        void OnProgress(int done, int total)
        {
            if (total <= 0)
                return;
            int percent = static_cast<int>((100.0 * done) / total);
            ::SendMessageW(hProgress_, PBM_SETPOS, percent, 0);
            std::wstring status = L"Memproses... " + std::to_wstring(percent) + L"%";
            SetStatus(status);
        }

        void OnDone(bool success)
        {
            processing_ = false;
            ::EnableWindow(hBtnProcess_, TRUE);
            ::EnableWindow(hBtnBrowseIn_, TRUE);
            ::EnableWindow(hBtnBrowseOut_, TRUE);
            ::SendMessageW(hProgress_, PBM_SETPOS, success ? 100 : 0, 0);

            if (success)
                SetStatus(L"Selesai! Gambar berhasil disimpan.");
            else
                SetStatus(L"Gagal memproses gambar.");
        }

        LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
        {
            switch (msg)
            {
            case WM_CREATE:
                CreateControls();
                return 0;

            case WM_DPICHANGED:
            {
                dpi_ = HIWORD(wParam);
                RECT *const prcNewWindow = (RECT *)lParam;
                ::SetWindowPos(hwnd_, nullptr, prcNewWindow->left, prcNewWindow->top,
                               prcNewWindow->right - prcNewWindow->left,
                               prcNewWindow->bottom - prcNewWindow->top,
                               SWP_NOZORDER | SWP_NOACTIVATE);
                return 0;
            }

            case WM_COMMAND:
            {
                if (HIWORD(wParam) == 0)
                {
                    switch (LOWORD(wParam))
                    {
                    case IDC_BTN_BROWSE_IN:
                        BrowseInput();
                        break;
                    case IDC_BTN_BROWSE_OUT:
                        BrowseOutput();
                        break;
                    case IDC_BTN_PROCESS:
                        StartProcessing();
                        break;
                    }
                }
                return 0;
            }

            case WM_ERASEBKGND:
            {
                HDC hdc = reinterpret_cast<HDC>(wParam);
                RECT rect;
                GetClientRect(hwnd_, &rect);
                FillRect(hdc, &rect, hBgBrush_);
                return 1;
            }

            case WM_CTLCOLORSTATIC:
            {
                HDC hdc = reinterpret_cast<HDC>(wParam);
                ::SetBkMode(hdc, TRANSPARENT);
                ::SetTextColor(hdc, textColor_);
                return reinterpret_cast<LRESULT>(hBgBrush_);
            }

            case WM_CTLCOLOREDIT:
            case WM_CTLCOLORLISTBOX:
            {
                HDC hdc = reinterpret_cast<HDC>(wParam);
                ::SetBkColor(hdc, RGB(255, 255, 255));
                ::SetTextColor(hdc, textColor_);
                ::SetBkMode(hdc, OPAQUE);
                return reinterpret_cast<LRESULT>(hEditBrush_);
            }

            case WM_CTLCOLORBTN:
            {
                return ::DefWindowProcW(hwnd_, msg, wParam, lParam);
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

        static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            Window *self = nullptr;
            if (msg == WM_NCCREATE)
            {
                auto *cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
                self = static_cast<Window *>(cs->lpCreateParams);
                ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
                self->hwnd_ = hwnd;
            }
            else
            {
                self = reinterpret_cast<Window *>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            }
            return self ? self->HandleMessage(msg, wParam, lParam)
                        : ::DefWindowProcW(hwnd, msg, wParam, lParam);
        }

        static constexpr int IDC_EDIT_INPUT = 1001;
        static constexpr int IDC_BTN_BROWSE_IN = 1002;
        static constexpr int IDC_EDIT_OUTPUT = 1003;
        static constexpr int IDC_BTN_BROWSE_OUT = 1004;
        static constexpr int IDC_CHK_PDE = 1006;
        static constexpr int IDC_BTN_PROCESS = 1007;
        static constexpr int IDC_PROGRESS = 1008;
        static constexpr int IDC_STATUS = 1009;
        static constexpr int IDC_RADIO_SCALE_2 = 1010;
        static constexpr int IDC_RADIO_SCALE_4 = 1011;
        static constexpr int IDC_RADIO_SCALE_8 = 1012;

        HINSTANCE hInstance_{};
        HWND hwnd_{};
        UINT dpi_ = 96;

        HWND hEditInput_{}, hBtnBrowseIn_{};
        HWND hEditOutput_{}, hBtnBrowseOut_{};
        HWND hChkPde_{};
        HWND hBtnProcess_{}, hProgress_{}, hStatus_{};

        HFONT hFont_{};
        HFONT hFontBold_{};

        COLORREF bgColor_{};
        COLORREF textColor_{};
        HBRUSH hBgBrush_{};
        HBRUSH hEditBrush_{};

        std::thread worker_;
        std::atomic<bool> processing_{false};
    };

} // namespace yuna::gui
#endif // YUNA_GUI_WINDOW_HH