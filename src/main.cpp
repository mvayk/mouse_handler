#include <iostream>
#include <windows.h>

#include "mouse/mouse.h"

/*
 * source engine sensitivity = 0.15915
 */

int main() {
    Sleep(2000);

    mouse m(LoadLibraryA("user32.dll"));

    POINT mp = {};
    m.get_position(&mp);
    m.relative_move_precise(4524.300268989501, 60);

    return 0;
}
