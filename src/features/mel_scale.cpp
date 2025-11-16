#include "libvoicefeat/features/mel_scale.h"

#include <cmath>

namespace
{
    constexpr double kLogBase = 6.4;

    double hz_to_mel_htk(double hz)
    {
        return 2595.0 * std::log10(1.0 + hz / 700.0);
    }

    double mel_to_hz_htk(double mel)
    {
        return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0);
    }

    double hz_to_mel_slaney(double hz)
    {
        const double f_sp = 200.0 / 3.0;
        const double min_log_hz = 1000.0;
        const double min_log_mel = min_log_hz / f_sp;
        const double log_step = std::log(kLogBase) / 27.0;

        double mel = hz / f_sp;
        if (hz >= min_log_hz)
        {
            mel = min_log_mel + std::log(hz / min_log_hz) / log_step;
        }
        return mel;
    }

    double mel_to_hz_slaney(double mel)
    {
        const double f_sp = 200.0 / 3.0;
        const double min_log_hz = 1000.0;
        const double min_log_mel = min_log_hz / f_sp;
        const double log_step = std::log(kLogBase) / 27.0;

        double hz = mel * f_sp;
        if (mel >= min_log_mel)
        {
            hz = min_log_hz * std::exp(log_step * (mel - min_log_mel));
        }
        return hz;
    }
}

namespace libvoicefeat::features
{
    double hzToMel(double hz, MelScale scale)
    {
        switch (scale)
        {
        case MelScale::HTK:
            return hz_to_mel_htk(hz);
        case MelScale::Slaney:
        default:
            return hz_to_mel_slaney(hz);
        }
    }

    double melToHz(double mel, MelScale scale)
    {
        switch (scale)
        {
        case MelScale::HTK:
            return mel_to_hz_htk(mel);
        case MelScale::Slaney:
        default:
            return mel_to_hz_slaney(mel);
        }
    }
}