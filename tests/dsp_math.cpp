#include "libvoicefeat/audio/audio_buffer.h"
#include "libvoicefeat/dsp/dft_transformer.h"
#include "libvoicefeat/dsp/fft_transformer.h"
#include "libvoicefeat/dsp/frame_extractor.h"
#include "libvoicefeat/dsp/resampler.h"
#include "libvoicefeat/dsp/voice_activity_detector.h"
#include "libvoicefeat/dsp/window_functiion.h"
#include "libvoicefeat/utils/constants.h"

#include <cmath>
#include <complex>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    constexpr float kTolerance = 1e-4f;

    int fail(const std::string& message)
    {
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    bool approximatelyEqual(float a, float b, float tolerance = kTolerance)
    {
        return std::fabs(a - b) <= tolerance;
    }

    bool approximatelyEqual(const std::complex<float>& a,
                            const std::complex<float>& b,
                            float tolerance = kTolerance)
    {
        return approximatelyEqual(a.real(), b.real(), tolerance) &&
               approximatelyEqual(a.imag(), b.imag(), tolerance);
    }
}

int main()
{
    using namespace libvoicefeat;
    using namespace libvoicefeat::dsp;

    // -----------------------------
    // DFT of an impulse is one in every bin.
    // -----------------------------
    {
        DFTTransformer dft;
        const auto spectrum = dft.transform({1.0f, 0.0f, 0.0f, 0.0f});
        if (spectrum.size() != 4)
            return fail("DFT impulse size mismatch");

        for (const auto& bin : spectrum)
        {
            if (!approximatelyEqual(bin, {1.0f, 0.0f}))
                return fail("DFT impulse value mismatch");
        }
    }

    // -----------------------------
    // FFT zero-pads non-power-of-two input and matches DFT on padded data.
    // -----------------------------
    {
        FFTTransformer fft;
        DFTTransformer dft;

        const std::vector<float> frame{1.0f, 2.0f, 3.0f};
        const std::vector<float> padded{1.0f, 2.0f, 3.0f, 0.0f};
        const auto fftSpectrum = fft.transform(frame);
        const auto dftSpectrum = dft.transform(padded);

        if (fftSpectrum.size() != padded.size())
            return fail("FFT zero-padded size mismatch");

        for (std::size_t i = 0; i < dftSpectrum.size(); ++i)
        {
            if (!approximatelyEqual(fftSpectrum[i], dftSpectrum[i]))
                return fail("FFT zero-padded value mismatch");
        }
    }

    // -----------------------------
    // Hanning window uses the expected coefficients.
    // -----------------------------
    {
        WindowFunction window(4, WindowType::Hanning);
        std::vector<float> frame(4, 1.0f);
        window.apply(frame);

        const std::vector<float> expected{0.0f, 0.75f, 0.75f, 0.0f};
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (!approximatelyEqual(frame[i], expected[i]))
                return fail("Hanning window coefficient mismatch");
        }
    }

    // -----------------------------
    // Frame extractor rejects invalid geometry and drops incomplete tails.
    // -----------------------------
    {
        bool threwWindow = false;
        try
        {
            FixedFrameExtractor invalid(0, 1);
        }
        catch (const std::invalid_argument&)
        {
            threwWindow = true;
        }
        if (!threwWindow)
            return fail("Invalid frame window did not throw");

        audio::AudioBuffer audio;
        audio.sampleRate = 16000;
        audio.samples = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};
        FixedFrameExtractor extractor(4, 3);
        const auto frames = extractor.extract(audio);
        if (frames.size() != 1 || frames.front().data != std::vector<float>({0.0f, 1.0f, 2.0f, 3.0f}))
            return fail("Frame extractor tail handling mismatch");
    }

    // -----------------------------
    // Resampler validates parameters, preserves identity, and changes rate.
    // -----------------------------
    {
        audio::AudioBuffer input;
        input.sampleRate = 4000;
        for (int n = 0; n < 40; ++n)
        {
            input.samples.push_back(std::sin(2.0 * constants::PI * 100.0 * n / input.sampleRate));
        }

        const auto same = Resampler::resampleTo(input, input.sampleRate);
        if (same.sampleRate != input.sampleRate || same.samples != input.samples)
            return fail("Resampler identity mismatch");

        const auto upsampled = Resampler::resampleTo(input, 8000);
        if (upsampled.sampleRate != 8000 || upsampled.samples.empty())
            return fail("Resampler output metadata mismatch");
        if (upsampled.samples.size() < 70 || upsampled.samples.size() > 90)
            return fail("Resampler output size is outside expected range");
        for (float sample : upsampled.samples)
        {
            if (!std::isfinite(sample))
                return fail("Resampler produced non-finite sample");
        }

        bool threwTargetRate = false;
        try
        {
            (void)Resampler::resampleTo(input, 0);
        }
        catch (const std::invalid_argument&)
        {
            threwTargetRate = true;
        }
        if (!threwTargetRate)
            return fail("Invalid target sample rate did not throw");

        input.sampleRate = 0;
        bool threwInputRate = false;
        try
        {
            (void)Resampler::resampleTo(input, 8000);
        }
        catch (const std::invalid_argument&)
        {
            threwInputRate = true;
        }
        if (!threwInputRate)
            return fail("Invalid input sample rate did not throw");
    }

    // -----------------------------
    // VAD smoothing removes speech runs shorter than minSpeechFrames.
    // -----------------------------
    {
        CepstralConfig config;
        config.framing.frameSize = 4;
        config.framing.frameStep = 4;
        config.vad.energyThresholdDb = 5.0f;
        config.vad.noiseFloorPercentile = 0.25f;
        config.vad.minSpeechFrames = 2;

        audio::AudioBuffer audio;
        audio.sampleRate = 16000;
        audio.samples = {
            0.001f, 0.001f, 0.001f, 0.001f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.001f, 0.001f, 0.001f, 0.001f,
        };

        VoiceActivityDetector vad(config);
        const auto flags = vad.detect(audio);
        for (auto flag : flags)
        {
            if (flag != VADState::NonSpeech)
                return fail("VAD smoothing did not remove short speech run");
        }
    }

    return EXIT_SUCCESS;
}
