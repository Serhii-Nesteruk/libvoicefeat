#include "libvoicefeat/features/mfcc.h"

#include "libvoicefeat/config.h"
#include "libvoicefeat/features/filterbanks/filterbank.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <vector>

namespace
{
    constexpr double kLogEps = 1e-10;
    constexpr double kPi = 3.14159265358979323846;

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

        FilterbankParams filterParams{};
        filterParams.sampleRate = opts.sampleRate;
        filterParams.nFft = nFft;
        filterParams.numFilters = opts.numFilters;
        filterParams.minFreq = opts.minFreq;
        filterParams.maxFreq = opts.maxFreq;

        const auto filterbank = createFilterbank(opts.filterbank, opts.melScale);
        const auto filters = filterbank->build(filterParams);
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