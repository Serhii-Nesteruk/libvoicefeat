#pragma once

#include "libvoicefeat/features/filterbanks/filterbank.h"

namespace libvoicefeat::features
{
    class LinearFilterbank : public IFilterbank
    {
    public:
        [[nodiscard]] std::vector<std::vector<double>> build(const FilterbankParams& params) const override;
    };
}