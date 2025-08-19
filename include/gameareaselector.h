#pragma once
#include <windows.h>
#include <string>
#include <windowsx.h>

class GameAreaSelector {
public:
    explicit GameAreaSelector(const std::string& gameTitle);
    ~GameAreaSelector();

    // Run the overlay selector (blocking until the two rectangles are chosen or ESC)
    // Returns true on success, false if canceled or failed.
    bool selectAreas();

    // Results (screen coordinates)
    RECT getPlayableArea() const { return results_[0]; }
    RECT getMapArea()      const { return results_[1]; }

private:
    // --- State ---
    const std::string gameTitle_;
    HWND hwndGame_        = nullptr;
    HWND hwndOverlay_     = nullptr;

    // Selection state (moved from globals)
    struct SelectionState {
        POINT start{0,0};
        POINT end{0,0};
        bool  selecting = false;   // dragging?
        int   step      = 0;       // 0 = playable, 1 = map
    } state_;

    // Final results (screen coords)
    RECT results_[2]      = {{0,0,0,0},{0,0,0,0}};

    // Game client on screen (used to convert overlay-local to screen coords)
    RECT clientOnScreen_  = {0,0,0,0};

    // Colors (kept from your code)
    COLORREF colorPlayable_ = RGB(0, 255, 0);
    COLORREF colorMap_      = RGB(255, 0, 0);
    COLORREF colorBackg_    = RGB(0, 0, 0);
    WNDCLASS wc{};


    // If you use a custom brush in WNDCLASS, keep it to free later
    HBRUSH hbrBackground_ = nullptr;

    // --- Helpers (same names, now private methods) ---
    bool GetClientRectOnScreen(HWND hwnd, RECT& out);
    void NormalizeRect(RECT& r);

    // Overlay creation & class registration
    HWND CreateOverlayWindow();
    bool RegisterOverlayClass();

    // Window procedure (static trampoline + instance method)
    static LRESULT CALLBACK StaticOverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Small utility to set instance pointer on the window (on WM_NCCREATE)
    void attachToWindow(HWND hwnd);
};
