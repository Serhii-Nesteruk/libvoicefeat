#pragma once

#include "audio_reader.h"

#include <cstdint>
#include <filesystem>

namespace libvoicefeat::audio
{
#pragma pack(push, 1)
    struct RiffHeader
    {
        char riff[4];
        uint32_t chunkSize;
        char wave[4];
    };

    struct ChunkHeader
    {
        char id[4];
        uint32_t size;
    };

    struct FmtChunk
    {
        uint16_t audioFormat;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
    };
#pragma pack(pop)

    class WavAudioReader : public IAudioReader
    {
    public:
        AudioBuffer load(const std::filesystem::path& inputFile,
                         libvoicefeat::compat::source_location loc = libvoicefeat::compat::source_location::current()) override;
    };
}
