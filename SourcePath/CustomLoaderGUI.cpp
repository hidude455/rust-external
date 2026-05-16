#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include "AdvancedSpoofer.h"
#include "RustAntiCheatEvasion.h"
#include "AdvancedObfuscation.h"

using namespace Spoofer;
using namespace RustEvasion;
using namespace AdvancedObfuscation;

// Custom GUI Colors
const int COLOR_BG = 0x0A0A0A;
const int COLOR_ACCENT = 0x00D4FF;
const int COLOR_SUCCESS = 0x00FF88;
const int COLOR_ERROR = 0xFF4444;
const int COLOR_WARNING = 0xFFAA00;
const int COLOR_TEXT = 0xFFFFFF;
const int COLOR_PURPLE = 0x9B59B6;
const int COLOR_CYAN = 0x00CED1;
const int COLOR_ORANGE = 0xFF8C00;

class CustomLoaderGUI {
private:
    HANDLE hConsole;
    CAdvancedSpoofer* spoofer;
    CRustAntiCheatEvasion* rustEvasion;
    bool initialized;
    bool bypassActive;
    std::string m_menuBuffer;
    std::string m_statusBuffer;
    
    void SetColor(int color) {
        SetConsoleTextAttribute(hConsole, color);
    }
    
    void ResetColor() {
        SetConsoleTextAttribute(hConsole, COLOR_TEXT);
    }
    
    void ClearScreen() {
        system("cls");
    }
    
    void DrawHeader() {
        SetColor(COLOR_ACCENT);
        std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║   ███████╗██╗   ██╗██████╗ ███████╗██████╗                   ║
║   ██╔════╝██║   ██║██╔══██╗██╔════╝██╔══██╗                  ║
║   ███████╗██║   ██║██████╔╝█████╗  ██████╔╝                  ║
║   ╚════██║██║   ██║██╔══██╗██╔══╝  ██╔══██╗                  ║
║   ███████║╚██████╔╝██████╔╝███████╗██║  ██║                  ║
║   ╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝                  ║
║                                                              ║
║        ███████╗ ██████╗ ███████╗ █████╗ ██████╗ ███████╗██╗  ║
║        ██╔════╝██╔═══██╗██╔════╝██╔══██╗██╔══██╗██╔════╝██║  ║
║        █████╗  ██║   ██║█████╗  ███████║██████╔╝█████╗  ██║  ║
║        ██╔══╝  ██║   ██║██╔══╝  ██╔══██║██╔══██╗██╔══╝  ╚═╝  ║
║        ██║     ╚██████╔╝███████╗██║  ██║██║  ██║███████╗██╗  ║
║        ╚═╝      ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ║
║                                                              ║
║              ADVANCED HWID SPOOFER v3.0                      ║
║              RUST EDITION - STEALTH MODE                      ║
║              ═════════════════════════════════════════════════ ║
║              [PROTECTED] [UNDetectable] [STEALTH]              ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝
)";
        ResetColor();
    }
    
    void DrawProgressBar(int progress, int width = 50) {
        std::cout << "[";
        int filled = (progress * width) / 100;
        for (int i = 0; i < width; i++) {
            if (i < filled) {
                // Gradient effect for progress bar
                if (i < width / 3) {
                    SetColor(COLOR_CYAN);
                } else if (i < (width * 2) / 3) {
                    SetColor(COLOR_ACCENT);
                } else {
                    SetColor(COLOR_SUCCESS);
                }
                std::cout << "█";
            } else {
                SetColor(COLOR_BG);
                std::cout << "░";
            }
        }
        ResetColor();
        std::cout << "] " << progress << "%";
        
        // Add status indicator based on progress
        if (progress < 30) {
            SetColor(COLOR_WARNING);
            std::cout << " [INITIALIZING]";
        } else if (progress < 70) {
            SetColor(COLOR_ACCENT);
            std::cout << " [PROCESSING]";
        } else if (progress < 100) {
            SetColor(COLOR_CYAN);
            std::cout << " [FINALIZING]";
        } else {
            SetColor(COLOR_SUCCESS);
            std::cout << " [COMPLETE]";
        }
        ResetColor();
    }
    
    void AnimateText(const std::string& text, int delayMs = 30) {
        for (char c : text) {
            std::cout << c;
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        }
    }
    
    void DrawStatus(const std::string& status, bool success = true) {
        SetColor(success ? COLOR_SUCCESS : COLOR_ERROR);
        std::cout << "[" << (success ? "+" : "-") << "] ";
        ResetColor();
        std::cout << status << std::endl;
    }
    
    void DrawWarning(const std::string& warning) {
        SetColor(COLOR_WARNING);
        std::cout << "[!] ";
        ResetColor();
        std::cout << warning << std::endl;
    }
    
    void DrawInfo(const std::string& info) {
        SetColor(COLOR_ACCENT);
        std::cout << "[*] ";
        ResetColor();
        std::cout << info << std::endl;
    }
    
    void DrawSeparator() {
        SetColor(COLOR_ACCENT);
        std::cout << "═══════════════════════════════════════════════════════════════";
        ResetColor();
        std::cout << std::endl;
    }
    
    void SimulateLoading(const std::string& task) {
        DrawInfo(task);
        for (int i = 0; i <= 100; i += 5) {
            std::cout << "\r";
            DrawProgressBar(i);
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        std::cout << std::endl;
    }
    
public:
    CustomLoaderGUI() : hConsole(GetStdHandle(STD_OUTPUT_HANDLE)), 
                       spoofer(nullptr), rustEvasion(nullptr), 
                       initialized(false), bypassActive(false) {
        SetConsoleOutputCP(CP_UTF8);
        
        // Reserve memory for strings to avoid reallocations
        m_menuBuffer.reserve(1024);
        m_statusBuffer.reserve(256);
        
        // Initialize obfuscation
        ControlFlowObfuscator::RandomDelay();
        ThreadObfuscator::CreateJunkThread();
    }
    
    ~CustomLoaderGUI() {
        if (spoofer) delete spoofer;
        if (rustEvasion) delete rustEvasion;
    }
    
    bool Initialize() {
        ClearScreen();
        DrawHeader();
        DrawSeparator();
        
        // Apply timing obfuscation
        TimingObfuscator::ObfuscateTiming();
        
        SimulateLoading("Initializing Advanced Spoofer Core...");
        
        spoofer = new CAdvancedSpoofer();
        SpooferConfig config;
        config.randomizeAll = true;
        config.enableWMIProtection = true;
        config.enableRegistryProtection = true;
        config.enableDriverProtection = true;
        config.enableNetworkProtection = true;
        config.enablePCISpoofing = true;
        config.createBackup = true;
        config.restoreOnExit = false;
        
        // Obfuscate configuration data
        MemoryObfuscator::ScrambleMemory(&config, sizeof(config));
        
        if (!spoofer->Initialize(config)) {
            DrawStatus("Spoofer initialization failed", false);
            return false;
        }
        
        // Unscramble after initialization
        MemoryObfuscator::UnscrambleMemory(&config, sizeof(config));
        
        DrawStatus("Advanced Spoofer initialized successfully");
        
        // Add random delay for stealth
        ControlFlowObfuscator::RandomDelay();
        
        SimulateLoading("Initializing Rust EAC Evasion System...");
        
        rustEvasion = new CRustAntiCheatEvasion();
        RustEvasionConfig evasionConfig;
        evasionConfig.bypassEACKernel = true;
        evasionConfig.bypassEACUserMode = true;
        evasionConfig.hideFromEACScanner = true;
        evasionConfig.spoofEACIdentifiers = true;
        evasionConfig.bypassEACIntegrity = true;
        evasionConfig.bypassEACTiming = true;
        evasionConfig.bypassEACNetwork = true;
        evasionConfig.bypassEACBehavior = true;
        evasionConfig.enableKernelDriver = true;
        evasionConfig.enableUserModeHooking = true;
        evasionConfig.enableMemoryObfuscation = true;
        evasionConfig.enableProcessHiding = true;
        evasionConfig.enableThreadHiding = true;
        evasionConfig.enableModuleHiding = true;
        evasionConfig.stealthMode = true;
        evasionConfig.aggressiveMode = false;
        evasionConfig.paranoidMode = false;
        
        // Obfuscate evasion configuration
        MemoryObfuscator::ScrambleMemory(&evasionConfig, sizeof(evasionConfig));
        
        if (!rustEvasion->Initialize(evasionConfig)) {
            DrawStatus("Rust EAC evasion initialization failed", false);
            return false;
        }
        
        // Unscramble after initialization
        MemoryObfuscator::UnscrambleMemory(&evasionConfig, sizeof(evasionConfig));
        
        DrawStatus("Rust EAC Evasion System initialized successfully");
        
        initialized = true;
        DrawSeparator();
        DrawStatus("All systems operational - Ready for action");
        DrawSeparator();
        
        // Additional timing obfuscation
        TimingObfuscator::RandomSleep(50, 150);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return true;
    }
    
    void ShowMainMenu() {
        while (true) {
            ClearScreen();
            DrawHeader();
            DrawSeparator();
            
            SetColor(COLOR_ACCENT);
            std::cout << "                    MAIN MENU                       \n";
            ResetColor();
            DrawSeparator();
            
            std::cout << "\n";
            SetColor(COLOR_TEXT);
            std::cout << "  [1] ";
            SetColor(COLOR_SUCCESS);
            std::cout << "FULL SYSTEM SPOOF";
            ResetColor();
            std::cout << " - All identifiers + EAC Bypass\n";
            
            std::cout << "  [2] ";
            SetColor(COLOR_CYAN);
            std::cout << "RUST OPTIMIZED";
            ResetColor();
            std::cout << " - Game-specific spoofing\n";
            
            std::cout << "  [3] ";
            SetColor(COLOR_WARNING);
            std::cout << "RESTORE SYSTEM";
            ResetColor();
            std::cout << " - Revert all changes\n";
            
            std::cout << "  [4] ";
            SetColor(COLOR_SUCCESS);
            std::cout << "ACTIVATE EAC BYPASS";
            ResetColor();
            std::cout << bypassActive ? " [ACTIVE]" : " [INACTIVE]\n";
            
            std::cout << "  [5] ";
            SetColor(COLOR_ERROR);
            std::cout << "DEACTIVATE EAC BYPASS";
            ResetColor();
            std::cout << "\n";
            
            std::cout << "  [6] ";
            SetColor(COLOR_PURPLE);
            std::cout << "SYSTEM STATUS";
            ResetColor();
            std::cout << " - View detailed info\n";
            
            std::cout << "  [7] ";
            SetColor(COLOR_ORANGE);
            std::cout << "ADVANCED OPTIONS";
            ResetColor();
            std::cout << " - Configure settings\n";
            
            std::cout << "  [8] ";
            SetColor(COLOR_WARNING);
            std::cout << "EXIT";
            ResetColor();
            std::cout << " - Safe shutdown\n";
            
            DrawSeparator();
            
            // Add footer with status
            SetColor(COLOR_ACCENT);
            std::cout << "\n  Status: ";
            SetColor(initialized ? COLOR_SUCCESS : COLOR_WARNING);
            std::cout << (initialized ? "ONLINE" : "OFFLINE");
            ResetColor();
            
            SetColor(COLOR_ACCENT);
            std::cout << " | Protection: ";
            SetColor(COLOR_SUCCESS);
            std::cout << "ACTIVE";
            ResetColor();
            
            SetColor(COLOR_ACCENT);
            std::cout << " | Mode: ";
            SetColor(COLOR_CYAN);
            std::cout << "STEALTH";
            ResetColor();
            
            std::cout << "\n\n  Select option: ";
            
            int choice;
            std::cin >> choice;
            
            switch (choice) {
                case 1:
                    FullSystemSpoof();
                    break;
                case 2:
                    RustOptimizedSpoof();
                    break;
                case 3:
                    RestoreSystem();
                    break;
                case 4:
                    ActivateEACBypass();
                    break;
                case 5:
                    DeactivateEACBypass();
                    break;
                case 6:
                    ShowSystemStatus();
                    break;
                case 7:
                    ShowAdvancedOptions();
                    break;
                case 8:
                    Shutdown();
                    return;
                default:
                    DrawWarning("Invalid option selected");
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    break;
            }
        }
    }
    
    void ShowAdvancedOptions() {
        ClearScreen();
        DrawHeader();
        DrawSeparator();
        SetColor(COLOR_ORANGE);
        std::cout << "                    ADVANCED OPTIONS                 \n";
        ResetColor();
        DrawSeparator();
        
        std::cout << "\n";
        SetColor(COLOR_TEXT);
        std::cout << "  [1] ";
        SetColor(COLOR_CYAN);
        std::cout << "Enable Aggressive Mode";
        ResetColor();
        std::cout << " - Maximum stealth (may affect performance)\n";
        
        std::cout << "  [2] ";
        SetColor(COLOR_PURPLE);
        std::cout << "Enable Paranoid Mode";
        ResetColor();
        std::cout << " - Extra protection checks\n";
        
        std::cout << "  [3] ";
        SetColor(COLOR_ACCENT);
        std::cout << "Enable Kernel Driver";
        ResetColor();
        std::cout << " - Use kernel-level protection\n";
        
        std::cout << "  [4] ";
        SetColor(COLOR_WARNING);
        std::cout << "Back to Main Menu";
        ResetColor();
        std::cout << "\n";
        
        DrawSeparator();
        std::cout << "\n  Select option: ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                DrawInfo("Aggressive mode enabled - Maximum stealth active");
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                break;
            case 2:
                DrawInfo("Paranoid mode enabled - Extra protection checks active");
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                break;
            case 3:
                DrawInfo("Kernel driver protection enabled");
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                break;
            case 4:
                return;
            default:
                DrawWarning("Invalid option selected");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                break;
        }
    }
    
    void FullSystemSpoof() {
        ClearScreen();
        DrawHeader();
        DrawSeparator();
        DrawInfo("Starting full system spoof...");
        DrawSeparator();
        
        SimulateLoading("Spoofing Hardware Identifiers...");
        if (spoofer->SpoofAll()) {
            DrawStatus("Hardware spoofing completed");
        } else {
            DrawStatus("Hardware spoofing failed", false);
        }
        
        SimulateLoading("Activating EAC Bypass...");
        if (rustEvasion->ActivateBypass()) {
            DrawStatus("EAC Bypass activated");
            bypassActive = true;
        } else {
            DrawStatus("EAC Bypass activation failed", false);
        }
        
        DrawSeparator();
        DrawStatus("Full system spoof completed successfully");
        DrawSeparator();
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    void RustOptimizedSpoof() {
        ClearScreen();
        DrawHeader();
        DrawSeparator();
        DrawInfo("Starting Rust-optimized spoof...");
        DrawSeparator();
        
        SimulateLoading("Applying Rust-specific spoofing...");
        if (spoofer->SpoofForRust()) {
            DrawStatus("Rust spoofing completed");
        } else {
            DrawStatus("Rust spoofing failed", false);
        }
        
        SimulateLoading("Activating EAC Bypass...");
        if (rustEvasion->ActivateBypass()) {
            DrawStatus("EAC Bypass activated");
            bypassActive = true;
        } else {
            DrawStatus("EAC Bypass activation failed", false);
        }
        
        DrawSeparator();
        DrawStatus("Rust-optimized spoof completed successfully");
        DrawWarning("Launch Rust game now for best results");
        DrawSeparator();
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    void RestoreSystem() {
        ClearScreen();
        DrawHeader();
        DrawSeparator();
        DrawWarning("This will restore all original identifiers");
        DrawSeparator();
        
        SimulateLoading("Restoring system identifiers...");
        if (spoofer->RestoreAll()) {
            DrawStatus("System restore completed");
        } else {
            DrawStatus("System restore failed", false);
        }
        
        DrawSeparator();
        DrawStatus("All identifiers restored to original values");
        DrawSeparator();
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    void ActivateEACBypass() {
        if (bypassActive) {
            DrawWarning("EAC Bypass is already active");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return;
        }
        
        SimulateLoading("Activating EAC Bypass...");
        if (rustEvasion->ActivateBypass()) {
            DrawStatus("EAC Bypass activated successfully");
            bypassActive = true;
        } else {
            DrawStatus("EAC Bypass activation failed", false);
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    void DeactivateEACBypass() {
        if (!bypassActive) {
            DrawWarning("EAC Bypass is not active");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return;
        }
        
        SimulateLoading("Deactivating EAC Bypass...");
        if (rustEvasion->DeactivateBypass()) {
            DrawStatus("EAC Bypass deactivated successfully");
            bypassActive = false;
        } else {
            DrawStatus("EAC Bypass deactivation failed", false);
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    void ShowSystemStatus() {
        ClearScreen();
        DrawHeader();
        DrawSeparator();
        SetColor(COLOR_ACCENT);
        std::cout << "                    SYSTEM STATUS                    \n";
        ResetColor();
        DrawSeparator();
        
        std::cout << "\n";
        SetColor(COLOR_ACCENT);
        std::cout << "SPOOFER STATUS:\n";
        ResetColor();
        std::cout << spoofer->GetStatusReport() << "\n";
        
        DrawSeparator();
        
        SetColor(COLOR_ACCENT);
        std::cout << "EAC BYPASS STATUS:\n";
        ResetColor();
        std::cout << rustEvasion->GetStatusReport() << "\n";
        
        DrawSeparator();
        
        SetColor(COLOR_ACCENT);
        std::cout << "SYSTEM INFO:\n";
        ResetColor();
        std::cout << "  Bypass Active: " << (bypassActive ? "YES" : "NO") << "\n";
        std::cout << "  System Initialized: " << (initialized ? "YES" : "NO") << "\n";
        std::cout << "  Stealth Mode: ENABLED\n";
        std::cout << "  Protection Level: MAXIMUM\n";
        
        DrawSeparator();
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    void Shutdown() {
        ClearScreen();
        DrawHeader();
        DrawSeparator();
        DrawInfo("Initiating safe shutdown sequence...");
        DrawSeparator();
        
        SimulateLoading("Deactivating EAC Bypass...");
        if (rustEvasion) {
            rustEvasion->DeactivateBypass();
        }
        
        SimulateLoading("Restoring system identifiers...");
        if (spoofer) {
            spoofer->RestoreAll();
        }
        
        DrawSeparator();
        DrawStatus("Shutdown completed successfully");
        DrawSeparator();
        
        SetColor(COLOR_ACCENT);
        std::cout << "\n  Thank you for using Advanced HWID Spoofer v3.0\n";
        std::cout << "  Stay safe and undetected!\n\n";
        ResetColor();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
};

int main() {
    CustomLoaderGUI gui;
    
    if (!gui.Initialize()) {
        std::cout << "\nFatal error: Failed to initialize system\n";
        system("pause");
        return 1;
    }
    
    gui.ShowMainMenu();
    
    return 0;
}
