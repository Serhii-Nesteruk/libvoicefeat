#pragma once

#include "audio_buffer.h"

#include <filesystem>
#include <source_location>

#include "libvoicefeat/compat/source_location.h"

namespace libvoicefeat::audio
{
    class IAudioReader
    {
    public:
        virtual ~IAudioReader() = default;
        virtual AudioBuffer load(const std::filesystem::path& inputFile,
                                 libvoicefeat::compat::source_location loc = libvoicefeat::compat::source_location::current()) = 0;
    };
}
