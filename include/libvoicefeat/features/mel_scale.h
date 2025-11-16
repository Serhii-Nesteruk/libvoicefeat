#pragma once

#include "libvoicefeat/config.h"

namespace libvoicefeat::features
{
    [[nodiscard]] double hzToMel(double hz, MelScale scale);
    [[nodiscard]] double melToHz(double mel, MelScale scale);
}