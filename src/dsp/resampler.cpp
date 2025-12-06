#include "libvoicefeat/dsp/resampler.h"

#include <stdexcept>
#include <samplerate.h>
#include <string>

namespace libvoicefeat::dsp
{
    audio::AudioBuffer Resampler::resampleTo(const audio::AudioBuffer& in, int targetSampleRate)
    {
        if (targetSampleRate <= 0) {
            throw std::invalid_argument("targetSampleRate must be positive");
        }

        if (in.sampleRate <= 0) {
            throw std::invalid_argument("input sampleRate must be positive");
        }

        if (in.sampleRate == targetSampleRate) {
            return in;
        }

        if (in.samples.empty()) {
            audio::AudioBuffer out;
            out.sampleRate = targetSampleRate;
            return out;
        }

        const int channels = 1; // mono
        const double ratio = static_cast<double>(targetSampleRate) /
                             static_cast<double>(in.sampleRate);

        const long inputFrames  = static_cast<long>(in.samples.size());       // frame == sample for mono
        const long outputFrames = static_cast<long>(inputFrames * ratio) + 8; // small safety margin

        audio::AudioBuffer out;
        out.sampleRate = targetSampleRate;
        out.samples.resize(outputFrames);

        SRC_DATA data{};
        data.data_in       = in.samples.data();
        data.input_frames  = inputFrames;
        data.data_out      = out.samples.data();
        data.output_frames = outputFrames;
        data.src_ratio     = ratio;
        data.end_of_input  = 1; // all audio is provided at once

        const int converterType = SRC_SINC_MEDIUM_QUALITY;

        int error = src_simple(&data, converterType, channels);
        if (error != 0) {
            throw std::runtime_error(
                std::string("libsamplerate src_simple failed: ") + src_strerror(error));
        }

        out.samples.resize(static_cast<size_t>(data.output_frames_gen));
        return out;
    }
}
