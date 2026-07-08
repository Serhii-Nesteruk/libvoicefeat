#include "libvoicefeat/dsp/transformer.h"
#include "libvoicefeat/features/cmvn_normalizer.h"
#include "libvoicefeat/features/delta.h"
#include "libvoicefeat/features/feature.h"
#include "libvoicefeat/features/feature_builder.h"
#include "libvoicefeat/features/filterbanks/filterbank.h"
#include "libvoicefeat/utils/constants.h"

#include <cmath>
#include <complex>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
    constexpr float kTolerance = 1e-5f;

    int fail(const std::string& message)
    {
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    bool approximatelyEqual(float a, float b, float tolerance = kTolerance)
    {
        return std::fabs(a - b) <= tolerance;
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

    bool allFinite(const libvoicefeat::FeatureMatrix& matrix)
    {
        for (const auto& row : matrix)
        {
            for (float value : row)
            {
                if (!std::isfinite(value))
                    return false;
            }
        }
        return true;
    }

    const char* cepstralName(libvoicefeat::CepstralType type)
    {
        using namespace libvoicefeat;
        switch (type)
        {
        case CepstralType::MFCC: return "MFCC";
        case CepstralType::LFCC: return "LFCC";
        case CepstralType::GFCC: return "GFCC";
        case CepstralType::PNCC: return "PNCC";
        case CepstralType::PLP: return "PLP";
        }
        return "unknown";
    }

    libvoicefeat::features::Feature buildManualFeature(libvoicefeat::CepstralType type,
                                                       libvoicefeat::FilterbankType filterbank,
                                                       libvoicefeat::CompressionType compression)
    {
        libvoicefeat::features::Feature feature;
        libvoicefeat::FeatureOptions options;
        options.sampleRate = 16000;
        options.numFilters = 8;
        options.numCoeffs = 4;
        options.minFreq = 0.0;
        options.maxFreq = 8000.0;
        options.includeEnergy = false;
        options.filterbank = filterbank;
        options.compressionType = compression;

        feature.setOptions(options);
        feature.setCepstralType(type);
        return feature;
    }
}

int main()
{
    using namespace libvoicefeat;
    using namespace libvoicefeat::features;

    // -----------------------------
    // All filterbank types produce finite non-negative triangular banks.
    // -----------------------------
    {
        FilterbankParams params;
        params.sampleRate = 16000;
        params.nFft = 512;
        params.numFilters = 10;
        params.minFreq = 20.0;
        params.maxFreq = 7600.0;

        for (FilterbankType type : {
                 FilterbankType::Mel,
                 FilterbankType::Linear,
                 FilterbankType::Gammatone,
                 FilterbankType::Bark,
             })
        {
            const auto filterbank = createFilterbank(type, MelScale::Slaney);
            const auto filters = filterbank->build(params);
            if (filters.size() != static_cast<std::size_t>(params.numFilters))
                return fail("Filterbank count mismatch");

            for (const auto& filter : filters)
            {
                if (filter.size() != static_cast<std::size_t>(params.nFft / 2 + 1))
                    return fail("Filterbank width mismatch");

                double sum = 0.0;
                for (double weight : filter)
                {
                    if (!std::isfinite(weight) || weight < 0.0 || weight > 1.0)
                        return fail("Filterbank weight outside valid range");
                    sum += weight;
                }
                if (sum <= 0.0)
                    return fail("Filterbank contains an empty filter");
            }
        }
    }

    // -----------------------------
    // FeatureBuilder writes every public option into Feature.
    // -----------------------------
    {
        const VADFlags flags{VADState::Speech, VADState::NonSpeech};
        const auto feature = FeatureBuilder{}
            .setSampleRate(22050)
            .setNumFilters(17)
            .setNumCoeffs(6)
            .setMinFreq(30.0)
            .setMaxFreq(7000.0)
            .setIncludeEnergy(false)
            .setFBankType(FilterbankType::Bark)
            .setMelScale(MelScale::HTK)
            .setCepstralType(CepstralType::PLP)
            .setCompressionType(CompressionType::CubeRoot)
            .setVADFlags(flags)
            .useDeltas(true)
            .useDeltaDeltas(true)
            .build();

        const auto options = feature.getOptions();
        if (options.sampleRate != 22050 ||
            options.numFilters != 17 ||
            options.numCoeffs != 6 ||
            options.minFreq != 30.0 ||
            options.maxFreq != 7000.0 ||
            options.includeEnergy ||
            options.filterbank != FilterbankType::Bark ||
            options.melScale != MelScale::HTK ||
            options.compressionType != CompressionType::CubeRoot)
        {
            return fail("FeatureBuilder option mismatch");
        }
        if (feature.getCepstralType() != CepstralType::PLP || feature.getVADFlags() != flags)
            return fail("FeatureBuilder metadata mismatch");
    }

    // -----------------------------
    // MFCC computes finite coefficients through the implemented cepstral path.
    // -----------------------------
    {
        const std::vector<dsp::Frame> frames{
            {std::vector<float>{1.0f, -0.5f, 0.25f, -0.125f}},
            {std::vector<float>{0.5f, 0.25f, -0.25f, -0.5f}},
        };
        FixedSpectrumTransformer transformer(std::vector<std::complex<float>>(512, {1.0f, 0.0f}));

        auto feature = buildManualFeature(CepstralType::MFCC, FilterbankType::Mel, CompressionType::Log);
        const auto matrix = feature.compute(frames, transformer);
        if (matrix.size() != frames.size())
            return fail("MFCC feature frame count mismatch");
        for (const auto& row : matrix)
        {
            if (row.size() != 4)
                return fail("MFCC coefficient count mismatch");
        }
        if (!allFinite(matrix))
            return fail("MFCC feature contains non-finite value");
    }

    // -----------------------------
    // Non-MFCC cepstral families must not silently pass until their real algorithms exist.
    // -----------------------------
    {
        const std::vector<dsp::Frame> frames{
            {std::vector<float>{1.0f, -0.5f, 0.25f, -0.125f}},
        };
        FixedSpectrumTransformer transformer(std::vector<std::complex<float>>(512, {1.0f, 0.0f}));

        struct Case
        {
            CepstralType type;
            FilterbankType filterbank;
            CompressionType compression;
        };

        const std::vector<Case> unimplementedCases{
            {CepstralType::LFCC, FilterbankType::Linear, CompressionType::Log},
            {CepstralType::GFCC, FilterbankType::Gammatone, CompressionType::Log},
            {CepstralType::PNCC, FilterbankType::Mel, CompressionType::PowerNormalized},
            {CepstralType::PLP, FilterbankType::Bark, CompressionType::CubeRoot},
        };

        std::vector<std::string> silentlyComputed;
        for (const auto& testCase : unimplementedCases)
        {
            bool threw = false;
            try
            {
                auto feature = buildManualFeature(testCase.type, testCase.filterbank, testCase.compression);
                (void)feature.compute(frames, transformer);
            }
            catch (const std::runtime_error&)
            {
                threw = true;
            }
            catch (const std::invalid_argument&)
            {
                threw = true;
            }

            if (!threw)
                silentlyComputed.emplace_back(cepstralName(testCase.type));
        }

        if (!silentlyComputed.empty())
        {
            std::cerr << "Unimplemented cepstral types silently computed:";
            for (const auto& name : silentlyComputed)
                std::cerr << " " << name;
            std::cerr << std::endl;
            return EXIT_FAILURE;
        }
    }

    // -----------------------------
    // Log energy replaces c0 when requested.
    // -----------------------------
    {
        auto feature = buildManualFeature(CepstralType::MFCC, FilterbankType::Mel, CompressionType::Log);
        auto options = feature.getOptions();
        options.includeEnergy = true;
        feature.setOptions(options);

        const std::vector<dsp::Frame> frames{{std::vector<float>{1.0f, 2.0f, 0.0f, 0.0f}}};
        FixedSpectrumTransformer transformer(std::vector<std::complex<float>>(512, {1.0f, 0.0f}));
        const auto matrix = feature.compute(frames, transformer);

        const float expectedEnergy = static_cast<float>(std::log(5.0 + constants::K_LOG_EPS));
        if (matrix.empty() || matrix.front().empty() || !approximatelyEqual(matrix.front().front(), expectedEnergy))
            return fail("Log energy c0 mismatch");
    }

    // -----------------------------
    // Factory defaults produce MFCC output.
    // -----------------------------
    {
        const std::vector<dsp::Frame> frames{{std::vector<float>(400, 0.25f)}};
        FixedSpectrumTransformer transformer(std::vector<std::complex<float>>(512, {1.0f, 0.0f}));

        CepstralConfig config;
        config.type = CepstralType::MFCC;
        config.feature.sampleRate = 16000;
        config.feature.maxFreq = 8000.0;

        auto feature = FeatureFactory::createDefaultFeature(config);
        if (feature.getCepstralType() != CepstralType::MFCC)
            return fail("FeatureFactory MFCC type mismatch");

        const auto matrix = feature.compute(frames, transformer);
        if (matrix.empty() || matrix.front().empty() || !allFinite(matrix))
            return fail("FeatureFactory produced invalid MFCC matrix");
    }

    // -----------------------------
    // Factory must reject non-MFCC until those feature families are implemented.
    // -----------------------------
    {
        std::vector<std::string> silentlyCreated;
        for (CepstralType type : {
                 CepstralType::LFCC,
                 CepstralType::GFCC,
                 CepstralType::PNCC,
                 CepstralType::PLP,
             })
        {
            CepstralConfig config;
            config.type = type;

            bool threw = false;
            try
            {
                (void)FeatureFactory::createDefaultFeature(config);
            }
            catch (const std::runtime_error&)
            {
                threw = true;
            }
            catch (const std::invalid_argument&)
            {
                threw = true;
            }

            if (!threw)
                silentlyCreated.emplace_back(cepstralName(type));
        }

        if (!silentlyCreated.empty())
        {
            std::cerr << "FeatureFactory created unimplemented cepstral types:";
            for (const auto& name : silentlyCreated)
                std::cerr << " " << name;
            std::cerr << std::endl;
            return EXIT_FAILURE;
        }
    }

    // -----------------------------
    // CMVN can use VAD flags as the statistics mask.
    // -----------------------------
    {
        CepstralConfig config;
        FeatureMatrix matrix{{100.0f}, {2.0f}, {4.0f}};
        const VADFlags vad{VADState::NonSpeech, VADState::Speech, VADState::Speech};

        CmvnNormalizer cmvn(config);
        cmvn.apply(matrix, &vad);

        if (!approximatelyEqual(matrix[1][0], -1.0f) ||
            !approximatelyEqual(matrix[2][0], 1.0f) ||
            !approximatelyEqual(matrix[0][0], 97.0f))
        {
            return fail("VAD-masked CMVN mismatch");
        }
    }

    // -----------------------------
    // Delta helpers reject invalid regression windows.
    // -----------------------------
    {
        bool threw = false;
        try
        {
            (void)computeDelta(FeatureMatrix{{1.0f}}, 0);
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }
        if (!threw)
            return fail("Invalid delta window did not throw");
    }

    return EXIT_SUCCESS;
}
