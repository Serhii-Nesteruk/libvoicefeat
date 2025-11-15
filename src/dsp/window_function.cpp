#include "libmfcc/dsp/window_functiion.h"

#include <algorithm>
#include <cmath>

using namespace libmfcc::dsp;

static std::vector<float> buildWindow(int size, WindowType type)
{
    std::vector<float> w(size);

    switch (type) {
    case WindowType::Hamming:
        for (int n = 0; n < size; ++n)
            w[n] = 0.54f - 0.46f * std::cos(2.f * M_PI * n / (size - 1));
        break;

    case WindowType::Hanning:
        for (int n = 0; n < size; ++n)
            w[n] = 0.5f - 0.5f * std::cos(2.f * M_PI * n / (size - 1));
        break;
    }

    return w;
}

WindowFunction::WindowFunction(int size, WindowType type)
    : _w(buildWindow(size, type))
    , _size(size)
{
}

void WindowFunction::apply(std::vector<float>& frame) const
{
    const auto N = std::min(frame.size(), _w.size());
    for (std::size_t i = 0; i < N; ++i)
        frame[i] *= _w[i];
}

HammingWindow::HammingWindow(int size)
    : WindowFunction(size, WindowType::Hamming)
{
}

HanningWindow::HanningWindow(int size)
    : WindowFunction(size, WindowType::Hanning)
{
}
