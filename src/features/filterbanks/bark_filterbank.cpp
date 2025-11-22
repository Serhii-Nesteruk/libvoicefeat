#include "libvoicefeat/features/filterbanks/bark_filterbank.h"

#include "libvoicefeat/features/filterbanks/filterbank_utils.h"

#include <cmath>

namespace
{
    double hzToBark(double hz)
    {
        return 26.81 * hz / (1960.0 + hz) - 0.53;
    }

    double barkToHz(double bark)
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

        const double barkMin = hzToBark(params.minFreq);
        const double barkMax = hzToBark(params.maxFreq);

        std::vector<double> barkPoints(params.numFilters + 2, 0.0);
        for (int i = 0; i < params.numFilters + 2; ++i)
        {
            barkPoints[i] = barkMin + (barkMax - barkMin) * i / (params.numFilters + 1);
        }

        std::vector<double> hzPoints(barkPoints.size(), 0.0);
        for (std::size_t i = 0; i < barkPoints.size(); ++i)
        {
            hzPoints[i] = barkToHz(barkPoints[i]);
        }

        return detail::buildTriangularFilters(params, hzPoints);
    }
}