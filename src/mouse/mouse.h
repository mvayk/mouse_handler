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

    void relative_move_precise(double total_dx, int steps, int delay_ms = 1) {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;

        double dx_per_step = total_dx / steps;
        double dx_accum = 0.0;

        for (int i = 0; i < steps; ++i) {
            dx_accum += dx_per_step;

            int move_dx = static_cast<int>(std::round(dx_accum));
            dx_accum -= move_dx;

            input.mi.dx = move_dx;
            input.mi.dy = 0;
            SendInput(1, &input, sizeof(INPUT));
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
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
