#pragma once
#include "libvoicefeat/audio/audio_buffer.h"

namespace libvoicefeat::dsp
{
    class Resampler
    {
    public:
        [[nodiscard]] static audio::AudioBuffer resampleTo(const audio::AudioBuffer& in, int targetSampleRate);
    };
}
