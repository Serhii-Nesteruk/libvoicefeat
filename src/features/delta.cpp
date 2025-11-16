#include "libvoicefeat/features/delta.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace libvoicefeat::features
{
    FeatureMatrix computeDelta(const FeatureMatrix& mfcc, int N)
    {
        FeatureMatrix deltas;
        if (mfcc.empty())
            return deltas;

        if (N <= 0)
            throw std::invalid_argument("Delta window N must be positive");

        const std::size_t T = mfcc.size();
        const std::size_t D = mfcc.front().size();
        deltas.assign(T, FeatureVector(D, 0.0f));

        double denominator = 0.0;
        for (int n = 1; n <= N; ++n)
        {
            denominator += static_cast<double>(n * n);
        }
        denominator *= 2.0;
        const double norm = denominator == 0.0 ? 0.0 : 1.0 / denominator;

        for (std::size_t t = 0; t < T; ++t)
        {
            for (std::size_t d = 0; d < D; ++d)
            {
                double num = 0.0;
                for (int n = 1; n <= N; ++n)
                {
                    const std::size_t prev = std::clamp(static_cast<int>(t) - n, 0, static_cast<int>(T) - 1);
                    const std::size_t next = std::clamp(static_cast<int>(t) + n, 0, static_cast<int>(T) - 1);
                    num += static_cast<double>(n) * (mfcc[next][d] - mfcc[prev][d]);
                }
                deltas[t][d] = static_cast<float>(num * norm);
            }
        }

        return deltas;
    }

    FeatureMatrix computeDeltaDelta(const FeatureMatrix& mfcc, int N)
    {
        return computeDelta(computeDelta(mfcc, N), N);
    }

    FeatureMatrix appendDeltas(const FeatureMatrix& base, bool useDelta, bool useDeltaDelta, int N)
    {
        if (base.empty())
            return base;

        if (!useDelta && !useDeltaDelta)
            return base;

        const auto delta = useDelta ? computeDelta(base, N) : FeatureMatrix{};
        const auto deltaDelta = useDeltaDelta ? computeDeltaDelta(base, N) : FeatureMatrix{};

        FeatureMatrix out = base;
        for (std::size_t t = 0; t < out.size(); ++t)
        {
            if (useDelta)
                out[t].insert(out[t].end(), delta[t].begin(), delta[t].end());
            if (useDeltaDelta)
                out[t].insert(out[t].end(), deltaDelta[t].begin(), deltaDelta[t].end());
        }

        return out;
    }
}
