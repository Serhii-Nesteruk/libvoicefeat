#pragma once

#include "libmfcc/dsp/frame.h"
#include "libmfcc/dsp/transformer.h"

#include <vector>

namespace libmfcc::features
{
    [[nodiscard]] float hzToMel(float hz);
    [[nodiscard]] float melToHz(float mel);
    void buildMelFilterbank(int sampleRate,
                               int nFft,
                               int nMels,
                               std::vector<std::vector<float>>& filters);

    [[nodiscard]] std::vector<float> magnitude(const std::vector<std::complex<float>>& spec);
    [[nodiscard]] std::vector<float> applyMel(const std::vector<std::vector<float>>& filters,
                                       const std::vector<float>& mag);

    [[nodiscard]] std::vector<float> dctII(const std::vector<float>& v, int nCoeffs);
    [[nodiscard]] std::vector<std::vector<float>> computeMFCC(
        const std::vector<libmfcc::dsp::Frame>& frames,
        int sampleRate,
        const libmfcc::dsp::ITransformer& transformer,
        int nMels = 26,
        int nCoeffs = 13
    );
}
