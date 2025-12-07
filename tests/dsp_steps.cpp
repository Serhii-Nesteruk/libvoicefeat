#include "libvoicefeat/dsp/dft_transformer.h"
#include "libvoicefeat/dsp/fft_transformer.h"
#include "libvoicefeat/dsp/frame_extractor.h"
#include "libvoicefeat/dsp/window_functiion.h"
#include "libvoicefeat/audio/audio_buffer.h"

#include <complex>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>

namespace
{
    constexpr float kTolerance = 1e-4f;

    bool approximatelyEqual(const std::complex<float>& a, const std::complex<float>& b)
    {
        return std::fabs(a.real() - b.real()) < kTolerance && std::fabs(a.imag() - b.imag()) < kTolerance;
    }

    bool approximatelyEqual(float a, float b)
    {
        return std::fabs(a - b) < kTolerance;
    }
}

int main()
{
    using namespace libvoicefeat;
    using namespace libvoicefeat::dsp;

    // -----------------------------
    // FFT transformer on a simple waveform
    // -----------------------------
    {
        const std::vector<float> frame{1.f, 0.f, -1.f, 0.f};
        FFTTransformer fft;
        auto spectrum = fft.transform(frame);

        const std::vector<std::complex<float>> expected{
            {0.f, 0.f},
            {2.f, 0.f},
            {0.f, 0.f},
            {2.f, 0.f},
        };

        if (spectrum.size() != expected.size())
        {
            std::cerr << "Unexpected FFT size" << std::endl;
            return EXIT_FAILURE;
        }

        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (!approximatelyEqual(spectrum[i], expected[i]))
            {
                std::cerr << "FFT value mismatch at bin " << i << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    // -----------------------------
    // DFT reference matches FFT output for the same small frame
    // -----------------------------
    {
        const std::vector<float> frame{0.f, 1.f, 0.f, -1.f};
        FFTTransformer fft;
        DFTTransformer dft;
        auto fftSpectrum = fft.transform(frame);
        auto dftSpectrum = dft.transform(frame);

        if (fftSpectrum.size() < dftSpectrum.size())
        {
            std::cerr << "FFT result shorter than expected" << std::endl;
            return EXIT_FAILURE;
        }

        for (std::size_t i = 0; i < dftSpectrum.size(); ++i)
        {
            if (!approximatelyEqual(fftSpectrum[i], dftSpectrum[i]))
            {
                std::cerr << "FFT/DFT mismatch at bin " << i << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    // -----------------------------
    // Windowing uses the expected coefficients
    // -----------------------------
    {
        dsp::WindowFunction window(4, WindowType::Hamming);
        std::vector<float> frame(4, 1.0f);
        window.apply(frame);

        const std::vector<float> expected{
            0.08f,
            0.77f,
            0.77f,
            0.08f,
        };

        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (!approximatelyEqual(frame[i], expected[i]))
            {
                std::cerr << "Window coefficient mismatch at index " << i << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    // -----------------------------
    // Frame extraction respects window and hop sizes
    // -----------------------------
    {
        audio::AudioBuffer buffer;
        buffer.sampleRate = 16000;
        buffer.samples = {0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f};

        FixedFrameExtractor extractor(4, 2);
        auto frames = extractor.extract(buffer);

        const std::size_t expectedFrames = 4; // (10 - 4) / 2 + 1
        if (frames.size() != expectedFrames)
        {
            std::cerr << "Unexpected frame count" << std::endl;
            return EXIT_FAILURE;
        }

        const std::vector<std::vector<float>> expected{
            {0.f, 1.f, 2.f, 3.f},
            {2.f, 3.f, 4.f, 5.f},
            {4.f, 5.f, 6.f, 7.f},
            {6.f, 7.f, 8.f, 9.f},
        };

        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (frames[i].data != expected[i])
            {
                std::cerr << "Frame content mismatch at index " << i << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}