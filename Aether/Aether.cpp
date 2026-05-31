// Aether Loader - Bootstrapper for P Client / Rust Injector
// Console front-end with purple ASCII art + key authentication.

#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>
#include <array>
#include <ctime>
#include <cctype>
#include <cstdint>
#include <conio.h>

namespace fs = std::filesystem;

// ============================================================
//  Compile-time XOR obfuscation - keeps key strings out of the
//  raw binary so `strings.exe Aether.exe` reveals nothing useful.
// ============================================================
namespace obf {
    constexpr unsigned char kKey = 0x5C;  // any non-zero byte

    template <size_t N>
    struct XorStr {
        char data[N];
        constexpr XorStr(const char (&s)[N]) : data{} {
            for (size_t i = 0; i < N; ++i) data[i] = static_cast<char>(s[i] ^ kKey);
        }
    };

    inline std::string decode(const char* enc, size_t n) {
        std::string out; out.reserve(n);
        for (size_t i = 0; i < n - 1; ++i) out.push_back(static_cast<char>(enc[i] ^ kKey));
        return out;
    }
}

#define OBF(s) ([]{ static constexpr obf::XorStr x{s}; return obf::decode(x.data, sizeof(s)); }())

// ---------- ANSI color setup ----------
static void EnableVTMode() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (!GetConsoleMode(h, &mode)) return;
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

#define COL_RESET   "\033[0m"
#define COL_PURPLE  "\033[38;5;141m"   // soft purple
#define COL_VIOLET  "\033[38;5;99m"    // deeper violet (banner)
#define COL_MAGENTA "\033[38;5;201m"   // accent magenta
#define COL_DIM     "\033[38;5;240m"
#define COL_BRIGHT  "\033[38;5;255m"
#define COL_YELLOW  "\033[38;5;220m"
#define COL_HL_BG   "\033[48;5;99m\033[38;5;255m"

// ---------- ASCII Art: AETHER ----------
static const char* kBanner[] = {
    "  $$$$$$\\  $$$$$$$$\\ $$$$$$$$\\ $$\\   $$\\ $$$$$$$$\\ $$$$$$$\\  ",
    " $$  __$$\\ $$  _____|\\__$$  __|$$ |  $$ |$$  _____|$$  __$$\\ ",
    " $$ /  $$ |$$ |         $$ |   $$ |  $$ |$$ |      $$ |  $$ |",
    " $$$$$$$$ |$$$$$\\       $$ |   $$$$$$$$ |$$$$$\\    $$$$$$$  |",
    " $$  __$$ |$$  __|      $$ |   $$  __$$ |$$  __|   $$  __$$< ",
    " $$ |  $$ |$$ |         $$ |   $$ |  $$ |$$ |      $$ |  $$ |",
    " $$ |  $$ |$$$$$$$$\\    $$ |   $$ |  $$ |$$$$$$$$\\ $$ |  $$ |",
    " \\__|  \\__|\\________|   \\__|   \\__|  \\__|\\________|\\__|  \\__|",
};

static void PrintBanner() {
    std::cout << "\n";
    for (auto* line : kBanner) {
        std::cout << COL_VIOLET << line << COL_RESET << "\n";
    }
    std::cout << COL_DIM
              << "  -------------------------------------------------------------"
              << COL_RESET << "\n";
    std::cout << "  " << COL_PURPLE << "Bootstrapper" << COL_RESET
              << "  " << COL_BRIGHT << "v1.0.3 BETA" << COL_RESET
              << "   " << COL_DIM << "|" << COL_RESET
              << "   " << COL_MAGENTA << "aether.gg" << COL_RESET << "\n";
    std::cout << COL_DIM
              << "  -------------------------------------------------------------"
              << COL_RESET << "\n\n";
}

// ============================================================
//  Tiered Key System
//  Format:  XXXX-XXXX-XXXX-XXXX  (4 groups of 4, dashes literal)
//  - Groups 1, 2          : random body chars (charset below)
//  - Group  3, char 0     : tier code  D / W / M / L
//                            D = 1 day, W = 1 week,
//                            M = 1 month, L = lifetime
//  - Group  3, chars 1-3  : random body chars
//  - Group  4 (4 chars)   : checksum derived from groups 1-3 + secret
//
//  The key is self-validating - no central server, no key list to
//  maintain. The generator tool (KeyGenerator.cpp) produces keys
//  using the same algorithm.
//
//  Activation (first successful use) is recorded per-key in
//      %APPDATA%\Aether\activations.dat
//  so a Day/Week/Month key naturally expires; user must obtain a
//  new key once expired. Lifetime keys never expire.
// ============================================================

// Charset - letters, digits, and a few safe symbols.
// Excludes 0/O/1/I to reduce mis-reads.
// 24 letters (A-Z minus I,O) + 8 digits (2-9) + 5 symbols = 37 chars.
static const char kKeyCharset[] =
    "ABCDEFGHJKLMNPQRSTUVWXYZ23456789!#$%&";
static constexpr size_t kCharsetLen = sizeof(kKeyCharset) - 1;
static_assert(kCharsetLen == 37, "charset length mismatch");

// Tier codes are intentionally NOT in the body charset so the tier
// position is unambiguous and cannot collide with a body char.
// D, W, M, L do appear in the charset though - that's fine, the
// position (index 10, first char of group 3) is what carries the meaning.

// Secret salt - obfuscated. CHANGE THIS for your own deployment so
// nobody can mint keys without your private build of the generator.
static std::string GetKeySecret() {
    return OBF("Aether::v1::SeCrEt::Salt::CHANGE-ME-PLEASE::9f3a7c2e");
}

enum class KeyTier {
    Invalid = 0,
    Day,
    Week,
    Month,
    Lifetime,
};

struct KeyInfo {
    KeyTier tier = KeyTier::Invalid;
    long long durationSeconds = 0;   // 0 == lifetime / never expires
    std::string normalized;          // upper-cased, dashes preserved
};

static const char* TierName(KeyTier t) {
    switch (t) {
        case KeyTier::Day:      return "1 Day";
        case KeyTier::Week:     return "1 Week";
        case KeyTier::Month:    return "1 Month";
        case KeyTier::Lifetime: return "Lifetime";
        default:                return "Invalid";
    }
}

// FNV-1a 64-bit, deterministic across compilers.
static uint64_t Fnv1a64(const std::string& s) {
    uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return h;
}

// Compute the 4-char checksum for a key body (15 chars: 3 groups + 2 dashes).
static std::string ComputeChecksum(const std::string& body) {
    uint64_t h = Fnv1a64(body + GetKeySecret());
    std::string out(4, '?');
    for (int i = 0; i < 4; ++i) {
        out[i] = kKeyCharset[h % kCharsetLen];
        h /= kCharsetLen;
        // mix again so successive chars aren't trivially related
        h ^= (h << 13);
        h ^= (h >> 7);
        h ^= (h << 17);
    }
    return out;
}

// Normalize: uppercase, strip whitespace. Leaves dashes intact.
static std::string NormalizeKey(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
        if (std::isspace(static_cast<unsigned char>(c))) continue;
        out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    return out;
}

static bool IsBodyChar(char c) {
    for (size_t i = 0; i < kCharsetLen; ++i) {
        if (kKeyCharset[i] == c) return true;
    }
    return false;
}

// Parse and verify. Fills out KeyInfo on success.
static bool ParseKey(const std::string& raw, KeyInfo& out) {
    out = {};
    std::string k = NormalizeKey(raw);
    if (k.size() != 19) return false;                     // 16 + 3 dashes
    if (k[4] != '-' || k[9] != '-' || k[14] != '-') return false;

    // body chars (positions 0-3, 5-8, 10-13, 15-18) must be in charset
    static const int bodyPositions[16] = {
        0,1,2,3, 5,6,7,8, 10,11,12,13, 15,16,17,18
    };
    for (int p : bodyPositions) {
        if (!IsBodyChar(k[p])) return false;
    }

    char tierChar = k[10];   // first char of group 3
    KeyTier tier = KeyTier::Invalid;
    long long dur = 0;
    switch (tierChar) {
        case 'D': tier = KeyTier::Day;      dur = 24LL * 3600;        break;
        case 'W': tier = KeyTier::Week;     dur = 7LL * 24 * 3600;    break;
        case 'M': tier = KeyTier::Month;    dur = 30LL * 24 * 3600;   break;
        case 'L': tier = KeyTier::Lifetime; dur = 0;                  break;
        default: return false;
    }

    // Verify checksum: hash everything before the last group.
    std::string body = k.substr(0, 14);                    // groups 1-3 + 2 dashes
    std::string expected = ComputeChecksum(body);
    std::string given    = k.substr(15, 4);
    if (expected != given) return false;

    out.tier = tier;
    out.durationSeconds = dur;
    out.normalized = k;
    return true;
}

// ============================================================
//  Activation persistence
//  File: %APPDATA%\Aether\activations.dat
//  Format (one entry per line):  <hexhash> <epochSeconds>
//  Hash is FNV-1a64 of (normalized key + secret), so the raw key
//  is NOT stored on disk.
// ============================================================
static fs::path GetActivationsPath() {
    wchar_t* appdata = nullptr;
    fs::path base;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdata)) && appdata) {
        base = fs::path(appdata) / L"Aether";
        CoTaskMemFree(appdata);
    } else {
        base = fs::temp_directory_path() / "Aether";
    }
    std::error_code ec;
    fs::create_directories(base, ec);
    return base / "activations.dat";
}

static std::string KeyHashHex(const std::string& normalizedKey) {
    uint64_t h = Fnv1a64(normalizedKey + GetKeySecret() + "::activation");
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016llX", static_cast<unsigned long long>(h));
    return buf;
}

// Returns first-use epoch for this key, or 0 if not yet activated.
static long long ReadActivation(const std::string& hashHex) {
    std::ifstream f(GetActivationsPath());
    if (!f) return 0;
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::string h; long long t = 0;
        if ((iss >> h >> t) && h == hashHex) return t;
    }
    return 0;
}

static void WriteActivation(const std::string& hashHex, long long epoch) {
    // Append. Duplicates are harmless - first match wins on read.
    std::ofstream f(GetActivationsPath(), std::ios::app);
    if (!f) return;
    f << hashHex << ' ' << epoch << '\n';
}

// Result of a full validation attempt.
struct AuthResult {
    bool        ok          = false;
    bool        expired     = false;
    KeyTier     tier        = KeyTier::Invalid;
    long long   secondsLeft = 0;       // 0 + tier==Lifetime  -> infinite
};

static AuthResult AuthenticateKey(const std::string& raw) {
    AuthResult r;
    KeyInfo info;
    if (!ParseKey(raw, info)) return r;            // ok stays false

    std::string hashHex = KeyHashHex(info.normalized);
    long long now   = static_cast<long long>(std::time(nullptr));
    long long first = ReadActivation(hashHex);

    if (first == 0) {
        // First time we ever see this key on this machine -> activate now.
        first = now;
        WriteActivation(hashHex, first);
    }

    r.tier = info.tier;
    if (info.tier == KeyTier::Lifetime) {
        r.ok = true;
        r.secondsLeft = 0;   // sentinel: infinite
        return r;
    }

    long long elapsed = now - first;
    if (elapsed < 0) elapsed = 0;                  // clock skew safety
    long long left = info.durationSeconds - elapsed;
    if (left <= 0) {
        r.expired = true;
        r.ok = false;
        return r;
    }
    r.ok = true;
    r.secondsLeft = left;
    return r;
}

static std::string FormatRemaining(KeyTier tier, long long secondsLeft) {
    if (tier == KeyTier::Lifetime) return "Lifetime (never expires)";
    long long s = secondsLeft;
    long long days  = s / 86400; s %= 86400;
    long long hours = s / 3600;  s %= 3600;
    long long mins  = s / 60;
    std::ostringstream os;
    if (days  > 0) os << days  << "d ";
    if (hours > 0 || days > 0) os << hours << "h ";
    os << mins << "m";
    return os.str();
}

// ---------- Process helpers ----------
static DWORD FindProcessIdByName(const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W entry{ sizeof(entry) };
    if (Process32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, name) == 0) {
                CloseHandle(snap);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return 0;
}

static bool LaunchInjector(const fs::path& exePath) {
    STARTUPINFOW si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    std::wstring cmd = L"\"" + exePath.wstring() + L"\"";
    BOOL ok = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE,
                             0, nullptr, exePath.parent_path().c_str(), &si, &pi);
    if (ok) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return true;
    }
    return false;
}

// ---------- Main flow ----------
int main() {
    EnableVTMode();
    SetConsoleTitleW(L"Aether Loader");

    PrintBanner();

    std::string key;
    while (true) {
        std::cout << COL_PURPLE << "[+] " << COL_RESET
                  << COL_BRIGHT << "Enter authentication key "
                  << COL_DIM    << "(XXXX-XXXX-XXXX-XXXX)" << COL_RESET
                  << COL_BRIGHT << ": " << COL_RESET;
        std::getline(std::cin, key);

        AuthResult auth = AuthenticateKey(key);
        if (auth.ok) {
            std::cout << COL_PURPLE << "[+] " << COL_RESET
                      << COL_BRIGHT << "Key valid  "
                      << COL_DIM    << "[" << COL_RESET
                      << COL_MAGENTA << TierName(auth.tier) << COL_RESET
                      << COL_DIM    << "]" << COL_RESET
                      << COL_BRIGHT << "  Time remaining: " << COL_RESET
                      << COL_MAGENTA << FormatRemaining(auth.tier, auth.secondsLeft)
                      << COL_RESET << "\n";
            break;
        }
        if (auth.expired) {
            std::cout << COL_MAGENTA << "[-] " << COL_RESET
                      << COL_BRIGHT << "This " << TierName(auth.tier)
                      << " key has expired. Please obtain a new key."
                      << COL_RESET << "\n";
        } else {
            std::cout << COL_MAGENTA << "[-] Invalid key, try again."
                      << COL_RESET << "\n";
        }
    }

    // Highlighted "Waiting for rust to open..."
    std::cout << "\n" << COL_HL_BG << " Waiting for rust to open... " << COL_RESET << "\n";
    std::cout << COL_DIM
              << "  Start Rust now. Press Q at any time to cancel and exit."
              << COL_RESET << "\n";

    // Locate injector executable next to this loader
    wchar_t selfPath[MAX_PATH];
    GetModuleFileNameW(nullptr, selfPath, MAX_PATH);
    fs::path baseDir = fs::path(selfPath).parent_path();

    // Candidate injector / GUI locations (first match wins)
    std::vector<fs::path> candidates = {
        baseDir / "AetherGUI.exe",
        baseDir / "Aether-GUI.exe",
        baseDir / "RustInjector.exe",
        baseDir / "EnhancedProject.exe",
        baseDir / "bin" / "AetherGUI.exe",
        baseDir / "bin" / "RustInjector.exe",
        baseDir.parent_path() / "AetherGUI" / "bin" / "AetherGUI.exe",
        baseDir.parent_path() / "SourcePath" / "bin" / "x64" / "Release" / "RustInjector.exe",
    };

    int dots = 0;
    while (true) {
        DWORD pid = FindProcessIdByName(L"RustClient.exe");
        if (pid == 0) pid = FindProcessIdByName(L"Rust.exe");
        if (pid != 0) {
            std::cout << "\n" << COL_PURPLE << "[+] " << COL_RESET
                      << COL_BRIGHT << "Rust detected (PID " << pid << ")."
                      << COL_RESET << "\n";

            for (const auto& path : candidates) {
                if (fs::exists(path)) {
                    std::cout << COL_PURPLE << "[+] " << COL_RESET
                              << "Launching injector: " << path.filename().string() << "\n";
                    LaunchInjector(path);
                    break;
                }
            }

            std::cout << COL_PURPLE << "[+] " << COL_RESET
                      << COL_BRIGHT << "Aether loader complete. Enjoy."
                      << COL_RESET << "\n\n";
            std::cout << COL_DIM << "Press ENTER to exit..." << COL_RESET;
            std::cin.get();
            return 0;
        }

        // Allow user to cancel with Q / Esc instead of being stuck forever.
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 'q' || ch == 'Q' || ch == 27 /*ESC*/) {
                std::cout << "\n" << COL_MAGENTA << "[-] " << COL_RESET
                          << COL_BRIGHT << "Cancelled by user. Exiting."
                          << COL_RESET << "\n";
                return 0;
            }
        }

        // Animated dots
        std::cout << "\r" << COL_HL_BG << " Waiting for rust to open"
                  << std::string(dots + 1, '.')
                  << std::string(3 - dots, ' ') << " " << COL_RESET
                  << COL_DIM << "  (press Q to cancel)" << COL_RESET << "   ";
        std::cout.flush();
        dots = (dots + 1) % 4;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
