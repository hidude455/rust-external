#include "JunkCode.h"
#include <sstream>

namespace MIT {
    JunkCodeGenerator::JunkCodeGenerator() : gen(rd()), intDist(0, 1000000), opDist(0, 9) {
        // Initialize junk operations
        junkOperations = {
            "xor eax, eax",
            "add ecx, edx",
            "sub ebx, esi",
            "imul eax, ebx, 2",
            "mov edx, [eax+4]",
            "push ebp",
            "pop ebp",
            "nop",
            "lea ecx, [eax+ebx*2]",
            "test edx, edx"
        };

        obfuscationStrings = {
            "JUNK_OBFUSCATION_1",
            "MIT_METHOD_JUNK_2", 
            "STEALTH_CODE_3",
            "ANTI_REVERSE_4",
            "PROTECTION_LAYER_5"
        };
    }

    void JunkCodeGenerator::GenerateJunkFunction() {
        volatile int result = 0;
        int iterations = intDist(gen) % 50 + 10;
        
        for (int i = 0; i < iterations; ++i) {
            switch (opDist(gen)) {
                case 0:
                    result ^= intDist(gen);
                    break;
                case 1:
                    result += intDist(gen);
                    break;
                case 2:
                    result *= intDist(gen) % 100 + 1;
                    break;
                case 3:
                    result = (result << 1) ^ (result >> 1);
                    break;
                case 4:
                    result = ~result;
                    break;
                case 5:
                    result &= intDist(gen);
                    break;
                case 6:
                    result |= intDist(gen);
                    break;
                case 7:
                    result = result ^ (result << 16);
                    break;
                case 8:
                    result = result + (result << 3);
                    break;
                case 9:
                    result = result - (result >> 2);
                    break;
            }
        }
        
        // Prevent compiler optimization
        volatile int dummy = result;
    }

    std::string JunkCodeGenerator::ObfuscateString(const std::string& input) {
        std::string obfuscated = input;
        int key = intDist(gen) % 255 + 1;
        
        for (char& c : obfuscated) {
            c ^= key;
        }
        
        return obfuscated;
    }

    void JunkCodeGenerator::RandomDelay() {
        int delayMicros = intDist(gen) % 1000 + 100;
        std::this_thread::sleep_for(std::chrono::microseconds(delayMicros));
    }

    void JunkCodeGenerator::DummyOperations() {
        volatile int x = intDist(gen);
        volatile int y = intDist(gen);
        volatile int z = 0;
        
        for (int i = 0; i < (intDist(gen) % 20 + 5); ++i) {
            z = x + y;
            z = x ^ y;
            z = x * y;
            z = x - y;
            z = x & y;
            z = x | y;
            z = ~x;
            z = x << 1;
            z = x >> 1;
        }
    }

    void JunkCodeGenerator::InsertRandomNops() {
        // This would insert NOP instructions in assembly
        // For C++, we simulate with dummy operations
        GenerateJunkFunction();
    }

    void JunkCodeGenerator::ScrambleMemory(uint8_t* data, size_t size) {
        if (!data || size == 0) return;
        
        int key = intDist(gen) % 255 + 1;
        for (size_t i = 0; i < size; ++i) {
            data[i] ^= key;
        }
    }

    uint32_t JunkCodeGenerator::GenerateRandomHash() {
        uint32_t hash = intDist(gen);
        
        // Apply multiple hashing rounds
        for (int i = 0; i < 32; ++i) {
            hash = ((hash << 5) + hash) + intDist(gen);
            hash ^= (hash >> 16);
            hash *= 0x85ebca6b;
            hash ^= (hash >> 13);
            hash *= 0xc2b2ae35;
            hash ^= (hash >> 16);
        }
        
        return hash;
    }

    void JunkCodeGenerator::ObfuscateEntryPoint() {
        // Generate junk code at entry point
        GenerateJunkFunction();
        RandomDelay();
        DummyOperations();
        
        // Add some random string operations
        std::string junkStr = obfuscationStrings[intDist(gen) % obfuscationStrings.size()];
        std::string obfuscated = ObfuscateString(junkStr);
        
        // Prevent optimization
        volatile size_t len = obfuscated.length();
    }

    void JunkCodeGenerator::RestoreEntryPoint() {
        // This would restore the original entry point
        // For now, just generate more junk to confuse analysis
        GenerateJunkFunction();
    }
}
