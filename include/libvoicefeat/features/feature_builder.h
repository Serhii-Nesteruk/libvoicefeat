#pragma once

#include "feature.h"

namespace libvoicefeat::features
{
    class FeatureBuilder
    {
    public:
        FeatureBuilder() = default;
        ~FeatureBuilder() = default;

        FeatureBuilder setOptions(const FeatureOptions& options);
        FeatureBuilder setSampleRate(int sampleRate);
        FeatureBuilder setNumFilters(int numFilters);
        FeatureBuilder setNumCoeffs(int numCoeffs);
        FeatureBuilder setMinFreq(double minFreq);
        FeatureBuilder setMaxFreq(double maxFreq);
        FeatureBuilder setIncludeEnergy(bool includeEnergy);
        FeatureBuilder setFBankType(FilterbankType fBankType);
        FeatureBuilder setMelScale(MelScale melScale);
        FeatureBuilder setCepstralType(CepstralType cepstralType);

        [[nodiscard]] Feature build() const;

    private:
        Feature _feature;
    };

    class FeatureDirector
    {
    public:
        [[nodiscard]] static Feature createDefaultMfccFeature(const CepstralConfig& cfg);
        [[nodiscard]] static Feature createDefaultGfccFeature(const CepstralConfig& cfg);
        [[nodiscard]] static Feature createDefaultLfccFeature(const CepstralConfig& cfg);
        [[nodiscard]] static Feature createDefaultPnccFeature(const CepstralConfig& cfg);
        [[nodiscard]] static Feature createDefaultPlpFeature(const CepstralConfig& cfg);
    };

    class FeatureFactory
    {
    public:
        [[nodiscard]] static Feature createDefaultFeature(const CepstralType& type, const CepstralConfig& cfg);
    };
}
