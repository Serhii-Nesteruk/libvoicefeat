#pragma once

#include "libvoicefeat/audio/audio_buffer.h"
#include "libvoicefeat/config.h"

#include <filesystem>

#include "compat/source_location.h"

namespace libvoicefeat {

    [[nodiscard]] FeatureMatrix computeFileMfcc(
        const std::filesystem::path& path,
        const CepstralConfig& config = {}
    );

    [[nodiscard]] FeatureMatrix computeBufferMfcc(
        const audio::AudioBuffer& audio,
        const CepstralConfig& config = {}
    );

}
