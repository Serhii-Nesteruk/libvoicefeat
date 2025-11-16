#pragma once

#include "libvoicefeat/features/filterbanks/filterbank.h"

#include <vector>

namespace libvoicefeat::features::detail
{
    [[nodiscard]] std::vector<std::vector<double>> buildTriangularFilters(const FilterbankParams& params,
                                                                         const std::vector<double>& hzPoints);
}