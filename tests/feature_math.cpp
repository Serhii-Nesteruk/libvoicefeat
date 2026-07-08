#include "libvoicefeat/dsp/voice_activity_detector.h"
#include "libvoicefeat/features/cmvn_normalizer.h"
#include "libvoicefeat/features/feature.h"
#include "libvoicefeat/features/filterbanks/filterbank.h"
#include "libvoicefeat/features/mel_scale.h"
#include "libvoicefeat/utils/constants.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace
{
    constexpr double kTolerance = 1e-5;

    bool approximatelyEqual(double a, double b, double tolerance = kTolerance)
    {
        return std::fabs(a - b) <= tolerance;
    }

    int fail(const std::string& message)
    {
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    double htkMel(double hz)
    {
        return 2595.0 * std::log10(1.0 + hz / 700.0);
    }

    std::vector<double> referenceDctII(const std::vector<double>& v, int numCoeffs)
    {
        const int N = static_cast<int>(v.size());
        const int K = std::min(numCoeffs, N);
        std::vector<double> out(K, 0.0);
        for (int k = 0; k < K; ++k)
        {
            for (int n = 0; n < N; ++n)
            {
                const double angle = libvoicefeat::constants::PI * k * (2.0 * n + 1.0) / (2.0 * N);
                out[k] += v[n] * std::cos(angle);
            }
        }
        return out;
    }

    class FixedSpectrumTransformer final : public libvoicefeat::dsp::ITransformer
    {
    public:
        explicit FixedSpectrumTransformer(std::vector<std::complex<float>> spectrum)
            : _spectrum(std::move(spectrum))
        {
        }

        [[nodiscard]] std::vector<std::complex<float>> transform(const std::vector<float>&) const override
        {
            return _spectrum;
        }

    private:
        std::vector<std::complex<float>> _spectrum;
    };
}

int main()
{
    using namespace libvoicefeat;
    using namespace libvoicefeat::features;

    // -----------------------------
    // Mel scale formulas match known values and round-trip.
    // -----------------------------
    {
        const double htkAt1000Hz = hzToMel(1000.0, MelScale::HTK);
        if (!approximatelyEqual(htkAt1000Hz, htkMel(1000.0)))
            return fail("HTK mel value mismatch at 1000 Hz");

        for (MelScale scale : {MelScale::HTK, MelScale::Slaney})
        {
            for (double hz : {0.0, 125.0, 1000.0, 3500.0, 7600.0})
            {
                const double roundTripHz = melToHz(hzToMel(hz, scale), scale);
                if (!approximatelyEqual(roundTripHz, hz, 1e-4))
                    return fail("Mel scale round-trip mismatch");
            }
        }
    }

    // -----------------------------
    // Linear filterbank builds predictable triangular weights.
    // -----------------------------
    {
        FilterbankParams params;
        params.sampleRate = 8;
        params.nFft = 8;
        params.numFilters = 2;
        params.minFreq = 0.0;
        params.maxFreq = 4.0;

        const auto filterbank = createFilterbank(FilterbankType::Linear, MelScale::Slaney);
        const auto filters = filterbank->build(params);
        const std::vector<std::vector<double>> expected{
            {0.0, 1.0, 0.5, 0.0, 0.0},
            {0.0, 0.0, 0.5, 1.0, 0.0},
        };

        if (filters.size() != expected.size())
            return fail("Unexpected number of linear filters");

        for (std::size_t m = 0; m < expected.size(); ++m)
        {
            if (filters[m].size() != expected[m].size())
                return fail("Unexpected linear filter width");

            for (std::size_t k = 0; k < expected[m].size(); ++k)
            {
                if (!approximatelyEqual(filters[m][k], expected[m][k]))
                    return fail("Linear filterbank weight mismatch");
            }
        }
    }

    // -----------------------------
    // Feature math: magnitude -> linear filterbank -> log -> DCT-II.
    // -----------------------------
    {
        Feature feature;
        FeatureOptions options;
        options.sampleRate = 8;
        options.numFilters = 2;
        options.numCoeffs = 2;
        options.minFreq = 0.0;
        options.maxFreq = 4.0;
        options.includeEnergy = false;
        options.filterbank = FilterbankType::Linear;
        options.compressionType = CompressionType::Log;
        feature.setOptions(options);
        feature.setCepstralType(CepstralType::MFCC);

        const std::vector<dsp::Frame> frames{{std::vector<float>{1.0f, 1.0f, 1.0f, 1.0f}}};
        FixedSpectrumTransformer transformer(std::vector<std::complex<float>>(8, {1.0f, 0.0f}));

        const auto matrix = feature.compute(frames, transformer);
        if (matrix.size() != 1 || matrix.front().size() != 2)
            return fail("Unexpected feature matrix shape");

        const double bandEnergy = 1.5;
        const auto expected = referenceDctII(
            {std::log(bandEnergy + constants::K_LOG_EPS), std::log(bandEnergy + constants::K_LOG_EPS)},
            2);

        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (!approximatelyEqual(matrix.front()[i], expected[i], 1e-5))
                return fail("Feature DCT pipeline value mismatch");
        }
    }

    // -----------------------------
    // CMVN produces zero mean and unit variance on a known matrix.
    // -----------------------------
    {
        CepstralConfig config;
        config.cmvn.enabled = true;

        FeatureMatrix matrix{{1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f}};
        CmvnNormalizer cmvn(config);
        cmvn.apply(matrix);

        for (std::size_t d = 0; d < matrix.front().size(); ++d)
        {
            double mean = 0.0;
            for (const auto& row : matrix)
                mean += row[d];
            mean /= static_cast<double>(matrix.size());

            double variance = 0.0;
            for (const auto& row : matrix)
            {
                const double diff = row[d] - mean;
                variance += diff * diff;
            }
            variance /= static_cast<double>(matrix.size());

            if (!approximatelyEqual(mean, 0.0, 1e-6))
                return fail("CMVN mean is not zero");
            if (!approximatelyEqual(std::sqrt(variance), 1.0, 1e-6))
                return fail("CMVN variance is not one");
        }
    }

    // -----------------------------
    // VAD classifies controlled low/high energy frames.
    // -----------------------------
    {
        CepstralConfig config;
        config.framing.frameSize = 4;
        config.framing.frameStep = 4;
        config.vad.energyThresholdDb = 5.0f;
        config.vad.noiseFloorPercentile = 0.25f;
        config.vad.minSpeechFrames = 1;

        audio::AudioBuffer audio;
        audio.sampleRate = 16000;
        audio.samples = {
            0.001f, 0.001f, 0.001f, 0.001f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.001f, 0.001f, 0.001f, 0.001f,
        };

        dsp::VoiceActivityDetector vad(config);
        const auto flags = vad.detect(audio);
        const VADFlags expected{
            VADState::NonSpeech,
            VADState::Speech,
            VADState::Speech,
            VADState::NonSpeech,
        };

        if (flags != expected)
            return fail("VAD flags mismatch on controlled signal");
    }

    return EXIT_SUCCESS;
}
