#include "AdvancedObfuscation.h"

namespace AdvancedObfuscation {
    std::mt19937 MemoryObfuscator::rng;
    std::uniform_int_distribution<uint16_t> MemoryObfuscator::dist(0, 255);

    std::mt19937 ControlFlowObfuscator::rng;
    std::uniform_int_distribution<int> ControlFlowObfuscator::dist(1, 50);

    std::mt19937 TimingObfuscator::rng;
    std::uniform_int_distribution<int> TimingObfuscator::dist(10, 100);
}

