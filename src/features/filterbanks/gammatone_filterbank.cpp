#include "libvoicefeat/features/filterbanks/gammatone_filterbank.h"

#include "libvoicefeat/features/filterbanks/filterbank_utils.h"

#include <cmath>

namespace
{
    double hzToErb(double hz)
    {
        return 21.4 * std::log10(4.37e-3 * hz + 1.0);
    }

    double erbToHz(double erb)
    {
        return (std::pow(10.0, erb / 21.4) - 1.0) / 4.37e-3;
    }
}

namespace libvoicefeat::features
{
    std::vector<std::vector<double>> GammatoneFilterbank::build(const FilterbankParams& params) const
    {
        if (params.numFilters <= 0)
        {
            return {};
        }

        const double erbMin = hzToErb(params.minFreq);
        const double erbMax = hzToErb(params.maxFreq);

        std::vector<double> erbPoints(params.numFilters + 2, 0.0);
        for (int i = 0; i < params.numFilters + 2; ++i)
        {
            erbPoints[i] = erbMin + (erbMax - erbMin) * i / (params.numFilters + 1);
        }

        std::vector<double> hzPoints(erbPoints.size(), 0.0);
        for (std::size_t i = 0; i < erbPoints.size(); ++i)
        {
            hzPoints[i] = erbToHz(erbPoints[i]);
        }

        return detail::buildTriangularFilters(params, hzPoints);
    }
}