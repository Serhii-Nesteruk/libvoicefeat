#include "libvoicefeat/features/feature.h"

using namespace libvoicefeat::features;
using namespace libvoicefeat::dsp;

libvoicefeat::FeatureMatrix Feature::compute(const std::vector<Frame>& frames,
    const ITransformer& transformer, const FeatureOptions& options)
{
}

void Feature::setOptions(const FeatureOptions& options)
{
    _options = options;
}

void Feature::setSampleRate(int sampleRate)
{
    _options.sampleRate = sampleRate;
}

void Feature::setNumFilters(int numFilters)
{
    _options.numFilters = numFilters;
}

void Feature::setNumCoeffs(int numCoeffs)
{
    _options.numCoeffs = numCoeffs;
}

void Feature::setMinFreq(double minFreq)
{
   _options.minFreq = minFreq
}

void Feature::setMaxFreq(double maxFreq)
{
    _options.maxFreq = maxFreq;
}

void Feature::setIncludeEnergy(bool includeEnergy)
{
    _options.includeEnergy = includeEnergy;
}

void Feature::setFBankType(FilterbankType fBankType)
{
    _options.filterbank = fBankType;
}

void Feature::setMelScale(MelScale melScale)
{
    _options.melScale = melScale;
}

void Feature::setCepstralType(CepstralType cepstralType)
{
    _cepstralType = cepstralType;
}
