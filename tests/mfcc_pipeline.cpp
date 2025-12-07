#include "libvoicefeat/libvoicefeat.h"

#include <cmath>
#include <iostream>

#include "libvoicefeat/dsp/frame_extractor.h"
#include "libvoicefeat/dsp/window_functiion.h"
#include "libvoicefeat/utils/constants.h"

namespace
{
    constexpr float kPi = 3.14159265358979323846f;

    libvoicefeat::audio::AudioBuffer buildTestSine(int totalSamples, float freq, int sampleRate)
    {
        libvoicefeat::audio::AudioBuffer buffer;
        buffer.sampleRate = sampleRate;
        buffer.samples.resize(totalSamples);

        for (int n = 0; n < totalSamples; ++n)
        {
            buffer.samples[n] = std::sin(2.0f * kPi * freq * static_cast<float>(n) / sampleRate);
        }

        return buffer;
    }
}

int main()
{
    constexpr int sampleRate = 16000;
    constexpr int totalSamples = 3200;

    auto buffer = buildTestSine(totalSamples, 440.0f, sampleRate);

    libvoicefeat::CepstralConfig config;
    config.feature.sampleRate = sampleRate;
    config.feature.includeEnergy = true;
    config.delta.useDeltas = true;
    config.delta.useDeltaDeltas = true;
    config.preemphasis.usePreEmphasis = false;

    libvoicefeat::CepstralExtractor extractor(config);
    auto feature = extractor.extractFromAudioBuffer(buffer);
    auto mfcc = feature.getComputedMatrix();
    if (mfcc.empty())
    {
        std::cerr << "MFCC matrix is empty" << std::endl;
        return 1;
    }

    const std::size_t expectedWidth = static_cast<std::size_t>(config.feature.numCoeffs) *
                                   (1 + config.delta.useDeltas + config.delta.useDeltaDeltas);
    if (mfcc.front().size() != expectedWidth)
    {
        std::cerr << "Unexpected coefficient count" << std::endl;
        return 1;
    }

    // Frames are extracted with frameSize=400 and frameStep=160 by default
    libvoicefeat::dsp::FixedFrameExtractor frameExtractor(config.framing.frameSize, config.framing.frameStep);
    auto frames = frameExtractor.extract(buffer);
    if (mfcc.size() != frames.size())
    {
        std::cerr << "Unexpected number of frames (got " << mfcc.size() << ", expected " << frames.size() << ")" << std::endl;
        return 1;
    }

    // Verify log-energy was injected into c0 when requested
    auto windowedFrame = frames.front().data;
    libvoicefeat::dsp::WindowFunction window(config.framing.frameSize, config.framing.window);
    window.apply(windowedFrame);

    double frameEnergy = 0.0;
    for (double v : windowedFrame)
        frameEnergy += v * v;

    const double expectedLogEnergy = std::log(frameEnergy + libvoicefeat::constants::K_LOG_EPS);
    if (std::fabs(static_cast<double>(mfcc.front().front()) - expectedLogEnergy) > 1e-4)
    {
        std::cerr << "Log energy coefficient mismatch" << std::endl;
        return 1;
    }


    for (const auto& row : mfcc)
    {
        for (float coeff : row)
        {
            if (!std::isfinite(coeff))
            {
                std::cerr << "Non-finite MFCC value" << std::endl;
                return 1;
            }
        }
    }

    return 0;
}