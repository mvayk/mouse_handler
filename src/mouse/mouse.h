#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include <cmath>

typedef BOOL(WINAPI* wapi_set_mpos)(int, int);
typedef BOOL(WINAPI* wapi_get_mpos)(LPPOINT);

struct mouse {
    HMODULE user32_dll = {};
    wapi_set_mpos set_position;
    wapi_get_mpos get_position;

    mouse(HMODULE dll) {
        user32_dll = dll;
        set_position = (wapi_set_mpos)GetProcAddress(user32_dll, "SetCursorPos");
        get_position = (wapi_get_mpos)GetProcAddress(user32_dll, "GetCursorPos");
    }

    ~mouse() {
        if (user32_dll != 0) {
            FreeLibrary(user32_dll);
        }
    }

    void relative_move_precise(double total_dx, double total_dy, int steps, int delay_ms = 1, bool use_easing = true) {
        double accum_dx = 0.0;
        double accum_dy = 0.0;

        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.mouseData = 0;
        input.mi.dwExtraInfo = GetMessageExtraInfo();
        input.mi.time = 0;

        for (int i = 1; i <= steps; ++i) {
            double t = static_cast<double>(i) / steps;

            double eased_t = use_easing ? (0.5 - 0.5 * cos(t * 3.14159265358979323846)) : t;

            double target_dx = total_dx * eased_t;
            double target_dy = total_dy * eased_t;

            double prev_dx = total_dx * (use_easing ? (0.5 - 0.5 * cos((i - 1.0) / steps * 3.14159265358979323846)) : (i - 1.0) / steps);
            double prev_dy = total_dy * (use_easing ? (0.5 - 0.5 * cos((i - 1.0) / steps * 3.14159265358979323846)) : (i - 1.0) / steps);

            accum_dx += (target_dx - prev_dx);
            accum_dy += (target_dy - prev_dy);

            int move_dx = static_cast<int>(accum_dx);
            int move_dy = static_cast<int>(accum_dy);

            accum_dx -= move_dx;
            accum_dy -= move_dy;

            if (move_dx != 0 || move_dy != 0) {
                input.mi.dx = move_dx;
                input.mi.dy = move_dy;
                SendInput(1, &input, sizeof(INPUT));
            }

            if (delay_ms > 0) {
                Sleep(delay_ms);
            }
        }
    }

    void relative_move(float delay, int step_size, int x, int y) {
        double distance = std::hypot(x, y);
        int steps = static_cast<int>(distance / step_size);
        if (steps == 0) steps = 1;

        double step_dx = static_cast<double>(x) / steps;
        double step_dy = static_cast<double>(y) / steps;

        double accum_dx = 0; double accum_dy = 0;

        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;

        for (int i = 0; i < steps; ++i) {
            accum_dx += step_dx;
            accum_dy += step_dy;

            input.mi.dx = static_cast<LONG>(std::round(accum_dx));
            input.mi.dy = static_cast<LONG>(std::round(accum_dy));

            accum_dx -= step_dx;
            accum_dy -= step_dy;

            UINT sent = SendInput(1, &input, sizeof(INPUT));
            Sleep(delay);
        }
    }

    void absolute_move(float delay, int steps, int target_x, int target_y) {
        int screen_width = GetSystemMetrics(SM_CXSCREEN);
        int screen_height = GetSystemMetrics(SM_CYSCREEN);

        POINT current_pos;
        get_position(&current_pos);

        double dx = target_x - current_pos.x;
        double dy = target_y - current_pos.y;

        for (int i = 1; i <= steps; ++i) {
            int x = static_cast<int>(current_pos.x + dx * (i / static_cast<double>(steps)));
            int y = static_cast<int>(current_pos.y + dy * (i / static_cast<double>(steps)));

            int norm_x = static_cast<int>(x * (65535.0 / screen_width));
            int norm_y = static_cast<int>(y * (65535.0 / screen_height));

            INPUT input = {};
            input.type = INPUT_MOUSE;
            input.mi.dx = norm_x;
            input.mi.dy = norm_y;
            input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

            SendInput(1, &input, sizeof(INPUT));
            //std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            Sleep(delay);
        }
    }
};
