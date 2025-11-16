#include "libvoicefeat/audio/mp3_audio_reader.h"

#include <filesystem>

#include "minimp3_ex.h"
#include <stdexcept>

#include "libvoicefeat/utils/path.h"

using libvoicefeat::compat::source_location;
using libvoicefeat::utils::resolve_from_callsite;

namespace libvoicefeat::audio
{
    AudioBuffer Mp3AudioReader::load(const std::filesystem::path& inputFile, source_location loc)
    {
        const auto resolvedPath = resolve_from_callsite(inputFile, loc);
        const std::string path = resolvedPath.string();

        if (path.empty())
            throw std::invalid_argument("path is empty");

        mp3dec_ex_t dec{};
        if (mp3dec_ex_open(&dec, path.c_str(), MP3D_SEEK_TO_SAMPLE) != 0)
        {
            throw std::runtime_error("Cannot open mp3: " + path);
        }

        std::vector<short> pcm16(dec.samples);
        size_t samplesRead = mp3dec_ex_read(&dec, pcm16.data(), dec.samples);
        if (samplesRead == 0)
        {
            mp3dec_ex_close(&dec);
            throw std::runtime_error("Empty mp3: " + path);
        }

        AudioBuffer buf;
        buf.sampleRate = dec.info.hz;
        int channels = dec.info.channels;

        buf.samples.reserve(samplesRead / channels);

        for (size_t i = 0; i < samplesRead; i += channels)
        {
            float mono = 0.f;
            for (int c = 0; c < channels; ++c)
            {
                mono += pcm16[i + c] / 32768.f;
            }
            mono /= channels;
            buf.samples.push_back(mono);
        }

        mp3dec_ex_close(&dec);
        return buf;
    }
}
