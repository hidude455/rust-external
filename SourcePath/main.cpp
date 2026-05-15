#include "ExternalCheat.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <conio.h>
#include <thread>
#include <chrono>

void SetConsoleColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void PrintBanner() {
    system("cls");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << R"(
    ____             __     _____      __        __              __
   / __ \__  _______/ /_   / ___/___  / /_____ _/ /____  _____  / /
  / /_/ / / / / ___/ __/   \__ \/ _ \/ __/ __ `/ __/ _ \/ ___/ / /
 / _, _/ /_/ (__  ) /_    ___/ /  __/ /_/ /_/ / /_/  __/ /__  /_/
/_/ |_|\__,_/____/\__/   /____/\___/\__/\__,_/\__/\___/\___/ (_)

)" << '\n';
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "                    External v2.0 | Premium Edition\n";
    std::cout << "  ============================================================\n\n";
}

void LoadingAnimation(const std::string& message, int durationMs) {
    const char spinner[] = { '|', '/', '-', '\\' };
    int steps = durationMs / 100;

    for (int i = 0; i < steps; i++) {
        std::cout << "\r  [" << spinner[i % 4] << "] " << message << std::flush;
        Sleep(100);
    }
    std::cout << "\r  [+] " << message << " - Done!                \n";
}

int main() {
    SetConsoleTitleA("Rust External v2.0 | Premium Edition");

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

    PrintBanner();

    std::cout << "  Welcome to Rust External Premium\n";
    std::cout << "  ----------------------------------------\n\n";

    std::string licenseKey;
    std::cout << "  Enter License Key: ";
    SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);

    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b' && !licenseKey.empty()) {
            licenseKey.pop_back();
            std::cout << "\b \b";
        } else if (ch >= 32 && ch <= 126) {
            licenseKey += ch;
            std::cout << '*';
        }
    }

    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "\n\n";

    App::CExternalCheat cheat;

    LoadingAnimation("Verifying license...", 1500);

    if (!cheat.Startup(licenseKey)) {
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        std::cout << "\n  [!] Failed to start. Press any key to exit...";
        _getch();
        return -1;
    }

    SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "\n  [+] Cheat loaded successfully!\n";
    std::cout << "  [+] Press INSERT in-game to open the menu\n";
    std::cout << "  [+] The overlay will appear on your game window\n\n";

    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "  Minimize this window and return to your game.\n";
    std::cout << "  DO NOT close this window - it keeps the cheat running.\n\n";

    try {
        cheat.Run();
    } catch (...) {
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        std::cout << "\n  [!] An error occurred. Shutting down...\n";
    }

    cheat.Shutdown();

    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "\n  Thank you for using Rust External Premium!\n";
    std::cout << "  Press any key to exit...";
    _getch();

    return 0;
}
