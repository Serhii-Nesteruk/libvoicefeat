#include "libvoicefeat/features/filterbanks/bark_filterbank.h"

#include "libvoicefeat/features/filterbanks/filterbank_utils.h"

#include <cmath>

namespace
{
    double hz_to_bark(double hz)
    {
        return 26.81 * hz / (1960.0 + hz) - 0.53;
    }

    double bark_to_hz(double bark)
    {
        const double z = bark + 0.53;
        return (1960.0 * z) / (26.81 - z);
    }
}

namespace libvoicefeat::features
{
    std::vector<std::vector<double>> BarkFilterbank::build(const FilterbankParams& params) const
    {
        if (params.numFilters <= 0)
        {
            return {};
        }

        const double barkMin = hz_to_bark(params.minFreq);
        const double barkMax = hz_to_bark(params.maxFreq);

        std::vector<double> barkPoints(params.numFilters + 2, 0.0);
        for (int i = 0; i < params.numFilters + 2; ++i)
        {
            barkPoints[i] = barkMin + (barkMax - barkMin) * i / (params.numFilters + 1);
        }

        std::vector<double> hzPoints(barkPoints.size(), 0.0);
        for (std::size_t i = 0; i < barkPoints.size(); ++i)
        {
            hzPoints[i] = bark_to_hz(barkPoints[i]);
        }

        return detail::buildTriangularFilters(params, hzPoints);
    }
}