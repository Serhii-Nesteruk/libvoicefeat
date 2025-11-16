#pragma once

#include "libvoicefeat/config.h"
#include "libvoicefeat/dsp/frame.h"
#include "libvoicefeat/dsp/transformer.h"
#include "libvoicefeat/features/mel_scale.h"

#include <vector>

namespace libvoicefeat::features
{
    [[nodiscard]] FeatureMatrix computeMFCC(const std::vector<libvoicefeat::dsp::Frame>& frames,
                       const libvoicefeat::dsp::ITransformer& transformer,
                       const FeatureOptions& options);
}


