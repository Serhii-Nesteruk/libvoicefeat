#pragma once

#include "libvoicefeat/features/filterbanks/filterbank.h"

namespace libvoicefeat::features
{
    class MelFilterbank : public IFilterbank
    {
    public:
        explicit MelFilterbank(MelScale scale);

        [[nodiscard]] std::vector<std::vector<double>> build(const FilterbankParams& params) const override;

    private:
        MelScale _scale;
    };
}