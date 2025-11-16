#include "libvoicefeat/features/filterbanks/linear_filterbank.h"

#include "libvoicefeat/features/filterbanks/filterbank_utils.h"

namespace libvoicefeat::features
{
    std::vector<std::vector<double>> LinearFilterbank::build(const FilterbankParams& params) const
    {
        if (params.numFilters <= 0)
        {
            return {};
        }

        std::vector<double> hzPoints(params.numFilters + 2, 0.0);
        for (int i = 0; i < params.numFilters + 2; ++i)
        {
            hzPoints[i] = params.minFreq + (params.maxFreq - params.minFreq) * i / (params.numFilters + 1);
        }

        return detail::buildTriangularFilters(params, hzPoints);
    }
}