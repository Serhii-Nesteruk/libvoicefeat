#pragma once

#include "transformer.h"

namespace libvoicefeat::dsp
{
    class DFTTransformer : public ITransformer {
    public:
        [[nodiscard]] std::vector<std::complex<float>> transform(const std::vector<float>& frame) const override;
    };
}
