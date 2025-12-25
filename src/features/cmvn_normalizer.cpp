#include "libvoicefeat/features/cmvn_normalizer.h"

#include <cmath>
#include <algorithm>

#include "libvoicefeat/utils/constants.h"

namespace libvoicefeat::features
{
    CmvnNormalizer::CmvnNormalizer(const CepstralConfig& config)
        : _enabled(config.cmvn.enabled), _type(config.cmvn.type), _mode(config.cmvn.mode), _eps(constants::EPS)
    {
    }

    bool CmvnNormalizer::useFrame(const VADFlags* vad, std::size_t idx, std::size_t total)
    {
        if (!vad || vad->size() != total)
            return true;
        return (*vad)[idx] == VADState::Speech;
    }

    std::vector<double> CmvnNormalizer::computeMean(const FeatureMatrix& feature,
                                                    const VADFlags* vad) const
    {
        const std::size_t T = feature.size();
        const std::size_t D = feature.front().size();
        std::vector<double> mean(D, 0.0);
        std::size_t count = 0;
        for (std::size_t t = 0; t < T; ++t)
        {
            if (!useFrame(vad, t, T))
                continue;
            ++count;
            const auto& frame = feature[t];
            for (std::size_t d = 0; d < D; ++d)
                mean[d] += static_cast<double>(frame[d]);
        }
        if (count == 0)
        {
            count = T;
            for (const auto& frame : feature)
                for (std::size_t d = 0; d < D; ++d)
                    mean[d] += static_cast<double>(frame[d]);
        }
        for (std::size_t d = 0; d < D; ++d)
            mean[d] /= static_cast<double>(count > 0 ? count : 1);
        return mean;
    }

    std::vector<double> CmvnNormalizer::computeStd(const FeatureMatrix& feature,
                                                   const VADFlags* vad,
                                                   const std::vector<double>& mean) const
    {
        const std::size_t T = feature.size();
        const std::size_t D = feature.front().size();
        std::vector<double> var(D, 0.0);
        std::size_t count = 0;
        for (std::size_t t = 0; t < T; ++t)
        {
            if (!useFrame(vad, t, T))
                continue;
            ++count;
            const auto& frame = feature[t];
            for (std::size_t d = 0; d < D; ++d)
            {
                const double diff = static_cast<double>(frame[d]) - mean[d];
                var[d] += diff * diff;
            }
        }
        if (count == 0)
        {
            count = T;
            for (const auto& frame : feature)
                for (std::size_t d = 0; d < D; ++d)
                {
                    const double diff = static_cast<double>(frame[d]) - mean[d];
                    var[d] += diff * diff;
                }
        }
        for (std::size_t d = 0; d < D; ++d)
        {
            if (count == 0)
            {
                var[d] = 1.0;
            }
            else
            {
                var[d] = std::sqrt(var[d] / static_cast<double>(count));
                if (var[d] < static_cast<double>(_eps))
                    var[d] = 1.0;
            }
        }
        return var;
    }

    void CmvnNormalizer::normalizeFeatures(FeatureMatrix& feature,
                                           const std::vector<double>& mean,
                                           const std::vector<double>& std) const
    {
        const std::size_t T = feature.size();
        const std::size_t D = feature.front().size();
        for (std::size_t t = 0; t < T; ++t)
        {
            auto& frame = feature[t];
            for (std::size_t d = 0; d < D; ++d)
            {
                if (_mode == CmvnNormMode::MeanVar)
                    frame[d] = static_cast<float>((static_cast<double>(frame[d]) - mean[d]) / std[d]);
                else
                    frame[d] = static_cast<float>(static_cast<double>(frame[d]) - mean[d]);
            }
        }
    }

    void CmvnNormalizer::apply(FeatureMatrix& feature, const VADFlags* vadFlags) const
    {
        if (feature.empty() || feature.front().empty())
            return;
        if (!_enabled)
            return;

        switch (_type)
        {
        case CmvnType::Utterance:
            {
                const auto mean = computeMean(feature, vadFlags);
                std::vector<double> stdVals;
                if (_mode == CmvnNormMode::MeanVar)
                    stdVals = computeStd(feature, vadFlags, mean);
                else
                    stdVals.assign(mean.size(), 1.0);
                normalizeFeatures(feature, mean, stdVals);
                break;
            }
        // TODO: other types (e.g. Global, Sliding)
        default:
            break;
        }
    }
}
