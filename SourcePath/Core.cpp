#include "Core.h"

namespace GameEnhance {
    
    // Static member definitions for RandomGen class
    std::random_device RandomGen::rd;
    std::mt19937 RandomGen::gen(RandomGen::rd());
    std::uniform_int_distribution<> RandomGen::dis(0, 1000);
    
}
