#include "libvoicefeat/audio/mp3_audio_reader.h"
#include "libvoicefeat/audio/wav_audio_reader.h"
#include "libvoicefeat/utils/path.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    constexpr float kTolerance = 1e-6f;

    int fail(const std::string& message)
    {
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    bool approximatelyEqual(float a, float b)
    {
        return std::fabs(a - b) <= kTolerance;
    }

    template <typename T>
    void writeValue(std::ofstream& out, T value)
    {
        out.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    void writeFourCc(std::ofstream& out, const char id[4])
    {
        out.write(id, 4);
    }

    std::filesystem::path tempPath(const std::string& name)
    {
        return std::filesystem::temp_directory_path() / name;
    }

    void writePcmWav(const std::filesystem::path& path,
                     uint16_t channels,
                     uint32_t sampleRate,
                     uint16_t bitsPerSample,
                     const std::vector<int16_t>& pcm16)
    {
        std::ofstream out(path, std::ios::binary);
        const uint32_t fmtSize = 16;
        const uint32_t dataSize = static_cast<uint32_t>(pcm16.size() * sizeof(int16_t));
        const uint32_t riffSize = 4 + 8 + fmtSize + 8 + dataSize;
        const uint16_t audioFormat = 1;
        const uint16_t blockAlign = static_cast<uint16_t>(channels * bitsPerSample / 8);
        const uint32_t byteRate = sampleRate * blockAlign;

        writeFourCc(out, "RIFF");
        writeValue(out, riffSize);
        writeFourCc(out, "WAVE");
        writeFourCc(out, "fmt ");
        writeValue(out, fmtSize);
        writeValue(out, audioFormat);
        writeValue(out, channels);
        writeValue(out, sampleRate);
        writeValue(out, byteRate);
        writeValue(out, blockAlign);
        writeValue(out, bitsPerSample);
        writeFourCc(out, "data");
        writeValue(out, dataSize);
        out.write(reinterpret_cast<const char*>(pcm16.data()), static_cast<std::streamsize>(dataSize));
    }

    void writeInvalidFile(const std::filesystem::path& path)
    {
        std::ofstream out(path, std::ios::binary);
        out << "not a wav";
    }
}

int main()
{
    using namespace libvoicefeat;
    using namespace libvoicefeat::audio;

    // -----------------------------
    // WAV reader converts 16-bit stereo PCM to normalized mono.
    // -----------------------------
    {
        const auto path = tempPath("libvoicefeat_audio_io_stereo.wav");
        const std::vector<int16_t> interleaved{
            32767, -32768,
            16384, 0,
            -16384, 16384,
        };
        writePcmWav(path, 2, 16000, 16, interleaved);

        WavAudioReader reader;
        const auto audio = reader.load(path);
        if (audio.sampleRate != 16000)
            return fail("WAV sample rate mismatch");
        if (audio.samples.size() != 3)
            return fail("WAV mono sample count mismatch");

        const std::vector<float> expected{
            (32767.0f / 32768.0f - 1.0f) / 2.0f,
            (0.5f + 0.0f) / 2.0f,
            (-0.5f + 0.5f) / 2.0f,
        };

        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (!approximatelyEqual(audio.samples[i], expected[i]))
                return fail("WAV mono conversion mismatch");
        }
    }

    // -----------------------------
    // WAV reader reports invalid containers and unsupported bit depth.
    // -----------------------------
    {
        const auto invalidPath = tempPath("libvoicefeat_audio_io_invalid.wav");
        writeInvalidFile(invalidPath);

        WavAudioReader reader;
        bool threwInvalid = false;
        try
        {
            (void)reader.load(invalidPath);
        }
        catch (const std::runtime_error&)
        {
            threwInvalid = true;
        }
        if (!threwInvalid)
            return fail("Invalid WAV did not throw");

        const auto eightBitPath = tempPath("libvoicefeat_audio_io_8bit.wav");
        writePcmWav(eightBitPath, 1, 8000, 8, {0, 1, 2, 3});

        bool threwBitDepth = false;
        try
        {
            (void)reader.load(eightBitPath);
        }
        catch (const std::runtime_error&)
        {
            threwBitDepth = true;
        }
        if (!threwBitDepth)
            return fail("Unsupported WAV bit depth did not throw");
    }

    // -----------------------------
    // MP3 reader loads the repository fixture and normalizes samples.
    // -----------------------------
    {
        const auto fixture = utils::resolve_from_callsite("data/common_voice_en_42698961.mp3");
        if (!std::filesystem::exists(fixture))
            return fail("MP3 fixture is missing");

        Mp3AudioReader reader;
        const auto audio = reader.load(fixture);
        if (audio.sampleRate <= 0 || audio.samples.empty())
            return fail("MP3 reader returned empty audio");

        for (float sample : audio.samples)
        {
            if (!std::isfinite(sample))
                return fail("MP3 reader returned non-finite sample");
            if (sample < -1.0f || sample > 1.0f)
                return fail("MP3 reader returned sample outside normalized range");
        }
    }

    // -----------------------------
    // Path resolver keeps absolute paths absolute and finds repo-relative files.
    // -----------------------------
    {
        const auto readme = utils::resolve_from_callsite("README.md");
        if (!std::filesystem::exists(readme) || readme.filename() != "README.md")
            return fail("Repo-relative path resolution failed");

        const auto absolute = utils::resolve_from_callsite(readme);
        if (absolute != std::filesystem::weakly_canonical(readme))
            return fail("Absolute path resolution changed the target");
    }

    return EXIT_SUCCESS;
}
