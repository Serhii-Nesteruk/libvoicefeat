#pragma once

#include <vector>

namespace libvoicefeat {

    // --------------------------
    // ENUMS
    // --------------------------

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

    // --------------------------
    // SUB-CONFIGS (OPTIONS)
    // --------------------------

    struct FeatureOptions
    {
        int sampleRate                  = 16000;                  // input audio sample rate (Hz)
        int numFilters                  = 26;                     // number of filters in filterbank (mel/linear/gammatone/etc)
        int numCoeffs                   = 13;                     // number of cepstral coefficients to output
        double minFreq                  = 0.0;                    // lower frequency bound (Hz)
        double maxFreq                  = 8000.0;                 // upper frequency bound (Hz), typically Nyquist
        bool includeEnergy              = true;                   // prepend log-energy as c0
        FilterbankType filterbank       = FilterbankType::Mel;    // filterbank type: Mel / Linear / Gammatone / Bark
        MelScale melScale               = MelScale::Slaney;       // mel frequency scale formula (HTK or Slaney)
    };

    struct FramingOptions {
        int frameSize                       = 400;                 // analysis window size (samples per frame)
        int frameStep                       = 160;                 // hop size between frames (samples)
        WindowType window                   = WindowType::Hamming; // window function applied to each frame
    };

    struct DeltaOptions {
        bool useDeltas                      = false;               // compute delta (1st derivative)
        bool useDeltaDeltas                 = false;               // compute delta-delta (2nd derivative)
        int regressionWindow                = 2;                   // N in delta formula (context size)
    };

    struct PreEmphasisOptions {
        bool usePreEmphasis                 = true;                // apply pre-emphasis filter
        float preEmphasisCoeff              = 0.97f;               // pre-emphasis coefficient (typically 0.95–0.97)
    };

    struct CepstralConfig {
        CepstralType type               = CepstralType::MFCC;     // type of cepstral feature (MFCC, LFCC, GFCC, PNCC, PLP)

        FramingOptions framing {};                                 // framing + windowing parameters
        FeatureOptions feature {};                                 // feature-specific spectral/cepstral layout
        DeltaOptions delta {};                                     // delta / delta-delta options
        PreEmphasisOptions preemphasis {};                         // pre-emphasis options
    };


    using FeatureVector = std::vector<float>;
    using FeatureMatrix = std::vector<FeatureVector>;

}