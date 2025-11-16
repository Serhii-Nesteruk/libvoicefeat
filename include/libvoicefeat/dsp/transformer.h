#pragma once

#include <vector>
#include <complex>

namespace libvoicefeat::dsp {

    class ITransformer {
    public:
        virtual ~ITransformer() = default;
        virtual std::vector<std::complex<float>> transform(const std::vector<float>& frame) const = 0;
    };

}