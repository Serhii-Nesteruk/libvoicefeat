#pragma once

#include "libvoicefeat/config.h"

#include "features/feature.h"
#include "utils/path.h"

namespace libvoicefeat
{
    using namespace features;
    using namespace audio;
    using namespace utils;
    using namespace dsp;

    class CepstralExtractor
    {
    public:
        explicit CepstralExtractor(const CepstralConfig& config);

        [[nodiscard]] Feature extractFromFile(const std::string& path);
        [[nodiscard]] Feature extractFromAudioBuffer(const AudioBuffer& audio);

    private:
        [[nodiscard]] static AudioBuffer loadAudio(const std::filesystem::path& path);
        static void applyPreEmphasis(std::vector<float>& samples, float coeff);
        void buildOptions(int sampleRate);

        // TODO: void resampleTo();

        CepstralConfig _config{};
        FeatureOptions _options{};
    };
}
