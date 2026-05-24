#include <windows.h>
#include "ExternalCheat.h"
#include <memory>

// Global cheat instance
std::unique_ptr<App::CExternalCheat> g_cheat;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // DLL has been injected into the process
        DisableThreadLibraryCalls(hModule);
        
        // Initialize the cheat
        g_cheat = std::make_unique<App::CExternalCheat>();
        if (g_cheat) {
            g_cheat->Initialize();
        }
        break;

    case DLL_PROCESS_DETACH:
        // DLL is being unloaded
        if (g_cheat) {
            g_cheat->Shutdown();
            g_cheat.reset();
        }
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
