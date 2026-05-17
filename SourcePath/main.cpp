#include "LoaderUI.h"
#include <windows.h>

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    CLoaderUI loader;
    if (!loader.Initialize()) {
        MessageBoxA(nullptr, "Failed to initialize loader UI.", "Rust External", MB_ICONERROR | MB_OK);
        return -1;
    }

    auto result = loader.Run();
    if (!result.shouldLaunch || !result.cheat) {
        return 0;
    }

    try {
        result.cheat->Run();
    } catch (...) {
        MessageBoxA(nullptr, "Runtime error occurred. The cheat will shut down.", "Rust External", MB_ICONERROR | MB_OK);
    }

    result.cheat->Shutdown();
    return 0;
}
