#include "libvoicefeat/features/filterbanks/filterbank.h"

#include <algorithm>
#include <cmath>

namespace libvoicefeat::features::detail
{
    namespace
    {
        [[nodiscard]] std::vector<int> buildBins(const FilterbankParams& params, const std::vector<double>& hzPoints)
        {
            const int nFreqs = params.nFft / 2 + 1;
            const double nyquist = static_cast<double>(params.sampleRate) / 2.0;
            std::vector<int> bins(hzPoints.size(), 0);
            for (std::size_t i = 0; i < hzPoints.size(); ++i)
            {
                const double freq = std::clamp(hzPoints[i], 0.0, nyquist);
                const double bin = std::floor((params.nFft + 1) * freq / params.sampleRate);
                bins[i] = std::clamp(static_cast<int>(bin), 0, nFreqs - 1);
            }
            return bins;
        }
    }

    std::vector<std::vector<double>> buildTriangularFilters(const FilterbankParams& params,
                                                            const std::vector<double>& hzPoints)
    {
        if (params.numFilters <= 0)
        {
            return {};
        }

        const int nFreqs = params.nFft / 2 + 1;
        std::vector<std::vector<double>> filters(params.numFilters, std::vector<double>(nFreqs, 0.0));
        if (hzPoints.size() < static_cast<std::size_t>(params.numFilters + 2))
        {
            return filters;
        }

        const auto bins = buildBins(params, hzPoints);
        for (int m = 1; m <= params.numFilters; ++m)
        {
            const int left = bins[m - 1];
            const int center = bins[m];
            const int right = bins[m + 1];

            const double leftDenom = static_cast<double>(std::max(1, center - left));
            const double rightDenom = static_cast<double>(std::max(1, right - center));

            for (int k = left; k < center; ++k)
            {
                filters[m - 1][k] = (k - left) / leftDenom;
            }
            for (int k = center; k < right; ++k)
            {
                filters[m - 1][k] = (right - k) / rightDenom;
            }
        }
        return filters;
    }
}