#pragma once

#include "libvoicefeat/config.h"
#include "libvoicefeat/dsp/frame.h"
#include "libvoicefeat/dsp/transformer.h"

namespace libvoicefeat::features
{
    using namespace libvoicefeat::dsp;

    class Feature
    {
    public:

        [[nodiscard]] FeatureMatrix compute(const std::vector<Frame>& frames,
                       const ITransformer& transformer);

        inline FeatureOptions getOptions() const { return _options; }
        inline CepstralType getCepstralType() const { return _cepstralType; }

        void setOptions(const FeatureOptions& options);
        void setSampleRate(int sampleRate);
        void setNumFilters(int numFilters);
        void setNumCoeffs(int numCoeffs);
        void setMinFreq(double minFreq);
        void setMaxFreq(double maxFreq);
        void setIncludeEnergy(bool includeEnergy);
        void setFBankType(FilterbankType fBankType);
        void setMelScale(MelScale melScale);
        void setCepstralType(CepstralType cepstralType);

    private:
        FeatureOptions _options{};
        CepstralType _cepstralType{CepstralType::MFCC};
    };

}
