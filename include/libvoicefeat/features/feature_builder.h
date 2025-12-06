#pragma once

#include "feature.h"

namespace libvoicefeat::features
{
    class FeatureBuilder
    {
    public:
        FeatureBuilder() = default;
        ~FeatureBuilder() = default;

        [[nodiscard]] FeatureBuilder setOptions(const FeatureOptions& options);
        [[nodiscard]] FeatureBuilder setSampleRate(int sampleRate);
        [[nodiscard]] FeatureBuilder setNumFilters(int numFilters);
        [[nodiscard]] FeatureBuilder setNumCoeffs(int numCoeffs);
        [[nodiscard]] FeatureBuilder setMinFreq(double minFreq);
        [[nodiscard]] FeatureBuilder setMaxFreq(double maxFreq);
        [[nodiscard]] FeatureBuilder setIncludeEnergy(bool includeEnergy);
        [[nodiscard]] FeatureBuilder setFBankType(const FilterbankType& fBankType);
        [[nodiscard]] FeatureBuilder setMelScale(const MelScale& melScale);
        [[nodiscard]] FeatureBuilder setCepstralType(const CepstralType& cepstralType);
        [[nodiscard]] FeatureBuilder setCompressionType(const CompressionType& compressionType);
        [[nodiscard]] FeatureBuilder useDeltas(bool use);
        [[nodiscard]] FeatureBuilder useDeltaDeltas(bool use);

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
        [[nodiscard]] static Feature createDefaultFeature(const CepstralConfig& cfg);
    };
}
