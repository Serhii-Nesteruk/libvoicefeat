#include "libvoicefeat/features/feature_builder.h"

#include "libvoicefeat/utils/constants.h"

using namespace libvoicefeat::features;

FeatureBuilder FeatureBuilder::setOptions(const FeatureOptions& options)
{
    _feature.setOptions(options);
    return *this;
}

FeatureBuilder FeatureBuilder::setSampleRate(int sampleRate)
{
    _feature.setSampleRate(sampleRate);
    return *this;
}

FeatureBuilder FeatureBuilder::setNumFilters(int numFilters)
{
    _feature.setNumFilters(numFilters);
    return *this;
}

FeatureBuilder FeatureBuilder::setNumCoeffs(int numCoeffs)
{
    _feature.setNumCoeffs(numCoeffs);
    return *this;
}

FeatureBuilder FeatureBuilder::setMinFreq(double minFreq)
{
    _feature.setMinFreq(minFreq);
    return *this;
}

FeatureBuilder FeatureBuilder::setMaxFreq(double maxFreq)
{
    _feature.setMaxFreq(maxFreq);
    return *this;
}

FeatureBuilder FeatureBuilder::setIncludeEnergy(bool includeEnergy)
{
    _feature.setIncludeEnergy(includeEnergy);
    return *this;
}

FeatureBuilder FeatureBuilder::setFBankType(const FilterbankType& fBankType)
{
    _feature.setFBankType(fBankType);
    return *this;
}

FeatureBuilder FeatureBuilder::setMelScale(const MelScale& melScale)
{
    _feature.setMelScale(melScale);
    return *this;
}

FeatureBuilder FeatureBuilder::setCepstralType(const CepstralType& cepstralType)
{
    _feature.setCepstralType(cepstralType);
    return *this;
}

FeatureBuilder FeatureBuilder::setCompressionType(const CompressionType& compressionType)
{
    _feature.setCompressionType(compressionType);
    return *this;
}

FeatureBuilder FeatureBuilder::useDeltas(bool use)
{
    _feature.useDeltas(use);
    return *this;
}

FeatureBuilder FeatureBuilder::useDeltaDeltas(bool use)
{
    _feature.useDeltaDeltas(use);
    return *this;
}

Feature FeatureBuilder::build() const
{
    return _feature;
}

Feature FeatureDirector::createDefaultMfccFeature(const CepstralConfig& cfg)
{
    return FeatureBuilder{}
            .setCepstralType(CepstralType::MFCC)
            .setFBankType(FilterbankType::Mel)
            .setMelScale(MelScale::Slaney)
            .setSampleRate(cfg.feature.sampleRate)
            .setNumFilters(constants::DEFAULT_MFCC_FILTERS_NUM)
            .setNumCoeffs(constants::DEFAULT_MFCC_COEFFS_NUM)
            .setMinFreq(constants::DEFAULT_MFCC_MIN_FREQ)
            .setMaxFreq(cfg.feature.maxFreq)
            .setCompressionType(CompressionType::Log)
            .setIncludeEnergy(cfg.feature.includeEnergy)
            .useDeltas(cfg.delta.useDeltas)
            .useDeltaDeltas(cfg.delta.useDeltaDeltas)
            .build();
}

Feature FeatureDirector::createDefaultGfccFeature(const CepstralConfig& cfg)
{
    return FeatureBuilder{}
            .setCepstralType(CepstralType::GFCC)
            .setFBankType(FilterbankType::Gammatone)
            .setSampleRate(cfg.feature.sampleRate)
            .setNumFilters(constants::DEFAULT_GFCC_FILTERS_NUM)
            .setNumCoeffs(constants::DEFAULT_GFCC_COEFFS_NUM)
            .setMinFreq(constants::DEFAULT_GFCC_MIN_FREQ)
            .setMaxFreq(cfg.feature.maxFreq)
            .setCompressionType(CompressionType::Log)
            .setIncludeEnergy(cfg.feature.includeEnergy)
            .useDeltas(cfg.delta.useDeltas)
            .useDeltaDeltas(cfg.delta.useDeltaDeltas)
            .build();
}

Feature FeatureDirector::createDefaultLfccFeature(const CepstralConfig& cfg)
{
    return FeatureBuilder{}
            .setCepstralType(CepstralType::LFCC)
            .setFBankType(FilterbankType::Linear)
            .setSampleRate(cfg.feature.sampleRate)
            .setNumFilters(constants::DEFAULT_LFCC_FILTERS_NUM)
            .setNumCoeffs(constants::DEFAULT_LFCC_COEFFS_NUM)
            .setMinFreq(constants::DEFAULT_LFCC_MIN_FREQ)
            .setMaxFreq(cfg.feature.maxFreq)
            .setCompressionType(CompressionType::Log)
            .setIncludeEnergy(cfg.feature.includeEnergy)
            .useDeltas(cfg.delta.useDeltas)
            .useDeltaDeltas(cfg.delta.useDeltaDeltas)
            .build();
}

Feature FeatureDirector::createDefaultPnccFeature(const CepstralConfig& cfg)
{
    return FeatureBuilder{}
            .setCepstralType(CepstralType::PNCC)
            .setFBankType(FilterbankType::Mel)
            .setMelScale(MelScale::Slaney)
            .setSampleRate(cfg.feature.sampleRate)
            .setNumFilters(constants::DEFAULT_PNCC_FILTERS_NUM)
            .setNumCoeffs(constants::DEFAULT_PNCC_COEFFS_NUM)
            .setMinFreq(constants::DEFAULT_PNCC_MIN_FREQ)
            .setMaxFreq(cfg.feature.maxFreq)
            .setCompressionType(CompressionType::PowerNormalized)
            .setIncludeEnergy(cfg.feature.includeEnergy)
            .useDeltas(cfg.delta.useDeltas)
            .useDeltaDeltas(cfg.delta.useDeltaDeltas)
            .build();
}

Feature FeatureDirector::createDefaultPlpFeature(const CepstralConfig& cfg)
{
    return FeatureBuilder{}
            .setCepstralType(CepstralType::PLP)
            .setFBankType(FilterbankType::Bark)
            .setSampleRate(cfg.feature.sampleRate)
            .setNumFilters(constants::DEFAULT_PLP_FILTERS_NUM)
            .setNumCoeffs(constants::DEFAULT_PLP_COEFFS_NUM)
            .setMinFreq(constants::DEFAULT_PLP_MIN_FREQ)
            .setMaxFreq(cfg.feature.maxFreq)
            .setCompressionType(CompressionType::CubeRoot)
            .setIncludeEnergy(cfg.feature.includeEnergy)
            .useDeltas(cfg.delta.useDeltas)
            .useDeltaDeltas(cfg.delta.useDeltaDeltas)
            .build();
}

Feature FeatureFactory::createDefaultFeature(const CepstralConfig& cfg)
{
    switch (cfg.type)
    {
    case CepstralType::MFCC: return FeatureDirector::createDefaultMfccFeature(cfg);
    case CepstralType::LFCC: return FeatureDirector::createDefaultLfccFeature(cfg);
    case CepstralType::GFCC: return FeatureDirector::createDefaultGfccFeature(cfg);
    case CepstralType::PNCC: return FeatureDirector::createDefaultPnccFeature(cfg);
    case CepstralType::PLP: return FeatureDirector::createDefaultPlpFeature(cfg);
    default: throw std::runtime_error("Unsupported feature type");
    }
}
