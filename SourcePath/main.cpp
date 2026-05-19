#include "LoaderUI.h"
#include <windows.h>

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    LoaderLog("WinMain entered");
    CLoaderUI loader;
    if (!loader.Initialize()) {
        LoaderLog("Loader initialization failed");
        MessageBoxA(nullptr, "Failed to initialize loader UI.", "Rust External", MB_ICONERROR | MB_OK);
        return -1;
    }

    auto result = loader.Run();
    LoaderLog("Run() returned");
    if (!result.shouldLaunch || !result.cheat) {
        LoaderLog("Exiting without launching cheat (user cancelled or auth failed)");
        return 0;
    }

    try {
        LoaderLog("Launching cheat instance");
        result.cheat->Run();
    } catch (...) {
        MessageBoxA(nullptr, "Runtime error occurred. The cheat will shut down.", "Rust External", MB_ICONERROR | MB_OK);
        LoaderLog("Cheat Run() threw exception");
    }

    result.cheat->Shutdown();
    LoaderLog("Cheat shutdown complete");
    return 0;
}
