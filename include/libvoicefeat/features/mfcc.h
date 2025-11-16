#pragma once

#include "libvoicefeat/config.h"
#include "libvoicefeat/dsp/frame.h"
#include "libvoicefeat/dsp/transformer.h"

#include <vector>

namespace libvoicefeat::features
{
    struct FeatureOptions
    {
        int sampleRate = 16000;
        int numFilters = 26;
        int numCoeffs = 13;
        double minFreq = 0.0;
        double maxFreq = 8000.0;
        bool includeEnergy = true;
        MelScale melScale = MelScale::Slaney;
    };

    [[nodiscard]] double hzToMel(double hz, MelScale scale);
    [[nodiscard]] double melToHz(double mel, MelScale scale);

    [[nodiscard]] FeatureMatrix computeMFCC(const std::vector<libvoicefeat::dsp::Frame>& frames,
                       const libvoicefeat::dsp::ITransformer& transformer,
                       const FeatureOptions& options);
}