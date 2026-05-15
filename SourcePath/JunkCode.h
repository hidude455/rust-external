#pragma once
#include "Common.h"

namespace MIT {
    class JunkCodeGenerator {
    private:
        std::random_device rd;
        std::mt19937 gen;
        std::uniform_int_distribution<> intDist;
        std::uniform_int_distribution<> opDist;
        
        std::vector<std::string> junkOperations;
        std::vector<std::string> obfuscationStrings;

    public:
        JunkCodeGenerator();
        
        void GenerateJunkFunction();
        std::string ObfuscateString(const std::string& input);
        void RandomDelay();
        void DummyOperations();
        
        // Advanced obfuscation techniques
        void InsertRandomNops();
        void ScrambleMemory(uint8_t* data, size_t size);
        uint32_t GenerateRandomHash();
        
        // Entry point obfuscation
        void ObfuscateEntryPoint();
        void RestoreEntryPoint();
    };
}
