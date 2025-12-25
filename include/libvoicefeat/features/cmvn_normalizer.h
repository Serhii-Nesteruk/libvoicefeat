#pragma once

#include "libvoicefeat/types.h"
#include "libvoicefeat/config.h"

namespace libvoicefeat::features
{
    class CmvnNormalizer
    {
    public:
        explicit CmvnNormalizer(const CepstralConfig& config);
        void apply(FeatureMatrix& feature, const VADFlags* vadFlags = nullptr) const;

    private:
        static bool useFrame(const VADFlags* vad, std::size_t idx, std::size_t total);
        std::vector<double> computeMean(const FeatureMatrix& feature,
                                        const VADFlags* vad) const;
        std::vector<double> computeStd(const FeatureMatrix& feature,
                                       const VADFlags* vad,
                                       const std::vector<double>& mean) const;
        void normalizeFeatures(FeatureMatrix& feature,
                               const std::vector<double>& mean,
                               const std::vector<double>& std) const;

        bool _enabled;

        CmvnType _type;
        CmvnNormMode _mode;
        float _eps{};
    };
}
