#pragma once

#include "libvoicefeat/config.h"

namespace libvoicefeat::features {

    [[nodiscard]] FeatureMatrix computeDelta(const FeatureMatrix& feature, int N = 2);
    [[nodiscard]] FeatureMatrix computeDeltaDelta(const FeatureMatrix& feature, int N = 2);
    [[nodiscard]] FeatureMatrix appendDeltas(
        const FeatureMatrix& base,
        bool useDelta,
        bool useDeltaDelta,
        int N = 2
    );

}