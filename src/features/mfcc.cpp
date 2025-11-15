#include "libmfcc/features/mfcc.h"

#include "libmfcc/dsp/dft_transformer.h"
#include "libmfcc/dsp/frame.h"
#include <cmath>
#include <algorithm>

namespace libmfcc::features
{
    float hzToMel(float hz)
    {
        return 2595.f * std::log10(1.f + hz / 700.f);
    }

    float melToHz(float mel)
    {
        return 700.f * (std::pow(10.f, mel / 2595.f) - 1.f);
    }

    void buildMelFilterbank(int sampleRate,
                            int nFft,
                            int nMels,
                            std::vector<std::vector<float>>& filters)
    {
        int nFreqs = nFft / 2 + 1;
        filters.assign(nMels, std::vector<float>(nFreqs, 0.f));

        float melMin = hzToMel(0.f);
        float melMax = hzToMel(sampleRate * 0.5f);

        std::vector<float> melPoints(nMels + 2);
        for (int i = 0; i < nMels + 2; ++i)
        {
            melPoints[i] = melMin + (melMax - melMin) * i / (nMels + 1);
        }

        std::vector<float> hzPoints(nMels + 2);
        for (int i = 0; i < nMels + 2; ++i)
        {
            hzPoints[i] = melToHz(melPoints[i]);
        }

        std::vector<int> bins(nMels + 2);
        for (int i = 0; i < nMels + 2; ++i)
        {
            bins[i] = static_cast<int>(std::floor((nFft + 1) * hzPoints[i] / sampleRate));
            if (bins[i] > nFreqs - 1) bins[i] = nFreqs - 1;
            if (bins[i] < 0) bins[i] = 0;
        }

        for (int m = 1; m <= nMels; ++m)
        {
            int left = bins[m - 1];
            int center = bins[m];
            int right = bins[m + 1];

            for (int k = left; k < center; ++k)
            {
                float v = (k - left) / float(center - left == 0 ? 1 : center - left);
                filters[m - 1][k] = v;
            }
            for (int k = center; k < right; ++k)
            {
                float v = (right - k) / float(right - center == 0 ? 1 : right - center);
                filters[m - 1][k] = v;
            }
        }
    }

    std::vector<float> magnitude(const std::vector<std::complex<float>>& spec)
    {
        std::vector<float> mag(spec.size());
        for (size_t i = 0; i < spec.size(); ++i)
        {
            mag[i] = std::abs(spec[i]);
        }
        return mag;
    }

    std::vector<float> applyMel(const std::vector<std::vector<float>>& filters,
                                const std::vector<float>& mag)
    {
        std::vector<float> mel(filters.size(), 0.f);
        for (size_t m = 0; m < filters.size(); ++m)
        {
            float s = 0.f;
            const auto& f = filters[m];
            size_t N = std::min(f.size(), mag.size());
            for (size_t k = 0; k < N; ++k)
            {
                s += mag[k] * f[k];
            }
            mel[m] = s;
        }
        return mel;
    }

    std::vector<float> dctII(const std::vector<float>& v, int nCoeffs)
    {
        int N = static_cast<int>(v.size());
        std::vector<float> out(nCoeffs, 0.f);

        for (int k = 0; k < nCoeffs; ++k)
        {
            float sum = 0.f;
            for (int n = 0; n < N; ++n)
            {
                float angle = static_cast<float>(M_PI) * k * (2.f * n + 1.f) / (2.f * N);
                sum += v[n] * std::cos(angle);
            }
            out[k] = sum;
        }
        return out;
    }

    std::vector<std::vector<float>> computeMFCC(
        const std::vector<libmfcc::dsp::Frame>& frames,
        int sampleRate,
        const libmfcc::dsp::ITransformer& transformer,
        int nMels,
        int nCoeffs
    )
    {
        std::vector<std::vector<float>> all;
        if (frames.empty())
            return all;

        auto firstSpectrum = transformer.transform(frames.front().data);
        const int nFft = static_cast<int>(firstSpectrum.size());
        const int nFreqs = nFft / 2 + 1;

        std::vector<std::vector<float>> melFilters;
        buildMelFilterbank(sampleRate, nFft, nMels, melFilters);

        all.reserve(frames.size());

        auto processSpectrum = [&](const std::vector<std::complex<float>>& spec)
        {
            auto mag = magnitude(spec);
            if (static_cast<int>(mag.size()) > nFreqs)
                mag.resize(nFreqs);

            auto mel = applyMel(melFilters, mag);

            for (auto& e : mel)
                e = std::log(e + 1e-10f);

            auto coeffs = dctII(mel, nCoeffs);
            all.push_back(std::move(coeffs));
        };

        processSpectrum(firstSpectrum);

        for (std::size_t i = 1; i < frames.size(); ++i)
        {
            auto spec = transformer.transform(frames[i].data);
            processSpectrum(spec);
        }

        return all;
    }
}
