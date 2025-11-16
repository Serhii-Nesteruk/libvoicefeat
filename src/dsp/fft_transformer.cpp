#include "libvoicefeat/dsp/fft_transformer.h"
#include <cmath>

namespace libvoicefeat::dsp {

void FFTTransformer::fft(std::vector<std::complex<float>>& data) const {
    const size_t N = data.size();
    if (N <= 1) return;

    std::vector<std::complex<float>> even(N / 2);
    std::vector<std::complex<float>> odd(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = data[i * 2];
        odd[i] = data[i * 2 + 1];
    }

    fft(even);
    fft(odd);

    for (size_t k = 0; k < N / 2; ++k) {
        std::complex<float> t = std::polar(1.f, -2.f * static_cast<float>(M_PI) * k / N) * odd[k];
        data[k] = even[k] + t;
        data[k + N / 2] = even[k] - t;
    }
}

std::vector<std::complex<float>> FFTTransformer::transform(const std::vector<float>& frame) const {
    size_t N = 1;
    while (N < frame.size()) N <<= 1;
    std::vector<std::complex<float>> data(N);
    for (size_t i = 0; i < frame.size(); ++i)
        data[i] = frame[i];

    fft(data);
    return data;
}

}