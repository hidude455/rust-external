#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

namespace AdvancedObfuscation {

    // String encryption/decryption
    class StringObfuscator {
    private:
        static constexpr uint8_t XOR_KEY = 0x5A;
        
    public:
        static std::string Decrypt(const char* encrypted, size_t len) {
            std::string result;
            result.reserve(len);
            for (size_t i = 0; i < len; i++) {
                result += encrypted[i] ^ XOR_KEY;
            }
            return result;
        }
        
        static std::vector<uint8_t> Encrypt(const std::string& data) {
            std::vector<uint8_t> result;
            result.reserve(data.length());
            for (char c : data) {
                result.push_back(c ^ XOR_KEY);
            }
            return result;
        }
    };

    // API obfuscation
    class APIObfuscator {
    private:
        static HMODULE GetModuleHashed(uint32_t hash) {
            HMODULE hMods[256];
            DWORD cbNeeded;
            
            if (EnumProcessModules(GetCurrentProcess(), hMods, sizeof(hMods), &cbNeeded)) {
                for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                    char modName[MAX_PATH];
                    if (GetModuleFileNameExA(GetCurrentProcess(), hMods[i], modName, sizeof(modName))) {
                        std::string name(modName);
                        if (ComputeHash(name) == hash) {
                            return hMods[i];
                        }
                    }
                }
            }
            return nullptr;
        }
        
        static uint32_t ComputeHash(const std::string& str) {
            uint32_t hash = 0x811c9dc5;
            for (char c : str) {
                hash ^= c;
                hash *= 0x01000193;
            }
            return hash;
        }
        
    public:
        static FARPROC GetProcAddressHashed(HMODULE hModule, uint32_t funcHash) {
            if (!hModule) return nullptr;
            
            auto* dosHeader = (PIMAGE_DOS_HEADER)hModule;
            auto* ntHeaders = (PIMAGE_NT_HEADERS)((uint8_t*)hModule + dosHeader->e_lfanew);
            auto* exportDir = (PIMAGE_EXPORT_DIRECTORY)((uint8_t*)hModule + 
                ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
            
            auto* names = (DWORD*)((uint8_t*)hModule + exportDir->AddressOfNames);
            auto* ordinals = (WORD*)((uint8_t*)hModule + exportDir->AddressOfNameOrdinals);
            auto* functions = (DWORD*)((uint8_t*)hModule + exportDir->AddressOfFunctions);
            
            for (DWORD i = 0; i < exportDir->NumberOfNames; i++) {
                char* funcName = (char*)((uint8_t*)hModule + names[i]);
                if (ComputeHash(funcName) == funcHash) {
                    return (FARPROC)((uint8_t*)hModule + functions[ordinals[i]]);
                }
            }
            return nullptr;
        }
        
        static HMODULE LoadLibraryHashed(const std::string& libName) {
            return GetModuleHashed(ComputeHash(libName));
        }
    };

    // Memory obfuscation
    class MemoryObfuscator {
    private:
        static std::mt19937 rng;
        static std::uniform_int_distribution<uint8_t> dist;
        
    public:
        static void ScrambleMemory(void* ptr, size_t size) {
            uint8_t* bytes = (uint8_t*)ptr;
            for (size_t i = 0; i < size; i++) {
                bytes[i] ^= dist(rng);
            }
        }
        
        static void UnscrambleMemory(void* ptr, size_t size) {
            uint8_t* bytes = (uint8_t*)ptr;
            for (size_t i = 0; i < size; i++) {
                bytes[i] ^= dist(rng);
            }
        }
        
        static void FillJunk(void* ptr, size_t size) {
            uint8_t* bytes = (uint8_t*)ptr;
            for (size_t i = 0; i < size; i++) {
                bytes[i] = dist(rng);
            }
        }
    };

    std::mt19937 MemoryObfuscator::rng(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<uint8_t> MemoryObfuscator::dist(0, 255);

    // Control flow obfuscation
    class ControlFlowObfuscator {
    private:
        static std::mt19937 rng;
        static std::uniform_int_distribution<int> dist;
        
    public:
        static void RandomDelay() {
            int delay = dist(rng);
            Sleep(delay);
        }
        
        static bool RandomBranch() {
            return dist(rng) % 2 == 0;
        }
        
        static void JunkCode() {
            volatile int x = 0;
            for (int i = 0; i < 10; i++) {
                x += i;
                x *= 2;
                x ^= 0x5A5A5A5A;
            }
        }
        
        static void ObfuscatedLoop(int iterations) {
            for (int i = 0; i < iterations; ) {
                JunkCode();
                if (RandomBranch()) {
                    i++;
                } else {
                    RandomDelay();
                }
            }
        }
    };

    std::mt19937 ControlFlowObfuscator::rng(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> ControlFlowObfuscator::dist(1, 50);

    // Thread obfuscation
    class ThreadObfuscator {
    public:
        static DWORD WINAPI JunkThread(LPVOID param) {
            volatile int counter = 0;
            while (true) {
                counter++;
                Sleep(100);
            }
            return 0;
        }
        
        static HANDLE CreateJunkThread() {
            return CreateThread(nullptr, 0, JunkThread, nullptr, 0, nullptr);
        }
        
        static void HideThread(HANDLE hThread) {
            // Set thread to hidden state
            // This is a simplified version
        }
    };

    // Timing obfuscation
    class TimingObfuscator {
    private:
        static std::mt19937 rng;
        static std::uniform_int_distribution<int> dist;
        
    public:
        static void RandomSleep(int minMs = 10, int maxMs = 100) {
            Sleep(dist(rng));
        }
        
        static void ObfuscateTiming() {
            LARGE_INTEGER freq, start, end;
            QueryPerformanceFrequency(&freq);
            QueryPerformanceCounter(&start);
            
            // Do some work
            volatile int x = 0;
            for (int i = 0; i < 1000; i++) {
                x += i;
            }
            
            QueryPerformanceCounter(&end);
            
            // Add random delay based on timing
            double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
            Sleep((int)(elapsed * 1000) + dist(rng));
        }
    };

    std::mt19937 TimingObfuscator::rng(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> TimingObfuscator::dist(10, 100);

    // Registry obfuscation
    class RegistryObfuscator {
    public:
        static std::string ObfuscateKey(const std::string& key) {
            std::string result;
            for (char c : key) {
                result += std::to_string((int)c) + "-";
            }
            return result;
        }
        
        static std::string DeobfuscateKey(const std::string& obfuscated) {
            std::string result;
            std::string temp;
            for (char c : obfuscated) {
                if (c == '-') {
                    if (!temp.empty()) {
                        result += (char)std::stoi(temp);
                        temp.clear();
                    }
                } else {
                    temp += c;
                }
            }
            return result;
        }
        
        static HKEY GetHiddenRootKey() {
            // Return an obfuscated registry key path
            return HKEY_CURRENT_USER;
        }
    };

    // File obfuscation
    class FileObfuscator {
    public:
        static std::string ObfuscateFileName(const std::string& filename) {
            std::string result;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 15);
            
            const char hex[] = "0123456789ABCDEF";
            for (int i = 0; i < 16; i++) {
                result += hex[dis(gen)];
            }
            result += ".dat";
            return result;
        }
        
        static bool HideFile(const std::string& path) {
            return SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
        }
        
        static bool MakeFileReadOnly(const std::string& path) {
            return SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_READONLY);
        }
    };

    // Network obfuscation
    class NetworkObfuscator {
    public:
        static std::string ObfuscateIP(const std::string& ip) {
            std::string result;
            for (char c : ip) {
                if (c == '.') {
                    result += "[.]";
                } else {
                    result += c;
                }
            }
            return result;
        }
        
        static std::string ObfuscateURL(const std::string& url) {
            std::string result;
            for (char c : url) {
                if (c == '/') {
                    result += "[/]";
                } else if (c == ':') {
                    result += "[:]";
                } else {
                    result += c;
                }
            }
            return result;
        }
    };

}
