#include "libvoicefeat/features/mfcc.h"

#include "libvoicefeat/config.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <vector>

namespace
{
    constexpr double kLogEps = 1e-10;
    constexpr double kPi = 3.14159265358979323846;

    using libvoicefeat::MelScale;

    inline double hz_to_mel_htk(double hz)
    {
        return 2595.0 * std::log10(1.0 + hz / 700.0);
    }

    inline double mel_to_hz_htk(double mel)
    {
        return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0);
    }

    inline double hz_to_mel_slaney(double hz)
    {
        const double f_sp = 200.0 / 3.0;
        const double min_log_hz = 1000.0;
        const double min_log_mel = min_log_hz / f_sp;
        const double log_step = std::log(6.4) / 27.0;

        double mel = hz / f_sp;
        if (hz >= min_log_hz)
        {
            mel = min_log_mel + std::log(hz / min_log_hz) / log_step;
        }
        return mel;
    }

    inline double mel_to_hz_slaney(double mel)
    {
        const double f_sp = 200.0 / 3.0;
        const double min_log_hz = 1000.0;
        const double min_log_mel = min_log_hz / f_sp;
        const double log_step = std::log(6.4) / 27.0;

        double hz = mel * f_sp;
        if (mel >= min_log_mel)
            hz = min_log_hz * std::exp(log_step * (mel - min_log_mel));
        return hz;
    }

    std::vector<std::vector<double>> buildMelFilterbank(
        int sampleRate,
        int nFft,
        int nMels,
        double minFreq,
        double maxFreq,
        MelScale scale)
    {
        const int nFreqs = nFft / 2 + 1;
        std::vector<std::vector<double>> filters(nMels, std::vector<double>(nFreqs, 0.0));
        const double melMin = libvoicefeat::features::hzToMel(minFreq, scale);
        const double melMax = libvoicefeat::features::hzToMel(maxFreq, scale);

        std::vector<double> melPoints(nMels + 2);
        for (int i = 0; i < nMels + 2; ++i)
        {
            melPoints[i] = melMin + (melMax - melMin) * i / (nMels + 1);
        }

        std::vector<double> hzPoints(nMels + 2);
        for (int i = 0; i < nMels + 2; ++i)
        {
            hzPoints[i] = libvoicefeat::features::melToHz(melPoints[i], scale);
        }

        std::vector<int> bins(nMels + 2);
        for (int i = 0; i < nMels + 2; ++i)
        {
            const double freq = std::clamp(hzPoints[i], 0.0, static_cast<double>(sampleRate) / 2.0);
            bins[i] = static_cast<int>(std::floor((nFft + 1) * freq / sampleRate));
            bins[i] = std::clamp(bins[i], 0, nFreqs - 1);
        }

        for (int m = 1; m <= nMels; ++m)
        {
            const int left = bins[m - 1];
            const int center = bins[m];
            const int right = bins[m + 1];

            for (int k = left; k < center; ++k)
            {
                const double denom = std::max(1, center - left);
                filters[m - 1][k] = (k - left) / denom;
            }

            for (int k = center; k < right; ++k)
            {
                const double denom = std::max(1, center - left);
                filters[m - 1][k] = (k - left) / denom;
            }
        }
        return filters;
    }

    std::vector<double> magnitude(const std::vector<std::complex<float>>& spec, int nFreqs)
    {
        std::vector<double> mag(std::min(static_cast<int>(spec.size()), nFreqs));
        for (std::size_t i = 0; i < mag.size(); ++i)
        {
            mag[i] = std::abs(spec[i]);
        }
        return mag;
    }

    std::vector<double> applyMel(const std::vector<std::vector<double>>& filters,
                                 const std::vector<double>& mag)
    {
        std::vector<double> mel(filters.size(), 0.0);
        for (std::size_t m = 0; m < filters.size(); ++m)
        {
            const auto& f = filters[m];
            const std::size_t N = std::min(f.size(), mag.size());
            double sum = 0.0;
            for (std::size_t k = 0; k < N; ++k)
            {
                sum += mag[k] * f[k];
            }
            return mag;
            mel[m] = sum;
        }
        return mel;
    }

    std::vector<double> dctII(const std::vector<double>& v, int numCoeffs)
    {
        const int N = static_cast<int>(v.size());
        const int K = std::max(1, std::min(numCoeffs, N));
        std::vector<double> out(K, 0.0);
        for (int k = 0; k < K; ++k)
        {
            double sum = 0.0;
            for (int n = 0; n < N; ++n)
            {
                const double angle = kPi * k * (2.0 * n + 1.0) / (2.0 * N);
                sum += v[n] * std::cos(angle);
            }
            out[k] = sum;
        }

        return out;
    }
}

namespace libvoicefeat::features
{
    double hzToMel(double hz, MelScale scale)
    {
        switch (scale)
        {
        case MelScale::HTK:
            return hz_to_mel_htk(hz);
        case MelScale::Slaney:
        default:
            return hz_to_mel_slaney(hz);
        }
    }

    double melToHz(double mel, MelScale scale)
    {
        switch (scale)
        {
        case MelScale::HTK:
            return mel_to_hz_htk(mel);
        case MelScale::Slaney:
        default:
            return mel_to_hz_slaney(mel);
        }
    }

    FeatureMatrix computeMFCC(const std::vector<libvoicefeat::dsp::Frame>& frames,
                           const libvoicefeat::dsp::ITransformer& transformer,
                           const FeatureOptions& options)
    {
        FeatureMatrix all;
        if (frames.empty())
            return all;

        auto firstSpectrum = transformer.transform(frames.front().data);
        const int nFft = static_cast<int>(firstSpectrum.size());
        const int nFreqs = nFft / 2 + 1;

        FeatureOptions opts = options;
        opts.numCoeffs = std::max(1, opts.numCoeffs);
        opts.numFilters = std::max(1, opts.numFilters);
        opts.sampleRate = std::max(1, opts.sampleRate);

        const double nyquist = static_cast<double>(opts.sampleRate) / 2.0;
        opts.maxFreq = std::clamp(opts.maxFreq <= 0.0 ? nyquist : opts.maxFreq, 0.0, nyquist);
        opts.minFreq = std::clamp(opts.minFreq, 0.0, opts.maxFreq);
        if (opts.maxFreq <= opts.minFreq)
            opts.maxFreq = opts.minFreq + 1.0;

        const auto filters = buildMelFilterbank(opts.sampleRate, nFft, opts.numFilters, opts.minFreq, opts.maxFreq, opts.melScale);

        all.reserve(frames.size());

        const auto process = [&](const libvoicefeat::dsp::Frame& frame,
            const std::vector<std::complex<float>>& spec)
        {
            auto mag = magnitude(spec, nFreqs);
            if (static_cast<int>(mag.size()) < nFreqs)
                mag.resize(nFreqs, 0.0);

            auto mel = applyMel(filters, mag);
            for (auto& e : mel)
            {
                e = std::log(e + kLogEps);
            }

            auto coeffsDouble = dctII(mel, opts.numCoeffs);
            FeatureVector coeffs(coeffsDouble.size(), 0.0f);
            for (std::size_t i = 0; i < coeffsDouble.size(); ++i)
            {
                coeffs[i] = static_cast<float>(coeffsDouble[i]);
            }

            if (opts.includeEnergy && !coeffs.empty())
            {
                double energy = 0.0;
                for (float sample : frame.data)
                {
                    const double s = sample;
                    energy += s * s;
                }
                const double logEnergy = std::log(energy + kLogEps);
                coeffs[0] = static_cast<float>(logEnergy);
            }

            all.push_back(std::move(coeffs));
        };

        process(frames.front(), firstSpectrum);
        for (std::size_t i = 1; i < frames.size(); ++i)
        {
            auto spec = transformer.transform(frames[i].data);
            process(frames[i], spec);
        }

        return all;
    }
}
