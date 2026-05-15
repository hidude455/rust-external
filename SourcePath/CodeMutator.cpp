/*
 * Runtime Code Mutation Implementation
 * Polymorphic code generation and anti-analysis
 * Author: Anonymous
 * Last Modified: 2026
 */

#include "CodeMutator.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace GameEnhance {
    
    CCodeMutator::CCodeMutator() 
        : m_mutationEnabled(true), m_mutationCounter(0), m_lastMutationTime(0), 
          m_mutationInterval(5000), m_rd(), m_gen(m_rd()), m_dist(0, 255), m_currentKeyIndex(0) {
        InitializeCriticalSection(&m_csMutation);
        
        // Initialize encryption keys
        for (int i = 0; i < 8; ++i) {
            m_encryptionKeys.push_back(GenerateRandomKey());
        }
        
        // Reserve space for code blocks
        m_codeBlocks.reserve(20);
        m_templates.reserve(10);
    }
    
    CCodeMutator::~CCodeMutator() {
        Shutdown();
        DeleteCriticalSection(&m_csMutation);
    }
    
    bool CCodeMutator::Initialize() {
        // Initialize code blocks from current module
        if (!InitializeCodeBlocks()) {
            return false;
        }
        
        // Load mutation templates
        LoadMutationTemplates();
        GenerateRandomTemplates();
        
        // Set initial mutation time
        m_lastMutationTime = GetTickCount64();
        
        return true;
    }
    
    void CCodeMutator::Shutdown() {
        EnterCriticalSection(&m_csMutation);
        
        // Restore all original code blocks
        for (auto& block : m_codeBlocks) {
            if (block.isProtected && !block.originalBytes.empty()) {
                MakeWritable(block.address, block.size);
                memcpy(block.address, block.originalBytes.data(), block.size);
                MakeExecutable(block.address, block.size);
            }
        }
        
        m_codeBlocks.clear();
        m_templates.clear();
        m_encryptionKeys.clear();
        
        LeaveCriticalSection(&m_csMutation);
    }
    
    bool CCodeMutator::IsInitialized() const {
        return !m_codeBlocks.empty();
    }
    
    bool CCodeMutator::InitializeCodeBlocks() {
        HMODULE hModule = GetModuleHandle(nullptr);
        if (!hModule) return false;
        
        // Get module information
        MODULEINFO modInfo;
        if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo))) {
            return false;
        }
        
        // Add main code section
        CodeBlock mainBlock = {};
        mainBlock.address = modInfo.lpBaseOfDll;
        mainBlock.size = modInfo.SizeOfImage;
        mainBlock.isExecutable = true;
        mainBlock.isProtected = false;
        mainBlock.blockName = "MainModule";
        
        // Store original bytes
        mainBlock.originalBytes.resize(mainBlock.size);
        memcpy(mainBlock.originalBytes.data(), mainBlock.address, mainBlock.size);
        mainBlock.mutatedBytes = mainBlock.originalBytes;
        
        m_codeBlocks.push_back(mainBlock);
        
        return true;
    }
    
    void CCodeMutator::LoadMutationTemplates() {
        // Common x86 junk instructions
        std::vector<uint8_t> nop = {0x90};
        std::vector<uint8_t> pushEax = {0x50};
        std::vector<uint8_t> popEax = {0x58};
        std::vector<uint8_t> pushEcx = {0x51};
        std::vector<uint8_t> popEcx = {0x59};
        std::vector<uint8_t> movEaxEax = {0x89, 0xC0};
        std::vector<uint8_t> xorEaxEax = {0x31, 0xC0};
        std::vector<uint8_t> addEsp4 = {0x83, 0xC4, 0x04};
        std::vector<uint8_t> subEsp4 = {0x83, 0xEC, 0x04};
        
        // Junk insertion templates
        MutationTemplate junk1 = {};
        junk1.pattern = {0x90}; // NOP
        junk1.replacement = {0x50, 0x58, 0x90}; // PUSH EAX, POP EAX, NOP
        junk1.type = MutationType::JUNK_INSERTION;
        junk1.priority = 1;
        junk1.isEnabled = true;
        m_templates.push_back(junk1);
        
        MutationTemplate junk2 = {};
        junk2.pattern = {0x89, 0xC0}; // MOV EAX, EAX
        junk2.replacement = {0x31, 0xC0}; // XOR EAX, EAX (equivalent)
        junk2.type = MutationType::INSTRUCTION_REORDER;
        junk2.priority = 2;
        junk2.isEnabled = true;
        m_templates.push_back(junk2);
        
        // Dead code insertion templates
        MutationTemplate deadCode1 = {};
        deadCode1.pattern = {0xC3}; // RET
        deadCode1.replacement = {0x31, 0xC0, 0xC3}; // XOR EAX, EAX, RET
        deadCode1.type = MutationType::DEAD_CODE_INSERTION;
        deadCode1.priority = 3;
        deadCode1.isEnabled = true;
        m_templates.push_back(deadCode1);
        
        // Control flow obfuscation
        MutationTemplate cfObf1 = {};
        cfObf1.pattern = {0x75, 0x0A}; // JNE +10
        cfObf1.replacement = {0x74, 0x02, 0xEB, 0x08}; // JE +2, JMP +8 (equivalent)
        cfObf1.type = MutationType::CONTROL_FLOW_OBFUSCATION;
        cfObf1.priority = 4;
        cfObf1.isEnabled = true;
        m_templates.push_back(cfObf1);
    }
    
    void CCodeMutator::GenerateRandomTemplates() {
        // Generate random mutation templates for variety
        for (int i = 0; i < 5; ++i) {
            MutationTemplate template_ = {};
            
            // Random junk instruction sequence
            template_.pattern = {0x90}; // Single NOP
            template_.replacement.clear();
            
            int junkCount = RandomGen::GetRandomInt(2, 5);
            for (int j = 0; j < junkCount; ++j) {
                uint8_t junk = GenerateRandomByte();
                // Ensure it's a valid instruction
                if (junk == 0x90 || junk == 0x50 || junk == 0x58 || 
                    junk == 0x51 || junk == 0x59 || junk == 0x31 || junk == 0x89) {
                    template_.replacement.push_back(junk);
                }
            }
            
            template_.type = MutationType::JUNK_INSERTION;
            template_.priority = RandomGen::GetRandomInt(5, 10);
            template_.isEnabled = true;
            
            m_templates.push_back(template_);
        }
    }
    
    bool CCodeMutator::AddCodeBlock(void* address, size_t size, const std::string& name) {
        if (!address || size == 0) return false;
        
        EnterCriticalSection(&m_csMutation);
        
        CodeBlock block = {};
        block.address = address;
        block.size = size;
        block.isExecutable = true;
        block.isProtected = false;
        block.blockName = name;
        
        // Store original bytes
        block.originalBytes.resize(size);
        memcpy(block.originalBytes.data(), address, size);
        block.mutatedBytes = block.originalBytes;
        
        m_codeBlocks.push_back(block);
        
        LeaveCriticalSection(&m_csMutation);
        return true;
    }
    
    bool CCodeMutator::RemoveCodeBlock(void* address) {
        EnterCriticalSection(&m_csMutation);
        
        auto it = std::find_if(m_codeBlocks.begin(), m_codeBlocks.end(),
                               [address](const CodeBlock& block) {
                                   return block.address == address;
                               });
        
        if (it != m_codeBlocks.end()) {
            // Restore original bytes before removing
            if (it->isProtected) {
                MakeWritable(it->address, it->size);
                memcpy(it->address, it->originalBytes.data(), it->size);
                MakeExecutable(it->address, it->size);
            }
            
            m_codeBlocks.erase(it);
            LeaveCriticalSection(&m_csMutation);
            return true;
        }
        
        LeaveCriticalSection(&m_csMutation);
        return false;
    }
    
    bool CCodeMutator::ProtectCodeBlock(void* address) {
        EnterCriticalSection(&m_csMutation);
        
        auto it = std::find_if(m_codeBlocks.begin(), m_codeBlocks.end(),
                               [address](const CodeBlock& block) {
                                   return block.address == address;
                               });
        
        if (it != m_codeBlocks.end()) {
            it->isProtected = true;
            LeaveCriticalSection(&m_csMutation);
            return true;
        }
        
        LeaveCriticalSection(&m_csMutation);
        return false;
    }
    
    bool CCodeMutator::UnprotectCodeBlock(void* address) {
        EnterCriticalSection(&m_csMutation);
        
        auto it = std::find_if(m_codeBlocks.begin(), m_codeBlocks.end(),
                               [address](const CodeBlock& block) {
                                   return block.address == address;
                               });
        
        if (it != m_codeBlocks.end()) {
            it->isProtected = false;
            LeaveCriticalSection(&m_csMutation);
            return true;
        }
        
        LeaveCriticalSection(&m_csMutation);
        return false;
    }
    
    bool CCodeMutator::MutateAllBlocks() {
        if (!m_mutationEnabled) return false;
        
        EnterCriticalSection(&m_csMutation);
        
        bool success = true;
        
        for (auto& block : m_codeBlocks) {
            if (block.isProtected) {
                // Select random mutation type
                MutationType types[] = {
                    MutationType::JUNK_INSERTION,
                    MutationType::INSTRUCTION_REORDER,
                    MutationType::DEAD_CODE_INSERTION,
                    MutationType::CONTROL_FLOW_OBFUSCATION
                };
                
                MutationType selectedType = types[RandomGen::GetRandomInt(0, 3)];
                
                if (!MutateBlock(block, selectedType)) {
                    success = false;
                }
            }
        }
        
        m_mutationCounter++;
        m_lastMutationTime = GetTickCount64();
        
        LeaveCriticalSection(&m_csMutation);
        return success;
    }
    
    bool CCodeMutator::MutateBlock(CodeBlock& block, MutationType type) {
        if (!block.isProtected || block.originalBytes.empty()) {
            return false;
        }
        
        // Make memory writable
        if (!MakeWritable(block.address, block.size)) {
            return false;
        }
        
        bool success = false;
        
        switch (type) {
            case MutationType::JUNK_INSERTION:
                success = InsertJunkCode(block);
                break;
                
            case MutationType::INSTRUCTION_REORDER:
                success = ReorderInstructions(block);
                break;
                
            case MutationType::DEAD_CODE_INSERTION:
                success = InsertDeadCode(block);
                break;
                
            case MutationType::CONTROL_FLOW_OBFUSCATION:
                success = ObfuscateControlFlow(block);
                break;
                
            case MutationType::REGISTER_SWAPPING:
                success = SwapRegisters(block);
                break;
                
            case MutationType::ENCRYPTION_ROTATION:
                success = RotateEncryption(block);
                break;
                
            default:
                success = false;
                break;
        }
        
        // Make memory executable again
        if (success) {
            MakeExecutable(block.address, block.size);
            LogMutation(block.blockName, type);
        }
        
        return success;
    }
    
    bool CCodeMutator::InsertJunkCode(CodeBlock& block) {
        if (block.size < 10) return false; // Need space for junk
        
        // Copy current mutated bytes
        std::vector<uint8_t> newBytes = block.mutatedBytes;
        
        // Insert random junk instructions at random positions
        int junkCount = RandomGen::GetRandomInt(1, 3);
        
        for (int i = 0; i < junkCount && newBytes.size() < block.size - 5; ++i) {
            size_t insertPos = RandomGen::GetRandomInt(0, newBytes.size() - 1);
            
            // Common junk instructions
            uint8_t junkInstructions[] = {0x90, 0x50, 0x58, 0x51, 0x59, 0x31, 0xC0};
            uint8_t selectedJunk = junkInstructions[RandomGen::GetRandomInt(0, 6)];
            
            newBytes.insert(newBytes.begin() + insertPos, selectedJunk);
        }
        
        // Ensure we don't exceed block size
        if (newBytes.size() > block.size) {
            newBytes.resize(block.size);
        }
        
        // Apply mutation
        memcpy(block.address, newBytes.data(), newBytes.size());
        block.mutatedBytes = newBytes;
        
        return true;
    }
    
    bool CCodeMutator::ReorderInstructions(CodeBlock& block) {
        // Simple instruction reordering - swap adjacent instructions
        if (block.size < 4) return false;
        
        std::vector<uint8_t> newBytes = block.mutatedBytes;
        
        // Find swap positions
        size_t pos1 = RandomGen::GetRandomInt(0, block.size - 4);
        size_t pos2 = pos1 + 2;
        
        if (pos2 >= block.size) {
            pos2 = block.size - 2;
        }
        
        // Swap 2-byte sequences
        if (pos1 + 2 <= block.size && pos2 + 2 <= block.size) {
            std::swap(newBytes[pos1], newBytes[pos2]);
            std::swap(newBytes[pos1 + 1], newBytes[pos2 + 1]);
            
            // Apply mutation
            memcpy(block.address, newBytes.data(), newBytes.size());
            block.mutatedBytes = newBytes;
            
            return true;
        }
        
        return false;
    }
    
    bool CCodeMutator::InsertDeadCode(CodeBlock& block) {
        if (block.size < 8) return false;
        
        std::vector<uint8_t> newBytes = block.mutatedBytes;
        
        // Insert dead code that doesn't affect execution
        size_t insertPos = RandomGen::GetRandomInt(0, block.size - 6);
        
        // Dead code: XOR EAX, EAX; MOV EAX, EAX (net effect: EAX = 0, then EAX = 0)
        std::vector<uint8_t> deadCode = {0x31, 0xC0, 0x89, 0xC0};
        
        newBytes.insert(newBytes.begin() + insertPos, deadCode.begin(), deadCode.end());
        
        // Ensure we don't exceed block size
        if (newBytes.size() > block.size) {
            newBytes.resize(block.size);
        }
        
        // Apply mutation
        memcpy(block.address, newBytes.data(), newBytes.size());
        block.mutatedBytes = newBytes;
        
        return true;
    }
    
    bool CCodeMutator::ObfuscateControlFlow(CodeBlock& block) {
        if (block.size < 6) return false;
        
        std::vector<uint8_t> newBytes = block.mutatedBytes;
        
        // Find a conditional jump to obfuscate
        for (size_t i = 0; i < newBytes.size() - 2; ++i) {
            if (newBytes[i] == 0x75 || newBytes[i] == 0x74) { // JNE or JE
                // Replace with equivalent but more complex sequence
                if (i + 2 < newBytes.size()) {
                    // Original: JNE offset
                    // New: JE +2, JMP offset+2, (original target)
                    newBytes[i] = (newBytes[i] == 0x75) ? 0x74 : 0x75; // Invert condition
                    newBytes.insert(newBytes.begin() + i + 1, 0x02); // Short jump
                    newBytes.insert(newBytes.begin() + i + 2, 0xEB); // JMP
                    
                    break;
                }
            }
        }
        
        // Ensure we don't exceed block size
        if (newBytes.size() > block.size) {
            newBytes.resize(block.size);
        }
        
        // Apply mutation
        memcpy(block.address, newBytes.data(), newBytes.size());
        block.mutatedBytes = newBytes;
        
        return true;
    }
    
    bool CCodeMutator::SwapRegisters(CodeBlock& block) {
        // Register swapping is complex and requires disassembly
        // For simplicity, we'll just add some register operations
        if (block.size < 4) return false;
        
        std::vector<uint8_t> newBytes = block.mutatedBytes;
        
        // Insert register swap operations
        size_t insertPos = RandomGen::GetRandomInt(0, block.size - 4);
        
        // PUSH EAX; PUSH EBX; POP EBX; POP EAX (swap EAX and EBX)
        std::vector<uint8_t> swapCode = {0x50, 0x53, 0x5B, 0x58};
        
        newBytes.insert(newBytes.begin() + insertPos, swapCode.begin(), swapCode.end());
        
        // Ensure we don't exceed block size
        if (newBytes.size() > block.size) {
            newBytes.resize(block.size);
        }
        
        // Apply mutation
        memcpy(block.address, newBytes.data(), newBytes.size());
        block.mutatedBytes = newBytes;
        
        return true;
    }
    
    bool CCodeMutator::RotateEncryption(CodeBlock& block) {
        // Simple XOR encryption rotation
        if (block.originalBytes.empty()) return false;
        
        uint32_t currentKey = m_encryptionKeys[m_currentKeyIndex];
        
        for (size_t i = 0; i < block.size && i < block.mutatedBytes.size(); ++i) {
            block.mutatedBytes[i] = block.originalBytes[i] ^ (currentKey & 0xFF);
            currentKey = (currentKey << 1) | (currentKey >> 31); // Rotate key
        }
        
        // Apply mutation
        memcpy(block.address, block.mutatedBytes.data(), block.size);
        
        // Move to next key
        m_currentKeyIndex = (m_currentKeyIndex + 1) % m_encryptionKeys.size();
        
        return true;
    }
    
    bool CCodeMutator::MakeWritable(void* address, size_t size) {
        DWORD oldProtect;
        return VirtualProtect(address, size, PAGE_READWRITE, &oldProtect);
    }
    
    bool CCodeMutator::MakeExecutable(void* address, size_t size) {
        DWORD oldProtect;
        return VirtualProtect(address, size, PAGE_EXECUTE_READ, &oldProtect);
    }
    
    uint8_t CCodeMutator::GenerateRandomByte() {
        return static_cast<uint8_t>(m_dist(m_gen));
    }
    
    uint32_t CCodeMutator::GenerateRandomKey() {
        std::uniform_int_distribution<uint32_t> keyDist(0x10000000, 0xFFFFFFFF);
        return keyDist(m_gen);
    }
    
    void CCodeMutator::LogMutation(const std::string& blockName, MutationType type) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        
        char timestamp[64];
        sprintf_s(timestamp, "[%04d-%02d-%02d %02d:%02d:%02d] ", 
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        
        std::string typeStr;
        switch (type) {
            case MutationType::JUNK_INSERTION: typeStr = "JUNK_INSERTION"; break;
            case MutationType::INSTRUCTION_REORDER: typeStr = "INSTRUCTION_REORDER"; break;
            case MutationType::REGISTER_SWAPPING: typeStr = "REGISTER_SWAPPING"; break;
            case MutationType::CONTROL_FLOW_OBFUSCATION: typeStr = "CONTROL_FLOW_OBFUSCATION"; break;
            case MutationType::ENCRYPTION_ROTATION: typeStr = "ENCRYPTION_ROTATION"; break;
            case MutationType::DEAD_CODE_INSERTION: typeStr = "DEAD_CODE_INSERTION"; break;
        }
        
        std::string logEntry = timestamp + "Mutated block '" + blockName + "' with " + typeStr;
        
        // Write to log file
        std::ofstream logFile("mutation.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
            logFile.close();
        }
    }
    
    void CCodeMutator::EnableMutation(bool enable) {
        m_mutationEnabled = enable;
    }
    
    bool CCodeMutator::IsMutationEnabled() const {
        return m_mutationEnabled;
    }
    
    void CCodeMutator::SetMutationInterval(uint32_t intervalMs) {
        m_mutationInterval = intervalMs;
    }
    
    uint32_t CCodeMutator::GetMutationInterval() const {
        return m_mutationInterval;
    }
    
    uint32_t CCodeMutator::GetMutationCount() const {
        return m_mutationCounter;
    }
    
    uint64_t CCodeMutator::GetLastMutationTime() const {
        return m_lastMutationTime;
    }
    
    size_t CCodeMutator::GetProtectedBlockCount() const {
        size_t count = 0;
        for (const auto& block : m_codeBlocks) {
            if (block.isProtected) {
                count++;
            }
        }
        return count;
    }
    
    std::vector<std::string> CCodeMutator::GetBlockNames() const {
        std::vector<std::string> names;
        for (const auto& block : m_codeBlocks) {
            names.push_back(block.blockName);
        }
        return names;
    }
    
    void CCodeMutator::PerformPolymorphicMutation() {
        // Advanced polymorphic mutation with multiple techniques
        if (!m_mutationEnabled) return;
        
        EnterCriticalSection(&m_csMutation);
        
        for (auto& block : m_codeBlocks) {
            if (block.isProtected) {
                // Apply multiple mutation types
                MutateBlock(block, MutationType::JUNK_INSERTION);
                MutateBlock(block, MutationType::ENCRYPTION_ROTATION);
                MutateBlock(block, MutationType::CONTROL_FLOW_OBFUSCATION);
            }
        }
        
        // Rotate encryption keys
        RotateEncryptionKeys();
        
        m_mutationCounter++;
        m_lastMutationTime = GetTickCount64();
        
        LeaveCriticalSection(&m_csMutation);
    }
    
    void CCodeMutator::RotateEncryptionKeys() {
        // Rotate all encryption keys
        for (auto& key : m_encryptionKeys) {
            key = (key << 1) | (key >> 31); // Rotate left
            key ^= GenerateRandomKey(); // XOR with random value
        }
    }
    
    void CCodeMutator::AddEncryptionKey(uint32_t key) {
        m_encryptionKeys.push_back(key);
    }
    
    void CCodeMutator::SetEncryptionKeys(const std::vector<uint32_t>& keys) {
        m_encryptionKeys = keys;
        m_currentKeyIndex = 0;
    }
    
    void CCodeMutator::EnableStealthMode() {
        // Enable additional stealth measures
        SetMutationInterval(1000); // More frequent mutations
    }
    
    void CCodeMutator::DisableStealthMode() {
        // Disable stealth mode
        SetMutationInterval(5000); // Normal mutation interval
    }
    
    bool CCodeMutator::IsStealthModeEnabled() const {
        return m_mutationInterval < 2000;
    }
    
} // namespace GameEnhance
