#pragma once


#include "audio_reader.h"

#include <filesystem>

namespace libvoicefeat::audio
{
    class Mp3AudioReader : public IAudioReader
    {
    public:
        AudioBuffer load(const std::filesystem::path& inputFile,
                         libvoicefeat::compat::source_location loc = libvoicefeat::compat::source_location::current()) override;
    };
}
