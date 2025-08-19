#include "gameareaselector.h"
#include "iostream"

GameAreaSelector::GameAreaSelector(const std::string& gameTitle)
    : gameTitle_(gameTitle)
{
    wc.hInstance = GetModuleHandle(nullptr);

    // 1) Find the game window
    hwndGame_ = FindWindowA(nullptr, gameTitle.c_str());
    if (!hwndGame_) {
        std::cerr << "Game window not found: " << gameTitle_ << "\n";
        return;
    }

    // 2) Get client area in screen coords
    if (!GetClientRectOnScreen(hwndGame_, clientOnScreen_)) {
        std::cerr << "Failed to get client rect\n";
        hwndGame_ = nullptr;
        return;
    }
}

GameAreaSelector::~GameAreaSelector(){};
// ===================== helpers =====================

bool GameAreaSelector::GetClientRectOnScreen(HWND hwnd, RECT& out) {
    RECT rc{};
    if (!GetClientRect(hwnd, &rc)) return false;
    POINT tl{ rc.left, rc.top }, br{ rc.right, rc.bottom };
    if (!ClientToScreen(hwnd, &tl)) return false;
    if (!ClientToScreen(hwnd, &br)) return false;
    out = { tl.x, tl.y, br.x, br.y };
    return true;
}

void GameAreaSelector::NormalizeRect(RECT& r) {
    if (r.left > r.right)   std::swap(r.left, r.right);
    if (r.top  > r.bottom)  std::swap(r.top,  r.bottom);
}

// ===================== public =====================

bool GameAreaSelector::selectAreas() {

    // 3) Create overlay (register class if needed)
    hwndOverlay_ = CreateOverlayWindow();
    if (!hwndOverlay_) return false;

    // Prompt for step 1
    MessageBox(hwndOverlay_, L"Select the PLAYABLE area", L"Step 1", MB_OK);

    // 4) Message loop (blocking until PostQuitMessage)
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // If we got both rectangles, step went 0 -> 1 -> done
    return (results_[0].right > results_[0].left) && (results_[1].right > results_[1].left);
}

HWND GameAreaSelector::CreateOverlayWindow() {
    if (!RegisterOverlayClass()) return nullptr;

    // Align overlay to game client
    int x = clientOnScreen_.left;
    int y = clientOnScreen_.top;
    int w = clientOnScreen_.right  - clientOnScreen_.left;
    int h = clientOnScreen_.bottom - clientOnScreen_.top;

    // Create overlay as a child of the game window (your choice)
    // NOTE: Parenting to hwndGame_ is simpler (auto-move/resize),
    //       but some games may repaint over children or affect transparency.
    //       If that happens, consider parent = nullptr + WS_EX_TOPMOST and manual SetWindowPos sync.
    HWND overlay = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        wc.lpszClassName, L"",
        WS_POPUP,
        x, y, w, h,
        hwndGame_, nullptr, wc.hInstance, this // pass 'this' via lpParam
        );

    if (!overlay) {
        std::cerr << "Failed to create overlay\n";
        return nullptr;
    }

    // Semi-transparency
    if (!SetLayeredWindowAttributes(overlay, 0, 100, LWA_ALPHA)) {
        std::cerr << "Error setting alpha blending\n";
        DestroyWindow(overlay);
        return nullptr;
    }

    ShowWindow(overlay, SW_SHOW);
    return overlay;
}

// ===================== overlay creation =====================

bool GameAreaSelector::RegisterOverlayClass() {
    // Background brush (we own it; free in dtor)
    if (!hbrBackground_) {
        hbrBackground_ = CreateSolidBrush(colorBackg_);
    }

    wc.lpfnWndProc   = &GameAreaSelector::StaticOverlayProc;
    wc.lpszClassName = L"RegionSelectorOverlay";
    wc.hCursor       = LoadCursor(nullptr, IDC_CROSS);
    wc.hbrBackground = hbrBackground_;

    // RegisterClass can fail with ERROR_CLASS_ALREADY_EXISTS if called twice — ok for us.
    if (!RegisterClass(&wc)) {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            std::cerr << "RegisterClass failed.\n";
            return false;
        }
    }

    return true;
}

// ===================== wndproc trampoline =====================

// Store 'this' on first create, so we can forward to the instance method.
void GameAreaSelector::attachToWindow(HWND hwnd) {
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

LRESULT CALLBACK GameAreaSelector::StaticOverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GameAreaSelector* self = reinterpret_cast<GameAreaSelector*>(
        GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (msg == WM_NCCREATE) {
        // First time: the lpCreateParams holds 'this'
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        self = reinterpret_cast<GameAreaSelector*>(cs->lpCreateParams);
        if (self) self->attachToWindow(hwnd);
    }

    if (self) {
        return self->OverlayProc(hwnd, msg, wParam, lParam);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ===================== instance wndproc =====================

LRESULT GameAreaSelector::OverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_LBUTTONDOWN: {
        state_.selecting = true;
        state_.start.x = GET_X_LPARAM(lParam);
        state_.start.y = GET_Y_LPARAM(lParam);
        state_.end = state_.start;
        SetCapture(hwnd); // keep receiving mouse even if cursor leaves
        return 0;
    }

    case WM_MOUSEMOVE: {
        if (state_.selecting) {
            state_.end.x = GET_X_LPARAM(lParam);
            state_.end.y = GET_Y_LPARAM(lParam);
            InvalidateRect(hwnd, nullptr, TRUE); // will trigger WM_PAINT
        }
        return 0;
    }

    case WM_LBUTTONUP: {
        if (state_.selecting) {
            state_.selecting = false;
            ReleaseCapture();

            RECT sel{ state_.start.x, state_.start.y, state_.end.x, state_.end.y };
            NormalizeRect(sel);

            // Convert from overlay-local (0..w/0..h) to screen coords by adding game client origin
            sel.left   += clientOnScreen_.left;
            sel.top    += clientOnScreen_.top;
            sel.right  += clientOnScreen_.left;
            sel.bottom += clientOnScreen_.top;

            results_[state_.step] = sel;

            // Log (kept from your code)
            std::cout
                << (state_.step == 0 ? "Playable area" : "Map area")
                << ": (" << sel.left << "," << sel.top << ") "
                << (sel.right - sel.left) << "x" << (sel.bottom - sel.top) << "\n";

            if (state_.step == 0) {
                MessageBox(hwnd, L"Now select the MAP area inside playable area", L"Step 2", MB_OK);
                state_.step = 1;
            } else {
                // Clean up overlay window & class
                if (hwndOverlay_) {
                    DestroyWindow(hwndOverlay_);
                }
                UnregisterClass(L"RegionSelectorOverlay", wc.hInstance);
                PostQuitMessage(0); // done
            }
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Only draw the current drag rectangle (eraser handled by hbrBackground + bErase=TRUE)
        const COLORREF penColor = (state_.step == 0) ? colorPlayable_ : colorMap_;
        HPEN pen = CreatePen(PS_SOLID, 2, penColor);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

        Rectangle(hdc, state_.start.x, state_.start.y, state_.end.x, state_.end.y);

        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            if (hwndOverlay_) {
                DestroyWindow(hwndOverlay_);
            }
            UnregisterClass(L"RegionSelectorOverlay", wc.hInstance);
            PostQuitMessage(0);
        }
        return 0;

    default:
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
