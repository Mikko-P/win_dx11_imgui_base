#include "window.h"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PWSTR lpCmdLine,
    int nCmdShow)
{
    auto window{ Window(hInstance, {800, 800}, nCmdShow) };

    window.mainLoop();

    return 0;
}