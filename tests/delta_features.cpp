#include "libvoicefeat/features/delta.h"
#include "libvoicefeat/features/feature.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
    constexpr float kTolerance = 1e-5f;

    bool approximatelyEqual(float a, float b)
    {
        return std::fabs(a - b) < kTolerance;
    }
}

int main()
{
    using namespace libvoicefeat;

    // -----------------------------
    // Delta and delta-delta follow linear growth in the feature matrix
    // -----------------------------
    {
        FeatureMatrix base{{0.f, 0.f}, {1.f, 1.f}, {2.f, 2.f}, {3.f, 3.f}};
        const std::vector<float> expectedDelta{0.5f, 1.0f, 1.0f, 0.5f};
        const std::vector<float> expectedDeltaDelta{0.25f, 0.25f, -0.25f, -0.25f};

        auto delta = features::computeDelta(base, 1);
        auto deltaDelta = features::computeDeltaDelta(base, 1);

        if (delta.size() != base.size() || deltaDelta.size() != base.size())
        {
            std::cerr << "Delta output size mismatch" << std::endl;
            return EXIT_FAILURE;
        }

        for (std::size_t t = 0; t < base.size(); ++t)
        {
            for (float v : delta[t])
            {
                if (!approximatelyEqual(v, expectedDelta[t]))
                {
                    std::cerr << "Unexpected delta value at frame " << t << std::endl;
                    return EXIT_FAILURE;
                }
            }
            for (float v : deltaDelta[t])
            {
                if (!approximatelyEqual(v, expectedDeltaDelta[t]))
                {
                    std::cerr << "Unexpected delta-delta value at frame " << t << std::endl;
                    return EXIT_FAILURE;
                }
            }
        }
    }

    // -----------------------------
    // Pre-emphasis helper mirrors algorithm used in the extractor
    // -----------------------------
    {
        features::Feature feature;
        std::vector<float> samples{1.f, 2.f, 3.f, 4.f};
        feature.applyPreEmphasis(samples, 0.97f);

        std::vector<float> expected{1.f, 2.f - 0.97f * 1.f, 3.f - 0.97f * 2.f, 4.f - 0.97f * 3.f};

        for (std::size_t i = 0; i < samples.size(); ++i)
        {
            if (!approximatelyEqual(samples[i], expected[i]))
            {
                std::cerr << "Pre-emphasis mismatch at index " << i << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    // -----------------------------
    // Append deltas concatenates all tracks in order
    // -----------------------------
    {
        FeatureMatrix base{{1.f, 2.f}, {2.f, 3.f}};
        const bool useDelta = true;
        const bool useDeltaDelta = true;
        auto full = features::appendDeltas(base, useDelta, useDeltaDelta, 1);

        const std::size_t expectedWidth = base.front().size() * (1 + useDelta + useDeltaDelta);
        if (full.front().size() != expectedWidth)
        {
            std::cerr << "Unexpected concatenated width" << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}