#pragma once

#include <vector>

namespace libvoicefeat {

    enum class CepstralType {
        MFCC,
        LFCC,
        GFCC,
        PNCC,
        PLP
    };

    enum class FilterbankType {
        Mel,
        Linear,
        Gammatone,
        Bark
    };

    enum class WindowType {
        Hamming,
        Hanning
    };

    enum class MelScale {
        HTK,
        Slaney
    };

    struct FeatureOptions
    {
        int sampleRate                      = 16000;
        int numFilters                      = 26;
        int numCoeffs                       = 13;
        double minFreq                      = 0.0;
        double maxFreq                      = 8000.0;
        bool includeEnergy                  = true;
        FilterbankType filterbank           = FilterbankType::Mel;
        MelScale melScale                   = MelScale::Slaney;
    };

    struct CepstralConfig {
        CepstralType type               = CepstralType::MFCC;     // type of cepstral feature (MFCC, LFCC, GFCC, PNCC, PLP)

        int sampleRate                  = 16000;                  // input audio sample rate (Hz)
        int frameSize                   = 400;                    // analysis window size (samples)
        int frameStep                   = 160;                    // hop size between frames (samples)

        int numCoeffs                   = 13;                     // number of cepstral coefficients to output
        int numFilters                  = 26;                     // number of filters in filterbank (mel/linear/gammatone/etc)

        float minFreq                   = 0.0f;                   // lower frequency bound (Hz)
        float maxFreq                   = 8000.0f;                // upper frequency bound (Hz), typically Nyquist

        WindowType window               = WindowType::Hamming;    // window function applied to each frame
        FilterbankType filterbank       = FilterbankType::Mel;    // filterbank type: Mel / Linear / Gammatone / Bark
        MelScale melScale               = MelScale::Slaney;       // mel frequency scale formula (HTK or Slaney)

        bool includeEnergy              = true;                   // prepend log-energy as c0
        bool useDeltas                  = false;                  // compute delta (1st derivative)
        bool useDeltaDeltas             = false;                  // compute delta-delta (2nd derivative)

        bool usePreEmphasis             = true;                   // apply pre-emphasis filter
        float preEmphasisCoeff          = 0.97f;                  // pre-emphasis coefficient (typically 0.95–0.97)
    };


    using FeatureVector = std::vector<float>;
    using FeatureMatrix = std::vector<FeatureVector>;

}