#include "AdvancedSpoofer.h"
#include "SpooferGUI.h"
#include "RustAntiCheatEvasion.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace Spoofer;
using namespace SpooferGUI;
using namespace RustEvasion;

int main() {
    std::cout << "========================================\n";
    std::cout << "   HWID Spoofer - Rust Edition v2.0   \n";
    std::cout << "========================================\n\n";

    // Initialize spoofer
    std::cout << "[*] Initializing Advanced Spoofer...\n";
    CAdvancedSpoofer* spoofer = new CAdvancedSpoofer();
    
    SpooferConfig config;
    config.randomizeAll = true;
    config.enableWMIProtection = true;
    config.enableRegistryProtection = true;
    config.enableDriverProtection = true;
    config.enableNetworkProtection = true;
    config.enablePCISpoofing = true;
    config.createBackup = true;
    config.restoreOnExit = false;
    
    if (!spoofer->Initialize(config)) {
        std::cout << "[!] Failed to initialize spoofer\n";
        delete spoofer;
        system("pause");
        return 1;
    }
    std::cout << "[+] Spoofer initialized successfully\n\n";

    // Initialize Rust EAC evasion
    std::cout << "[*] Initializing Rust EAC Evasion...\n";
    CRustAntiCheatEvasion* rustEvasion = new CRustAntiCheatEvasion();
    
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
    
    if (!rustEvasion->Initialize(evasionConfig)) {
        std::cout << "[!] Failed to initialize Rust EAC evasion\n";
        delete rustEvasion;
        delete spoofer;
        system("pause");
        return 1;
    }
    std::cout << "[+] Rust EAC evasion initialized successfully\n\n";

    // Menu loop
    while (true) {
        std::cout << "========================================\n";
        std::cout << "           SPOOFER MENU               \n";
        std::cout << "========================================\n";
        std::cout << "1. Spoof All Identifiers\n";
        std::cout << "2. Spoof for Rust (Recommended)\n";
        std::cout << "3. Restore All Identifiers\n";
        std::cout << "4. Activate EAC Bypass\n";
        std::cout << "5. Deactivate EAC Bypass\n";
        std::cout << "6. View Spoofer Status\n";
        std::cout << "7. View EAC Bypass Status\n";
        std::cout << "8. Exit\n";
        std::cout << "========================================\n";
        std::cout << "Select option: ";

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "\n[*] Spoofing all identifiers...\n";
                if (spoofer->SpoofAll()) {
                    std::cout << "[+] Spoofing completed successfully\n";
                } else {
                    std::cout << "[!] Spoofing failed\n";
                }
                break;

            case 2:
                std::cout << "\n[*] Spoofing for Rust...\n";
                if (spoofer->SpoofForRust()) {
                    std::cout << "[+] Rust spoofing completed successfully\n";
                    // Auto-activate EAC bypass
                    if (rustEvasion->ActivateBypass()) {
                        std::cout << "[+] EAC bypass activated\n";
                    } else {
                        std::cout << "[!] EAC bypass activation failed\n";
                    }
                } else {
                    std::cout << "[!] Rust spoofing failed\n";
                }
                break;

            case 3:
                std::cout << "\n[*] Restoring all identifiers...\n";
                if (spoofer->RestoreAll()) {
                    std::cout << "[+] Restore completed successfully\n";
                } else {
                    std::cout << "[!] Restore failed\n";
                }
                break;

            case 4:
                std::cout << "\n[*] Activating EAC bypass...\n";
                if (rustEvasion->ActivateBypass()) {
                    std::cout << "[+] EAC bypass activated\n";
                } else {
                    std::cout << "[!] EAC bypass activation failed\n";
                }
                break;

            case 5:
                std::cout << "\n[*] Deactivating EAC bypass...\n";
                if (rustEvasion->DeactivateBypass()) {
                    std::cout << "[+] EAC bypass deactivated\n";
                } else {
                    std::cout << "[!] EAC bypass deactivation failed\n";
                }
                break;

            case 6:
                std::cout << "\n" << spoofer->GetStatusReport() << "\n";
                break;

            case 7:
                std::cout << "\n" << rustEvasion->GetStatusReport() << "\n";
                break;

            case 8:
                std::cout << "\n[*] Shutting down...\n";
                spoofer->Shutdown();
                rustEvasion->Shutdown();
                delete spoofer;
                delete rustEvasion;
                std::cout << "[+] Shutdown complete\n";
                return 0;

            default:
                std::cout << "\n[!] Invalid option\n";
                break;
        }

        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
        system("cls");
    }

    return 0;
}
