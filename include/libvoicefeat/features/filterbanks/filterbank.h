#pragma once

#include "libvoicefeat/config.h"

#include <memory>
#include <vector>

namespace libvoicefeat::features
{
    struct FilterbankParams
    {
        int sampleRate = 0;
        int nFft = 0;
        int numFilters = 0;
        double minFreq = 0.0;
        double maxFreq = 0.0;
    };

    class IFilterbank
    {
    public:
        virtual ~IFilterbank() = default;

        [[nodiscard]] virtual std::vector<std::vector<double>> build(const FilterbankParams& params) const = 0;
    };

    [[nodiscard]] std::unique_ptr<IFilterbank> createFilterbank(FilterbankType type, MelScale melScale);
}