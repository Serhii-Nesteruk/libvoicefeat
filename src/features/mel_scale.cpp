#include "libvoicefeat/features/mel_scale.h"

#include <cmath>

namespace
{
    constexpr double kLogBase = 6.4;

    double hzToMelHtk(double hz)
    {
        return 2595.0 * std::log10(1.0 + hz / 700.0);
    }

    double melToHzHtk(double mel)
    {
        return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0);
    }

    double hzToMelSlaney(double hz)
    {
        const double fSp = 200.0 / 3.0;
        const double minLogHz = 1000.0;
        const double minLogMel = minLogHz / fSp;
        const double logStep = std::log(kLogBase) / 27.0;

        double mel = hz / fSp;
        if (hz >= minLogHz)
        {
            mel = minLogMel + std::log(hz / minLogHz) / logStep;
        }
        return mel;
    }

    double melToHzSlaney(double mel)
    {
        const double fSp = 200.0 / 3.0;
        const double minLogHz = 1000.0;
        const double minLogMel = minLogHz / fSp;
        const double logStep = std::log(kLogBase) / 27.0;

        double hz = mel * fSp;
        if (mel >= minLogMel)
        {
            hz = minLogHz * std::exp(logStep * (mel - minLogMel));
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
            return hzToMelHtk(hz);
        case MelScale::Slaney:
        default:
            return hzToMelSlaney(hz);
        }
    }

    double melToHz(double mel, MelScale scale)
    {
        switch (scale)
        {
        case MelScale::HTK:
            return melToHzHtk(mel);
        case MelScale::Slaney:
        default:
            return melToHzSlaney(mel);
        }
    }
}
