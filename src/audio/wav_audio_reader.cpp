#include "libvoicefeat/audio/wav_audio_reader.h"

#include "stdexcept"
#include "fstream"
#include "libvoicefeat/utils/path.h"

using libvoicefeat::compat::source_location;
using libvoicefeat::utils::resolve_from_callsite;

namespace libvoicefeat::audio
{
    static bool fourcc_eq(const char id[4], const char* s)
    {
        return id[0] == s[0] && id[1] == s[1] && id[2] == s[2] && id[3] == s[3];
    }

    AudioBuffer WavAudioReader::load(const std::filesystem::path& inputFile, source_location loc)
    {
        const auto resolvedPath = resolve_from_callsite(inputFile, loc);
        const std::string path = resolvedPath.string();

        std::ifstream in(path, std::ios::binary);
        if (!in)
            throw std::runtime_error("Cannot open wav file: " + path);

        RiffHeader riff{};
        in.read(reinterpret_cast<char*>(&riff), sizeof(riff));
        if (!fourcc_eq(riff.riff, "RIFF") || !fourcc_eq(riff.wave, "WAVE"))
            throw std::runtime_error("Not a RIFF/WAVE file: " + path);

        uint16_t numChannels = 0;
        uint32_t sampleRate = 0;
        uint16_t bitsPerSample = 0;

        std::vector<uint8_t> dataChunk;

        while (in)
        {
            ChunkHeader ch{};
            in.read(reinterpret_cast<char*>(&ch), sizeof(ch));
            if (!in) break;

            if (fourcc_eq(ch.id, "fmt "))
            {
                FmtChunk fmt{};
                in.read(reinterpret_cast<char*>(&fmt), sizeof(FmtChunk));

                numChannels = fmt.numChannels;
                sampleRate = fmt.sampleRate;
                bitsPerSample = fmt.bitsPerSample;

                uint32_t fmtExtra = ch.size - sizeof(FmtChunk);
                if (fmtExtra > 0)
                {
                    in.seekg(fmtExtra, std::ios::cur);
                }
            }
            else if (fourcc_eq(ch.id, "data"))
            {
                dataChunk.resize(ch.size);
                in.read(reinterpret_cast<char*>(dataChunk.data()), ch.size);
                break;
            }
            else
            {
                in.seekg(ch.size, std::ios::cur);
            }
        }

        if (dataChunk.empty())
            throw std::runtime_error("No data chunk in wav: " + path);

        if (bitsPerSample != 16)
            throw std::runtime_error("Only 16-bit wav supported in this reader");

        const size_t numSamples = dataChunk.size() / (bitsPerSample / 8) / numChannels;
        const int16_t* pcm16 = reinterpret_cast<const int16_t*>(dataChunk.data());

        AudioBuffer buf;
        buf.sampleRate = static_cast<int>(sampleRate);
        buf.samples.reserve(numSamples);

        for (size_t i = 0; i < numSamples; ++i)
        {
            float mono = 0.f;
            for (int c = 0; c < numChannels; ++c)
            {
                mono += pcm16[i * numChannels + c] / 32768.f;
            }
            mono /= numChannels;
            buf.samples.push_back(mono);
        }

        return buf;
    }
}
