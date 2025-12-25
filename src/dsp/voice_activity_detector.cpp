#include "libvoicefeat/dsp/voice_activity_detector.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace libvoicefeat::dsp;
using namespace libvoicefeat;

VoiceActivityDetector::VoiceActivityDetector(const CepstralConfig& config)
    : _energyThresholdDb(config.vad.energyThresholdDb),
      _noiseFloorPercentile(config.vad.noiseFloorPercentile),
      _minSpeechFrames(config.vad.minSpeechFrames), _frameSize(config.framing.frameSize),
      _frameStep(config.framing.frameStep)

{
    validateParams();
};

VADFlags VoiceActivityDetector::detect(const audio::AudioBuffer& audio) const
{

    if (_frameSize <= 0 || _frameStep <= 0)
        throw std::invalid_argument("frameSize and frameStep must be positive");

    const auto& samples = audio.samples;
    const std::size_t totalSamples = samples.size();
    if (totalSamples < static_cast<std::size_t>(_frameSize))
        return {};

    auto energies = computeFrameEnergies(samples, _frameSize, _frameStep);

    const float noiseFloorDb = estimateNoiseFloor(energies);
    const float threshold = noiseFloorDb + _energyThresholdDb;

    auto flags = classifyFrames(energies, threshold);

    if (_minSpeechFrames > 1)
        smoothFlags(flags);

    return flags;
}


std::vector<float> VoiceActivityDetector::computeFrameEnergies(const std::vector<float>& samples,
                                                               int frameSize,
                                                               int hopSize)
{
    std::vector<float> energies;
    const std::size_t totalSamples = samples.size();
    energies.reserve((totalSamples - frameSize) / hopSize + 1);

    for (std::size_t start = 0; start + frameSize <= totalSamples; start += hopSize)
    {
        double sumSq = 0.0;
        for (int j = 0; j < frameSize; ++j)
        {
            const float v = samples[start + static_cast<std::size_t>(j)];
            sumSq += static_cast<double>(v) * static_cast<double>(v);
        }
        const double meanSq = sumSq / static_cast<double>(frameSize);
        const double energyDb = 10.0 * std::log10(meanSq + 1e-12);
        energies.push_back(static_cast<float>(energyDb));
    }
    return energies;
}

float VoiceActivityDetector::estimateNoiseFloor(const std::vector<float>& energies) const
{
    if (energies.empty())
        return 0.0f;

    std::vector<float> sorted = energies;
    std::sort(sorted.begin(), sorted.end());

    const auto k = static_cast<std::size_t>(
        std::floor(static_cast<double>(sorted.size()) * _noiseFloorPercentile));
    const std::size_t count = std::max<std::size_t>(1, k);
    float sum = 0.0f;
    for (std::size_t i = 0; i < count; ++i)
        sum += sorted[i];
    return sum / static_cast<float>(count);
}

VADFlags VoiceActivityDetector::classifyFrames(const std::vector<float>& energies,
                                                        float threshold)
{
    VADFlags flags;
    flags.reserve(energies.size());
    for (float e : energies)
    {
        VADState f = e > threshold ? VADState::Speech : VADState::NonSpeech;
        flags.push_back(f);
    }
    return flags;
}

void VoiceActivityDetector::smoothFlags(VADFlags& flags) const
{
    const std::size_t n = flags.size();
    std::size_t runStart = 0;
    while (runStart < n)
    {
        while (runStart < n && !static_cast<bool>(flags[runStart]))
            ++runStart;
        if (runStart >= n)
            break;
        std::size_t runEnd = runStart;
        while (runEnd < n && static_cast<bool>(flags[runEnd]))
            ++runEnd;
        const std::size_t runLength = runEnd - runStart;
        if (runLength < static_cast<std::size_t>(_minSpeechFrames))
        {
            for (std::size_t i = runStart; i < runEnd; ++i)
                flags[i] = VADState::NonSpeech;
        }
        runStart = runEnd;
    }
}

void VoiceActivityDetector::validateParams()
{
    if (_noiseFloorPercentile < 0.0f)
        _noiseFloorPercentile = 0.0f;
    if (_noiseFloorPercentile > 0.5f)
        _noiseFloorPercentile = 0.5f;
    if (_minSpeechFrames < 1)
        _minSpeechFrames = 1;

    if (_frameSize <= 0 || _frameStep <= 0)
        throw std::invalid_argument("frameSize and frameStep must be positive");
}
