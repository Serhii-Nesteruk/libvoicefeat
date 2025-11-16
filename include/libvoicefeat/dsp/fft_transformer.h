#pragma once

#include "transformer.h"

namespace libvoicefeat::dsp
{
    class FFTTransformer : public ITransformer {
    public:
        [[nodiscard]] std::vector<std::complex<float>> transform(const std::vector<float>& frame) const override;
    private:
        void fft(std::vector<std::complex<float>>& data) const;
    };
}