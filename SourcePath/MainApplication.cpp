/*
 * MIT Method Main Application
 * Core entry point with stealth and protection
 * Author: Anonymous
 * Last Modified: 2026
 * 
 * This is the main entry point for our game enhancement utility.
 * Implements stealth techniques, anti-detection, and polymorphic code.
 */

#include "Core.h"
#include "MemoryAccess.h"
#include "RendererDX.h"
#include "AdvancedMenu.h"
#include "CodeMutator.h"
#include "Protection.h"
#include "../MenuPath/imgui/imgui.h"
#include <windows.h>
#include <iostream>
#include <memory>
#include <algorithm>

using namespace GameEnhance;

// Global application state
namespace {
    std::unique_ptr<GameEnhance::CMemoryAccess> g_memorySystem;
    std::unique_ptr<GameEnhance::CDXRenderer> g_renderSystem;
    std::unique_ptr<UI::CAdvancedMenu> g_userInterface;
    std::unique_ptr<GameEnhance::CCodeMutator> g_codeMutator;
    std::unique_ptr<Security::CProtection> g_protectionSystem;
    
    HWND g_targetWindow = nullptr;
    bool g_applicationRunning = true;
    bool g_stealthMode = true;
    
    // Performance tracking
    LARGE_INTEGER g_appStartTime;
    LARGE_INTEGER g_lastFrameTime;
    float g_totalRuntime = 0.0f;
    int g_frameCount = 0;
}

// Forward declarations for internal functions
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool InitializeApplicationComponents();
void CleanupApplicationComponents();
void MainApplicationLoop();
void HandleSystemInput();
void PerformPeriodicTasks();
bool AttachToTargetProcess();
void UpdatePerformanceMetrics();

// Anti-analysis entry point obfuscation
__declspec(noinline) void StealthEntryPoint() {
    // Generate some junk code to confuse analysis
    volatile int counter = 0;
    for (int i = 0; i < RandomGen::GetRandomInt(10, 50); ++i) {
        counter ^= i;
        counter = (counter << 1) | (counter >> 31);
    }
    
    // Random delay to confuse timing analysis
    Sleep(RandomGen::GetRandomInt(100, 500));
    
    // More junk operations
    volatile float junk = 3.14159f;
    for (int i = 0; i < RandomGen::GetRandomInt(5, 20); ++i) {
        junk *= 1.618f;
        junk = fmod(junk, 2.0f);
    }
}

// Main application entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize stealth measures immediately
    StealthEntryPoint();
    
    // Get application start time for performance tracking
    QueryPerformanceCounter(&g_appStartTime);
    QueryPerformanceCounter(&g_lastFrameTime);
    
    // Initialize protection system first
    g_protectionSystem = std::make_unique<Security::CProtection>();
    if (!g_protectionSystem->Initialize()) {
        // Protection failed - this is critical
        MessageBoxA(nullptr, "Protection system failed to initialize", "Critical Error", MB_ICONERROR);
        return -1;
    }
    
    // Initialize code mutator for polymorphic behavior
    g_codeMutator = std::make_unique<GameEnhance::CCodeMutator>();
    if (!g_codeMutator->Initialize()) {
        // Code mutation failed - continue but log warning
        OutputDebugStringA("Warning: Code mutator failed to initialize\n");
    }
    
    // Enable stealth mode if requested
    if (g_stealthMode) {
        g_protectionSystem->EnableStealthMode();
        g_codeMutator->EnableStealthMode();
    }
    
    // Initialize all application components
    if (!InitializeApplicationComponents()) {
        CleanupApplicationComponents();
        return -1;
    }
    
    // Main application loop
    MainApplicationLoop();
    
    // Cleanup before exit
    CleanupApplicationComponents();
    
    return 0;
}

bool InitializeApplicationComponents() {
    // Find target game window
    g_targetWindow = FindWindowA(nullptr, "Rust"); // Game-specific window title
    if (!g_targetWindow) {
        MessageBoxA(nullptr, "Target game window not found. Please ensure the game is running.", 
                   "Initialization Error", MB_ICONWARNING);
        return false;
    }
    
    // Initialize memory access system
    g_memorySystem = std::make_unique<GameEnhance::CMemoryAccess>();
    if (!g_memorySystem->Initialize("Rust.exe")) { // Game-specific process name
        MessageBoxA(nullptr, "Failed to initialize memory access system", 
                   "Initialization Error", MB_ICONERROR);
        return false;
    }
    
    // Initialize DirectX renderer
    g_renderSystem = std::make_unique<GameEnhance::CDXRenderer>();
    if (!g_renderSystem->Initialize(g_targetWindow)) {
        MessageBoxA(nullptr, "Failed to initialize rendering system", 
                   "Initialization Error", MB_ICONERROR);
        return false;
    }
    
    // Initialize user interface
    g_userInterface = std::make_unique<UI::CAdvancedMenu>(g_renderSystem.get());
    g_userInterface->Initialize();
    
    // Set up periodic mutation
    if (g_codeMutator) {
        g_codeMutator->SetMutationInterval(3000); // Mutate every 3 seconds
    }
    
    return true;
}

void CleanupApplicationComponents() {
    g_applicationRunning = false;
    
    // Cleanup in reverse order of initialization
    
    if (g_userInterface) {
        g_userInterface->Shutdown();
        g_userInterface.reset();
    }
    
    if (g_renderSystem) {
        g_renderSystem->Shutdown();
        g_renderSystem.reset();
    }
    
    if (g_memorySystem) {
        g_memorySystem->Shutdown();
        g_memorySystem.reset();
    }
    
    if (g_codeMutator) {
        g_codeMutator->Shutdown();
        g_codeMutator.reset();
    }
    
    if (g_protectionSystem) {
        g_protectionSystem->Shutdown();
        g_protectionSystem.reset();
    }
}

void MainApplicationLoop() {
    MSG msg = {};
    ZeroMemory(&msg, sizeof(msg));
    
    // Periodic task timer
    DWORD lastPeriodicTask = GetTickCount();
    const DWORD PERIODIC_TASK_INTERVAL = 1000; // 1 second
    
    while (g_applicationRunning) {
        // Process Windows messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_QUIT) {
                g_applicationRunning = false;
                break;
            }
        }
        
        if (!g_applicationRunning) break;
        
        // Check if target process is still valid
        if (!g_memorySystem || !g_memorySystem->IsProcessValid()) {
            // Try to reattach
            if (!AttachToTargetProcess()) {
                // Wait a bit before retrying
                Sleep(1000);
                continue;
            }
        }
        
        // Handle system input
        HandleSystemInput();
        
        // Perform periodic tasks
        DWORD currentTime = GetTickCount();
        if (currentTime - lastPeriodicTask >= PERIODIC_TASK_INTERVAL) {
            PerformPeriodicTasks();
            lastPeriodicTask = currentTime;
        }
        
        // Update performance metrics
        UpdatePerformanceMetrics();
        
        // Start rendering frame
        if (g_renderSystem) {
            g_renderSystem->BeginFrame();
        }
        
        // Render user interface
        if (g_userInterface && g_userInterface->IsMenuVisible()) {
            g_userInterface->Render();
        }
        
        // End frame and present
        if (g_renderSystem) {
            g_renderSystem->EndFrame();
            g_renderSystem->RenderFrame();
        }
        
        // Small delay to prevent excessive CPU usage
        Sleep(1);
        
        g_frameCount++;
    }
}

void HandleSystemInput() {
    // Toggle menu with INSERT key
    if (GetAsyncKeyState(VK_INSERT) & 1) {
        if (g_userInterface) {
            g_userInterface->ToggleMenu();
        }
    }
    
    // Quick toggle functions
    if (GetAsyncKeyState(VK_F1) & 1) {
        // Toggle ESP (would need to be implemented in ESP system)
        if (g_userInterface) {
            g_userInterface->ShowNotification("ESP Toggle", ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
        }
    }
    
    if (GetAsyncKeyState(VK_F2) & 1) {
        // Toggle recoil control
        if (g_userInterface) {
            g_userInterface->ShowNotification("Recoil Control Toggle", ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
        }
    }
    
    if (GetAsyncKeyState(VK_F3) & 1) {
        // Toggle aim assist
        if (g_userInterface) {
            g_userInterface->ShowNotification("Aim Assist Toggle", ImVec4(1.0f, 0.2f, 0.5f, 1.0f));
        }
    }
    
    // Emergency exit with END key
    if (GetAsyncKeyState(VK_END) & 1) {
        g_applicationRunning = false;
    }
}

void PerformPeriodicTasks() {
    // Perform code mutation
    if (g_codeMutator && g_codeMutator->IsMutationEnabled()) {
        g_codeMutator->MutateAllBlocks();
    }
    
    // Rotate encryption keys
    if (g_codeMutator) {
        g_codeMutator->RotateEncryptionKeys();
    }
    
    // Check protection system status
    if (g_protectionSystem && g_protectionSystem->IsCompromised()) {
        // Protection system detected compromise - take action
        g_applicationRunning = false;
        return;
    }
    
    // Update memory access patterns
    if (g_memorySystem) {
        g_memorySystem->FlushAccessHistory();
    }
    
    // Perform random junk operations for obfuscation
    if (RandomGen::GetRandomInt(0, 100) < 10) { // 10% chance
        volatile int junk = RandomGen::GetRandomInt();
        for (int i = 0; i < RandomGen::GetRandomInt(5, 15); ++i) {
            junk = (junk * 1103515245 + 12345) & 0x7fffffff;
        }
    }
}

bool AttachToTargetProcess() {
    if (!g_memorySystem) return false;
    
    // Try to reattach to the process
    bool reattached = g_memorySystem->Initialize("Rust.exe");
    
    if (reattached && g_userInterface) {
        g_userInterface->ShowNotification("Reattached to target process", 
                                      ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
    }
    
    return reattached;
}

void UpdatePerformanceMetrics() {
    LARGE_INTEGER currentTime, frequency;
    QueryPerformanceCounter(&currentTime);
    QueryPerformanceFrequency(&frequency);
    
    // Calculate frame time
    float frameTime = static_cast<float>(currentTime.QuadPart - g_lastFrameTime.QuadPart) / 
                     frequency.QuadPart * 1000.0f; // Convert to milliseconds
    
    // Update total runtime
    g_totalRuntime = static_cast<float>(currentTime.QuadPart - g_appStartTime.QuadPart) / 
                     frequency.QuadPart;
    
    g_lastFrameTime = currentTime;
    
    // Log performance issues
    if (frameTime > 50.0f) { // Frame took longer than 50ms
        char buffer[256];
        sprintf_s(buffer, "Performance warning: Frame time %.2f ms", frameTime);
        OutputDebugStringA(buffer);
    }
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Forward input to ImGui for menu interaction
    if (g_userInterface) {
        if (g_userInterface->HandleInput(uMsg, wParam, lParam)) {
            return true;
        }
    }
    
    switch (uMsg) {
        case WM_KEYDOWN:
            if (wParam == VK_INSERT) {
                if (g_userInterface) {
                    g_userInterface->ToggleMenu();
                }
                return 0;
            }
            break;
            
        case WM_DESTROY:
            g_applicationRunning = false;
            PostQuitMessage(0);
            return 0;
            
        case WM_SIZE:
            if (g_renderSystem) {
                g_renderSystem->Resize(LOWORD(lParam), HIWORD(lParam));
            }
            return 0;
            
        case WM_PAINT:
            // Handle paint messages if needed
            break;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
