#include "libvoicefeat/libvoicefeat.h"

#include "libvoicefeat/audio/mp3_audio_reader.h"
#include "libvoicefeat/audio/wav_audio_reader.h"
#include "libvoicefeat/dsp/frame_extractor.h"
#include "libvoicefeat/dsp/window_functiion.h"
#include "libvoicefeat/dsp/fft_transformer.h"
#include "libvoicefeat/features/delta.h"
#include "libvoicefeat/features/mfcc.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

#include "libvoicefeat/features/feature_builder.h"
#include "libvoicefeat/utils/path.h"

namespace
{
    using namespace libvoicefeat;
    using namespace libvoicefeat::utils;

    libvoicefeat::audio::AudioBuffer loadAudio(const std::filesystem::path& path)
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
            libvoicefeat::audio::WavAudioReader reader;
            return reader.load(path);
        }
        if (ext == ".mp3")
        {
            libvoicefeat::audio::Mp3AudioReader reader;
            return reader.load(path);
        }

        throw std::invalid_argument("Unsupported audio format: " + path.string());
    }

    void applyPreEmphasis(std::vector<float>& samples, float coeff)
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

    FeatureOptions buildOptions(int sampleRate, const CepstralConfig& config)
    {
        FeatureOptions opts;
        opts.sampleRate = sampleRate;
        opts.numCoeffs = config.feature.numCoeffs;
        opts.numFilters = config.feature.numFilters;
        opts.minFreq = config.feature.minFreq;
        opts.maxFreq = config.feature.maxFreq > 0.0f ? config.feature.maxFreq : static_cast<float>(sampleRate) / 2.0f;
        opts.includeEnergy = config.feature.includeEnergy;
        opts.filterbank = config.feature.filterbank;
        opts.melScale = config.feature.melScale;
        opts.compressionType = config.feature.compressionType;
        return opts;
    }
}

namespace libvoicefeat
{
    FeatureMatrix computeFileMfcc(const std::filesystem::path& path, const CepstralConfig& config)
    {
        auto audio = loadAudio(path);
        return computeBufferMfcc(audio, config);
    }

    FeatureMatrix computeBufferMfcc(const audio::AudioBuffer& audio, const CepstralConfig& config)
    {
        if (config.framing.frameSize <= 0 || config.framing.frameStep <= 0)
            throw std::invalid_argument("Frame size and step must be positive");

        int sampleRate = audio.sampleRate > 0 ? audio.sampleRate : config.feature.sampleRate;
        if (sampleRate <= 0)
            throw std::invalid_argument("Sample rate must be positive");

        audio::AudioBuffer working = audio;
        working.sampleRate = sampleRate;

        if (config.preemphasis.usePreEmphasis)
            applyPreEmphasis(working.samples, config.preemphasis.preEmphasisCoeff);

        dsp::FixedFrameExtractor extractor(config.framing.frameSize, config.framing.frameStep);
        auto frames = extractor.extract(working);
        if (frames.empty())
            return {};

        dsp::HanningWindow window(config.framing.frameSize);
        for (auto& frame : frames)
            window.apply(frame.data);

        dsp::FFTTransformer transformer;
        const auto options = buildOptions(sampleRate, config);
        auto mfccFeature = features::FeatureDirector::createDefaultMfccFeature(config);
        const auto base = mfccFeature.compute(frames, transformer);
        // auto base = features::computeMFCC(frames, transformer, options);
        return features::appendDeltas(base, config.delta.useDeltas, config.delta.useDeltaDeltas);
    }
}
