#include <windows.h>
#include <string>
#include <iostream>
#include <windowsx.h>

// Structure to store selection process state
struct SelectionState {
    POINT start{};     // Mouse-down position
    POINT end{};       // Current mouse position while dragging
    bool selecting = false; // Are we dragging now?
    int step = 0;      // 0 = selecting playable area, 1 = selecting map area
    RECT results[2]{}; // Stores final rectangles for both selections
} g_state;

// The client area of the game window in screen coordinates
RECT g_clientOnScreen{};
COLORREF colorPlayable = RGB(0, 255, 0); // Green rectangle for playable
COLORREF colorMap = RGB(255, 0, 0);      // Red rectangle for map
COLORREF colorBackg= RGB(0, 0, 0);
// -----------------------------------------------------------
// Convert a window's client area to screen coordinates
// -----------------------------------------------------------
static bool GetClientRectOnScreen(HWND hwnd, RECT& out) {
    RECT rc{};
    if (!GetClientRect(hwnd, &rc)) return false;
    POINT tl{ rc.left, rc.top }, br{ rc.right, rc.bottom };
    if (!ClientToScreen(hwnd, &tl)) return false;
    if (!ClientToScreen(hwnd, &br)) return false;
    out = { tl.x, tl.y, br.x, br.y };
    return true;
}

// -----------------------------------------------------------
// Ensure left < right and top < bottom
// -----------------------------------------------------------
static void NormalizeRect(RECT& r) {
    if (r.left > r.right) std::swap(r.left, r.right);
    if (r.top > r.bottom) std::swap(r.top, r.bottom);
}

// -----------------------------------------------------------
// Window procedure for the overlay
// Handles mouse events, drawing, and keyboard escape
// -----------------------------------------------------------
LRESULT CALLBACK OverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_LBUTTONDOWN: // Start dragging
        g_state.selecting = true;
        g_state.start.x = GET_X_LPARAM(lParam);
        g_state.start.y = GET_Y_LPARAM(lParam);
        g_state.end = g_state.start;
        SetCapture(hwnd); // Capture mouse until button released
        return 0;

    case WM_MOUSEMOVE: // Update drag rectangle
        if (g_state.selecting) {
            g_state.end.x = GET_X_LPARAM(lParam);
            g_state.end.y = GET_Y_LPARAM(lParam);
            InvalidateRect(hwnd, nullptr, TRUE); // Trigger WM_PAINT
        }
        return 0;

    case WM_LBUTTONUP: { // End dragging
        if (g_state.selecting) {
            g_state.selecting = false;
            ReleaseCapture();
            // Create rect from drag start and end
            RECT sel{ g_state.start.x, g_state.start.y, g_state.end.x, g_state.end.y };
            NormalizeRect(sel);

            // Convert rect to screen coordinates relative to the game window
            sel.left   += g_clientOnScreen.left;
            sel.top    += g_clientOnScreen.top;
            sel.right  += g_clientOnScreen.left;
            sel.bottom += g_clientOnScreen.top;

            // Save rect
            g_state.results[g_state.step] = sel;

            // Show result in console
            std::cout << (g_state.step == 0 ? "Playable area" : "Map area")
                      << ": (" << sel.left << "," << sel.top << ") "
                      << (sel.right - sel.left) << "x" << (sel.bottom - sel.top) << "\n";

            if (g_state.step == 0) {
                MessageBox(hwnd, L"Now select the MAP area inside playable area", L"Step 2", MB_OK);
                g_state.step = 1;
            } else {
                PostQuitMessage(0); // Done selecting
            }
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Only draw your green/red selection rectangle
        HPEN pen = CreatePen(PS_SOLID, 2, colorPlayable);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
        Rectangle(hdc, g_state.start.x, g_state.start.y, g_state.end.x, g_state.end.y);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        EndPaint(hwnd, &ps);
        return 0;
    }


    case WM_KEYDOWN: // ESC key to quit
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// -----------------------------------------------------------
// CreateOverlayWindow
// Finds the game window, gets its client rect, registers
// overlay class, and creates a semi-transparent overlay.
// Returns overlay HWND on success, nullptr on failure.
// -----------------------------------------------------------
HWND CreateOverlayWindow(const std::string& title, HINSTANCE hInst) {
    // 1) Find the game window
    HWND hwndGame = FindWindowA(nullptr, title.c_str());
    if (!hwndGame) {
        std::cerr << "Game window not found\n";
        return nullptr;
    }

    // 2) Get client area in screen coords
    if (!GetClientRectOnScreen(hwndGame, g_clientOnScreen)) {
        std::cerr << "Failed to get client rect\n";
        return nullptr;
    }

    // 3) Register overlay window class
    WNDCLASS wc{};
    wc.lpfnWndProc   = OverlayProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = L"RegionSelectorOverlay";
    wc.hCursor       = LoadCursor(nullptr, IDC_CROSS);
    wc.hbrBackground = CreateSolidBrush(colorBackg); // black background
    RegisterClass(&wc);

    // 4) Create overlay as a child of the game window
    // NOTE: Parenting to hwndGame makes overlay auto-move/resize with the game,
    //       which is simpler. However, in some games that repaint aggressively,
    //       child layered windows may flicker or lose transparency.
    //       If that happens, consider switching parent to nullptr
    //       and using WS_EX_TOPMOST + SetWindowPos to sync manually.
    HWND overlay = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        wc.lpszClassName, L"",
        WS_POPUP,
        g_clientOnScreen.left, g_clientOnScreen.top,
        g_clientOnScreen.right - g_clientOnScreen.left,
        g_clientOnScreen.bottom - g_clientOnScreen.top,
        hwndGame, nullptr, wc.hInstance, nullptr
        );
    if (!overlay) {
        std::cerr << "Failed to create overlay\n";
        return nullptr;
    }

    // 5) Semi-transparency
    if (!SetLayeredWindowAttributes(overlay, 0, 100, LWA_ALPHA)) {
        std::cerr << "Error setting alpha blending\n";
        DestroyWindow(overlay);
        return nullptr;
    }

    ShowWindow(overlay, SW_SHOW);
    return overlay;
}

// -----------------------------------------------------------
// Entry point
// -----------------------------------------------------------
int main(int argc, char** argv) {
    std::string title = (argc > 1) ? argv[1] : "Dark Orbit";
    HINSTANCE hInst = GetModuleHandle(nullptr);

    HWND overlay = CreateOverlayWindow(title, hInst);
    if (!overlay) {
        return 1; // failed
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
