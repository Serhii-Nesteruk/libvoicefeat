#include "libvoicefeat/libvoicefeat.h"

#include "libvoicefeat/audio/mp3_audio_reader.h"
#include "libvoicefeat/audio/wav_audio_reader.h"
#include "libvoicefeat/dsp/frame_extractor.h"
#include "libvoicefeat/dsp/window_functiion.h"
#include "libvoicefeat/dsp/fft_transformer.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

#include "libvoicefeat/dsp/resampler.h"
#include "libvoicefeat/features/feature_builder.h"

namespace libvoicefeat
{
    CepstralExtractor::CepstralExtractor(const CepstralConfig& config)
        : _config(config)
    {
    }

    Feature CepstralExtractor::extractFromFile(const std::string& path)
    {
        auto buffer = loadAudio(path);
        return extractFromAudioBuffer(buffer);
    }

    Feature CepstralExtractor::extractFromAudioBuffer(const AudioBuffer& audio)
    {
        if (_config.framing.frameSize <= 0 || _config.framing.frameStep <= 0)
            throw std::invalid_argument("Frame size and step must be positive");

        AudioBuffer working = audio;
        if (audio.sampleRate != _config.feature.sampleRate)
        {
            int targetSampleRate = _config.feature.sampleRate;
            working = Resampler::resampleTo(audio, targetSampleRate);
        }

        if (working.samples.empty() || working.sampleRate != _config.feature.sampleRate)
            throw std::invalid_argument("Resampling is failed");

        if (_config.preemphasis.usePreEmphasis)
            applyPreEmphasis(working.samples, _config.preemphasis.preEmphasisCoeff);

        FixedFrameExtractor frameExtractor(_config.framing.frameSize, _config.framing.frameStep);
        auto frames = frameExtractor.extract(working);
        if (frames.empty())
            return {};

        WindowFunction window(_config.framing.frameSize, _config.framing.window);
        for (auto& frame : frames)
            window.apply(frame.data);

        FFTTransformer transformer;
        buildOptions(working.sampleRate);
        auto feature = FeatureFactory::createDefaultFeature(_config);
        feature.compute(frames, transformer);

        return feature;
    }

    AudioBuffer CepstralExtractor::loadAudio(const std::filesystem::path& path)
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
            WavAudioReader reader;
            return reader.load(path);
        }
        if (ext == ".mp3")
        {
            Mp3AudioReader reader;
            return reader.load(path);
        }

        throw std::invalid_argument("Unsupported audio format: " + path.string());
    }

    void CepstralExtractor::applyPreEmphasis(std::vector<float>& samples, float coeff)
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

    void CepstralExtractor::buildOptions(int sampleRate)
    {
        _options.sampleRate = sampleRate;
        _options.numCoeffs = _config.feature.numCoeffs;
        _options.numFilters = _config.feature.numFilters;
        _options.minFreq = _config.feature.minFreq;
        _options.maxFreq = _config.feature.maxFreq > 0.0f
                               ? _config.feature.maxFreq
                               : static_cast<float>(sampleRate) / 2.0f;
        _options.includeEnergy = _config.feature.includeEnergy;
        _options.filterbank = _config.feature.filterbank;
        _options.melScale = _config.feature.melScale;
        _options.compressionType = _config.feature.compressionType;
    }
}
