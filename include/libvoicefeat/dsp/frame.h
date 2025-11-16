#pragma once

#include <vector>

namespace libvoicefeat::dsp
{
    struct Frame
    {
        std::vector<float> data;
    };
}
