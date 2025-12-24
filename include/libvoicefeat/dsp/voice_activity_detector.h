#pragma once

#include "libvoicefeat/types.h"
#include "libvoicefeat/audio/audio_buffer.h"

#include <vector>

#include "libvoicefeat/config.h"

namespace libvoicefeat::dsp
{
    class VoiceActivityDetector
    {
    public:
        VoiceActivityDetector(const CepstralConfig& config);
        ~VoiceActivityDetector() = default;
        [[nodiscard]] VADFlags detect(const audio::AudioBuffer& audio) const;

    private:
        [[nodiscard]] static std::vector<float>
        computeFrameEnergies(const std::vector<float>& samples,
                             int frameSize,
                             int hopSize);

        [[nodiscard]] float estimateNoiseFloor(const std::vector<float>& energies) const;
        [[nodiscard]] static VADFlags classifyFrames(const std::vector<float>& energies, float threshold);
        void smoothFlags(std::vector<bool>& flags) const;

        void validateParams();

        float _energyThresholdDb{};
        float _noiseFloorPercentile{};
        int _minSpeechFrames{};

        int _frameSize{}, _frameStep{};
    };
}
