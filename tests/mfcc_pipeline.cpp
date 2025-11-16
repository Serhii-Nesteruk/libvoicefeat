#include "libvoicefeat/libvoicefeat.h"

#include <cmath>
#include <iostream>

namespace {
    constexpr float kPi = 3.14159265358979323846f;
}

int main()
{
    libvoicefeat::audio::AudioBuffer buffer;
    buffer.sampleRate = 16000;

    const int totalSamples = 3200;
    buffer.samples.resize(totalSamples);
    const float freq = 440.0f;
    for (int n = 0; n < totalSamples; ++n)
    {
        buffer.samples[n] = std::sin(2.0f * kPi * freq * static_cast<float>(n) / buffer.sampleRate);
    }

    libvoicefeat::CepstralConfig config;
    config.useDeltas = true;
    config.useDeltaDeltas = true;

    auto mfcc = libvoicefeat::compute_buffer_mfcc(buffer, config);
    if (mfcc.empty())
    {
        std::cerr << "MFCC matrix is empty" << std::endl;
        return 1;
    }

    const std::size_t expectedWidth = static_cast<std::size_t>(config.numCoeffs) * (1 + config.useDeltas + config.useDeltaDeltas);
    if (mfcc.front().size() != expectedWidth)
    {
        std::cerr << "Unexpected coefficient count" << std::endl;
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