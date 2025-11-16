#include "libmfcc/libmfcc.h"

#include "libmfcc/audio/mp3_audio_reader.h"
#include "libmfcc/audio/wav_audio_reader.h"
#include "libmfcc/dsp/frame_extractor.h"
#include "libmfcc/dsp/window_functiion.h"
#include "libmfcc/dsp/fft_transformer.h"
#include "libmfcc/features/delta.h"
#include "libmfcc/features/mfcc.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

#include "libmfcc/utils/path.h"

namespace
{
    using namespace libmfcc;
    using namespace libmfcc::utils;

    libmfcc::audio::AudioBuffer load_audio(const std::filesystem::path& path)
    {
        const auto extStr = path.extension().string();
        std::string ext;
        ext.resize(extStr.size());
        std::transform(extStr.begin(), extStr.end(), ext.begin(), [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });

        if (ext == ".wav")
        {
            libmfcc::audio::WavAudioReader reader;
            return reader.load(path);
        }
        if (ext == ".mp3")
        {
            libmfcc::audio::Mp3AudioReader reader;
            return reader.load(path);
        }

        throw std::invalid_argument("Unsupported audio format: " + path.string());
    }

    void apply_pre_emphasis(std::vector<float>& samples, float coeff)
    {
        if (samples.empty())
        {
            return;
        }

        std::vector<float> emphasized(samples.size());
        emphasized[0] = samples[0];
        for (std::size_t i = 1; i < samples.size(); ++i)
        {
            emphasized[i] = samples[i] - coeff * samples[i - 1];
        }
        samples.swap(emphasized);
    }

    features::MfccOptions build_options(int sampleRate, const MfccConfig& config)
    {
        features::MfccOptions opts;
        opts.sampleRate = sampleRate;
        opts.numCoeffs = config.numCoeffs;
        opts.numFilters = config.numFilters;
        opts.minFreq = config.minFreq;
        opts.maxFreq = config.maxFreq > 0.0f ? config.maxFreq : static_cast<float>(sampleRate) / 2.0f;
        opts.includeEnergy = config.includeEnergy;
        opts.melScale = config.melScale;
        return opts;
    }
}

namespace libmfcc
{
    MfccMatrix compute_file_mfcc(const std::filesystem::path& path, const MfccConfig& config)
    {
        auto audio = load_audio(path);
        return compute_buffer_mfcc(audio, config);
    }

    MfccMatrix compute_buffer_mfcc(const audio::AudioBuffer& audio, const MfccConfig& config)
    {
        if (config.frameSize <= 0 || config.frameStep <= 0)
            throw std::invalid_argument("Frame size and step must be positive");

        int sampleRate = audio.sampleRate > 0 ? audio.sampleRate : config.sampleRate;
        if (sampleRate <= 0)
            throw std::invalid_argument("Sample rate must be positive");

        audio::AudioBuffer working = audio;
        working.sampleRate = sampleRate;

        if (config.usePreEmphasis)
            apply_pre_emphasis(working.samples, config.preEmphasisCoeff);

        dsp::FixedFrameExtractor extractor(config.frameSize, config.frameStep);
        auto frames = extractor.extract(working);
        if (frames.empty())
            return {};

        dsp::HanningWindow window(config.frameSize);
        for (auto& frame : frames)
            window.apply(frame.data);

        dsp::FFTTransformer transformer;
        const auto options = build_options(sampleRate, config);
        auto base = features::computeMFCC(frames, transformer, options);
        return features::appendDeltas(base, config.useDeltas, config.useDeltaDeltas);
    }
}
