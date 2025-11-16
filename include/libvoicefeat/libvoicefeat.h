#pragma once

#include "libvoicefeat/audio/audio_buffer.h"
#include "libvoicefeat/config.h"

#include <filesystem>

#include "compat/source_location.h"

namespace libvoicefeat {

    [[nodiscard]] FeatureMatrix compute_file_mfcc(
        const std::filesystem::path& path,
        const CepstralConfig& config = {}
    );

    [[nodiscard]] FeatureMatrix compute_buffer_mfcc(
        const audio::AudioBuffer& audio,
        const CepstralConfig& config = {}
    );

}
