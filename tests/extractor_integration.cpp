#include "libvoicefeat/libvoicefeat.h"
#include "libvoicefeat/utils/constants.h"
#include "libvoicefeat/utils/path.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    int fail(const std::string& message)
    {
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    libvoicefeat::audio::AudioBuffer sineAudio(int sampleRate, int totalSamples, float frequency)
    {
        libvoicefeat::audio::AudioBuffer audio;
        audio.sampleRate = sampleRate;
        audio.samples.resize(totalSamples);
        for (int n = 0; n < totalSamples; ++n)
        {
            audio.samples[n] = std::sin(2.0 * libvoicefeat::constants::PI * frequency * n / sampleRate);
        }
        return audio;
    }

    int defaultCoeffCount(libvoicefeat::CepstralType type)
    {
        using namespace libvoicefeat;
        switch (type)
        {
        case CepstralType::MFCC: return constants::DEFAULT_MFCC_COEFFS_NUM;
        case CepstralType::LFCC: return constants::DEFAULT_LFCC_COEFFS_NUM;
        case CepstralType::GFCC: return constants::DEFAULT_GFCC_COEFFS_NUM;
        case CepstralType::PNCC: return constants::DEFAULT_PNCC_COEFFS_NUM;
        case CepstralType::PLP: return constants::DEFAULT_PLP_COEFFS_NUM;
        }
        return 0;
    }

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
}

int main()
{
    using namespace libvoicefeat;

    // -----------------------------
    // CepstralExtractor supports MFCC end-to-end.
    // -----------------------------
    {
        const auto audio = sineAudio(16000, 3200, 440.0f);

        CepstralConfig config;
        config.type = CepstralType::MFCC;
        config.cmvn.enabled = false;
        config.preemphasis.usePreEmphasis = false;
        config.delta.useDeltas = true;
        config.delta.useDeltaDeltas = true;

        CepstralExtractor extractor(config);
        auto feature = extractor.extractFromAudioBuffer(audio);
        const auto& matrix = feature.getComputedMatrix();

        if (feature.getCepstralType() != CepstralType::MFCC)
            return fail("Extractor MFCC type mismatch");
        if (matrix.empty())
            return fail("Extractor produced empty MFCC matrix");

        const std::size_t expectedWidth = static_cast<std::size_t>(defaultCoeffCount(CepstralType::MFCC) * 3);
        for (const auto& row : matrix)
        {
            if (row.size() != expectedWidth)
                return fail("Extractor MFCC width mismatch");
        }
        if (!allFinite(matrix))
            return fail("Extractor produced non-finite MFCC coefficient");
        if (feature.getVADFlags().size() != matrix.size())
            return fail("Extractor VAD flag count mismatch");
    }

    // -----------------------------
    // Non-MFCC extractors must not silently pass until implemented.
    // -----------------------------
    {
        const auto audio = sineAudio(16000, 3200, 440.0f);
        std::vector<std::string> silentlyExtracted;

        for (CepstralType type : {
                 CepstralType::LFCC,
                 CepstralType::GFCC,
                 CepstralType::PNCC,
                 CepstralType::PLP,
             })
        {
            CepstralConfig config;
            config.type = type;
            config.cmvn.enabled = false;
            config.preemphasis.usePreEmphasis = false;

            bool threw = false;
            try
            {
                CepstralExtractor extractor(config);
                (void)extractor.extractFromAudioBuffer(audio);
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
                silentlyExtracted.emplace_back(cepstralName(type));
        }

        if (!silentlyExtracted.empty())
        {
            std::cerr << "Extractor silently computed unimplemented cepstral types:";
            for (const auto& name : silentlyExtracted)
                std::cerr << " " << name;
            std::cerr << std::endl;
            return EXIT_FAILURE;
        }
    }

    // -----------------------------
    // Extractor resamples input audio when input rate differs from config rate.
    // -----------------------------
    {
        const auto audio = sineAudio(8000, 1600, 220.0f);

        CepstralConfig config;
        config.feature.sampleRate = 16000;
        config.cmvn.enabled = false;
        config.preemphasis.usePreEmphasis = false;

        CepstralExtractor extractor(config);
        auto feature = extractor.extractFromAudioBuffer(audio);
        const auto& matrix = feature.getComputedMatrix();
        if (matrix.empty() || !allFinite(matrix))
            return fail("Extractor resampling path produced invalid features");
    }

    // -----------------------------
    // File-based extraction works for repository WAV and MP3 fixtures.
    // -----------------------------
    {
        CepstralConfig config;
        config.cmvn.enabled = false;
        config.preemphasis.usePreEmphasis = false;

        CepstralExtractor extractor(config);
        const auto wav = extractor.extractFromFile("data/common_voice_en_42698961.wav").getComputedMatrix();
        const auto mp3 = extractor.extractFromFile("data/common_voice_en_42698961.mp3").getComputedMatrix();

        if (wav.empty() || mp3.empty())
            return fail("File-based extraction returned empty matrix");
        if (!allFinite(wav) || !allFinite(mp3))
            return fail("File-based extraction returned non-finite values");
    }

    // -----------------------------
    // Error paths: invalid framing, empty input, short input, unsupported extension.
    // -----------------------------
    {
        CepstralConfig invalidFrameConfig;
        invalidFrameConfig.framing.frameSize = 0;
        CepstralExtractor invalidFrameExtractor(invalidFrameConfig);

        bool threwInvalidFrame = false;
        try
        {
            (void)invalidFrameExtractor.extractFromAudioBuffer(sineAudio(16000, 800, 440.0f));
        }
        catch (const std::invalid_argument&)
        {
            threwInvalidFrame = true;
        }
        if (!threwInvalidFrame)
            return fail("Invalid frame config did not throw");

        CepstralExtractor extractor(CepstralConfig{});
        audio::AudioBuffer empty;
        empty.sampleRate = 16000;

        bool threwEmpty = false;
        try
        {
            (void)extractor.extractFromAudioBuffer(empty);
        }
        catch (const std::invalid_argument&)
        {
            threwEmpty = true;
        }
        if (!threwEmpty)
            return fail("Empty audio did not throw");

        auto shortFeature = extractor.extractFromAudioBuffer(sineAudio(16000, 100, 440.0f));
        if (!shortFeature.getComputedMatrix().empty())
            return fail("Short audio should produce an empty feature matrix");

        const auto unsupported = std::filesystem::temp_directory_path() / "libvoicefeat_unsupported.flac";
        std::ofstream(unsupported) << "not audio";
        bool threwUnsupported = false;
        try
        {
            (void)extractor.extractFromFile(unsupported.string());
        }
        catch (const std::invalid_argument&)
        {
            threwUnsupported = true;
        }
        if (!threwUnsupported)
            return fail("Unsupported file extension did not throw");
    }

    return EXIT_SUCCESS;
}
