#include <windows.h>

#include "mouse/mouse.h"

/*
 * source engine sensitivity = 0.15915
 */

static mouse* mouse_ptr = nullptr;

/* bug: spamming breaks everything */
static auto last_click = std::chrono::steady_clock::now();

LRESULT CALLBACK mouse_proc(int n_code, WPARAM w_param, LPARAM l_param) {
    if (n_code >= 0) {
        PMSLLHOOKSTRUCT p_mouse = (PMSLLHOOKSTRUCT)l_param;

        if (w_param == WM_XBUTTONDOWN) {
            if (HIWORD(p_mouse->mouseData) == XBUTTON1) {
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_click).count() > 200) {
                    last_click = now;
                    if (mouse_ptr) {
                        std::thread([] {
                                mouse_ptr->relative_move_precise(4524.300268989501, 60, 1);
                        }).detach();
                    }
                }
            }
        }
    }
    return CallNextHookEx(nullptr, n_code, w_param, l_param);
}

int main() {
    HHOOK mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, mouse_proc, nullptr, 0);
    MSG message;

    mouse m(LoadLibraryA("user32.dll"));
    mouse_ptr = &m;

    while(GetMessage(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    UnhookWindowsHookEx(mouse_hook);
    return 0;
}
