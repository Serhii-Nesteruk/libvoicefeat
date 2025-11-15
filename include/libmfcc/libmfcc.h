#pragma once

#include "libmfcc/audio/audio_buffer.h"
#include "libmfcc/config.h"

#include <filesystem>

#include "compat/source_location.h"

namespace libmfcc {

    [[nodiscard]] MfccMatrix compute_file_mfcc(
        const std::filesystem::path& path,
        const MfccConfig& config = {}
    );

    [[nodiscard]] MfccMatrix compute_buffer_mfcc(
        const audio::AudioBuffer& audio,
        const MfccConfig& config = {}
    );

}
