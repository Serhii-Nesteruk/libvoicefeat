#include "libvoicefeat/dsp/dft_transformer.h"
#include <cmath>

#include "libvoicefeat/utils/constants.h"

namespace libvoicefeat::dsp
{

    std::vector<std::complex<float>> DFTTransformer::transform(const std::vector<float>& frame) const
    {
        const size_t N = frame.size();
        std::vector<std::complex<float>> out(N);

        for (size_t k = 0; k < N; ++k)
        {
            std::complex<float> sum(0.f, 0.f);
            for (size_t n = 0; n < N; ++n)
            {
                float angle = -2.f * constants::PI * k * n / float(N);
                std::complex<float> w(std::cos(angle), std::sin(angle));
                sum += frame[n] * w;
            }
            out[k] = sum;
        }
        return out;
    }
}
