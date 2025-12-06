#pragma once

#include <vector>

#include "../config.h"

namespace libvoicefeat::dsp
{
    class WindowFunction
    {
    public:
        WindowFunction(int size, WindowType type);
        virtual ~WindowFunction() = default;

        void apply(std::vector<float>& frame) const;

    protected:
        std::vector<float> _w;
        int _size;
    };

    class HammingWindow : public WindowFunction
    {
    public:
        explicit HammingWindow(int size);
    };

    class HanningWindow : public WindowFunction
    {
    public:
        explicit HanningWindow(int size);
    };
}
