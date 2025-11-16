#include "libvoicefeat/features/filterbanks/mel_filterbank.h"

#include "libvoicefeat/features/filterbanks/filterbank_utils.h"
#include "libvoicefeat/features/mel_scale.h"

namespace libvoicefeat::features
{
    MelFilterbank::MelFilterbank(MelScale scale)
        : _scale(scale)
    {
    }

    std::vector<std::vector<double>> MelFilterbank::build(const FilterbankParams& params) const
    {
        if (params.numFilters <= 0)
        {
            return {};
        }

        const double melMin = hzToMel(params.minFreq, _scale);
        const double melMax = hzToMel(params.maxFreq, _scale);

        std::vector<double> melPoints(params.numFilters + 2, 0.0);
        for (int i = 0; i < params.numFilters + 2; ++i)
        {
            melPoints[i] = melMin + (melMax - melMin) * i / (params.numFilters + 1);
        }

        std::vector<double> hzPoints(melPoints.size(), 0.0);
        for (std::size_t i = 0; i < melPoints.size(); ++i)
        {
            hzPoints[i] = melToHz(melPoints[i], _scale);
        }

        return detail::buildTriangularFilters(params, hzPoints);
    }
}