#pragma once

#include "filterbanks/filterbank.h"
#include "libvoicefeat/config.h"
#include "libvoicefeat/audio/audio_buffer.h"
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

        [[nodiscard]] inline FeatureOptions getOptions() const { return _options; }
        [[nodiscard]] inline CepstralType getCepstralType() const { return _cepstralType; }
        [[nodiscard]] inline FeatureMatrix getComputedMatrix() const { return _computed; }

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
        void setCompressionType(CompressionType compressionType);

        void applyPreEmphasis(std::vector<float>& samples, float coeff);

    private:
        void normalizeFrequencyRange();
        void setupFbParams(const int nFft);
        [[nodiscard]] std::vector<double> magnitude(const std::vector<std::complex<float>>& spec, int nFreqs);
        [[nodiscard]] std::vector<double> applyFilterbank(const std::vector<std::vector<double>>& filters,
                                                          const std::vector<double>& mag);
        void applyCompression(std::vector<double>& v, libvoicefeat::CompressionType type);

        void processFrame(const Frame& frame,
                          const std::vector<std::complex<float>>& spec, const int nFreqs);
        void log(std::vector<double>& v);
        void cubeRoot(std::vector<double>& v);
        void powerNormalized(std::vector<double>& v);

        void meanPowerNormalization(std::vector<double>& v);
        void asymmetricNonlinear(std::vector<double>& v);
        void spectralFloor(std::vector<double>& v);
        void dctII(const std::vector<double>& v, int numCoeffs);
        // TODO: Real LPV/PLP implementation
        std::vector<double> plpCepstraPlaceholder(const std::vector<double>& barkEnergies,
                                                  int numCoeffs);

        FeatureOptions _options{};
        CepstralType _cepstralType{CepstralType::MFCC};
        FeatureMatrix _computed{};

        FilterbankParams _fbParams{};
    };
}
