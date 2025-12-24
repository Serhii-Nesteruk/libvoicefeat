#pragma once
#include <cstdint>
#include <vector>

namespace libvoicefeat
{
    enum class VADState : uint8_t
    {
        NonSpeech = 0,
        Speech = 1
    };

    using VADFlags = std::vector<VADState>; // non-speech = 0, speech = 1
}
