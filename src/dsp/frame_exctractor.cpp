#include "libvoicefeat/dsp/frame_extractor.h"

#include <stdexcept>

using namespace libvoicefeat::dsp;

FixedFrameExtractor::FixedFrameExtractor(int windowSize, int hopSize)
    : _windowSize(windowSize), _hopSize(hopSize)
{
    if (windowSize <= 0)
        throw std::invalid_argument("windowSize must be positive");
    if (hopSize <= 0)
        throw std::invalid_argument("hopSize must be positive");
}

std::vector<Frame> FixedFrameExtractor::extract(const audio::AudioBuffer& audio)
{
    std::vector<Frame> frames;
    const auto& x = audio.samples;
    const auto N = x.size();
    const auto windowSize = static_cast<std::size_t>(_windowSize);
    const auto hopSize = static_cast<std::size_t>(_hopSize);

    for (std::size_t i = 0; i + windowSize <= N; i += hopSize)
    {
        Frame f;
        f.data.resize(windowSize);
        for (std::size_t j = 0; j < windowSize; ++j)
            f.data[j] = x[i + j];
        frames.push_back(f);
    }

    return frames;
}
