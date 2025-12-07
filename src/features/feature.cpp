#include "libvoicefeat/features/feature.h"

#include <algorithm>

#include "libvoicefeat/features/delta.h"
#include "libvoicefeat/utils/constants.h"

using namespace libvoicefeat::features;
using namespace libvoicefeat::dsp;

libvoicefeat::FeatureMatrix Feature::compute(const std::vector<Frame>& frames,
                                             const ITransformer& transformer)
{
    if (frames.empty())
        return _computed;

    _computed.clear();

    _options.numCoeffs = std::max(1, _options.numCoeffs);
    _options.numFilters = std::max(1, _options.numFilters);
    _options.sampleRate = std::max(1, _options.sampleRate);

    auto firstSpectrum = transformer.transform(frames.front().data);
    const int nFft = static_cast<int>(firstSpectrum.size());
    const int nFreqs = nFft / 2 + 1; // number of unique freqs

    normalizeFrequencyRange();
    setupFbParams(nFft);

    const auto fbank = createFilterbank(_options.filterbank, _options.melScale);
    const auto filters = fbank->build(_fbParams);

    for (std::size_t i = 0; i < frames.size(); ++i)
    {
        auto spec = transformer.transform(frames[i].data);
        processFrame(frames[i], spec, filters, nFreqs);
    }

    _computed = appendDeltas(_computed, _useDeltas, _useDelteDeltas);
    return _computed;
}

void Feature::setOptions(const FeatureOptions& options)
{
    _options = options;
}

void Feature::setSampleRate(int sampleRate)
{
    _options.sampleRate = sampleRate;
}

void Feature::setNumFilters(int numFilters)
{
    _options.numFilters = numFilters;
}

void Feature::setNumCoeffs(int numCoeffs)
{
    _options.numCoeffs = numCoeffs;
}

void Feature::setMinFreq(double minFreq)
{
    _options.minFreq = minFreq;
}

void Feature::setMaxFreq(double maxFreq)
{
    _options.maxFreq = maxFreq;
}

void Feature::setIncludeEnergy(bool includeEnergy)
{
    _options.includeEnergy = includeEnergy;
}

void Feature::setFBankType(FilterbankType fBankType)
{
    _options.filterbank = fBankType;
}

void Feature::setMelScale(MelScale melScale)
{
    _options.melScale = melScale;
}

void Feature::setCepstralType(CepstralType cepstralType)
{
    _cepstralType = cepstralType;
}

void Feature::setCompressionType(CompressionType compressionType)
{
    _options.compressionType = compressionType;
}

void Feature::useDeltas(bool use)
{
    _useDeltas = use;
}

void Feature::useDeltaDeltas(bool use)
{
    _useDelteDeltas = use;
}

void Feature::normalizeFrequencyRange()
{
    const double nyquist = static_cast<double>(_options.sampleRate) / 2.0;
    _options.maxFreq = std::clamp(_options.maxFreq <= 0.0 ? nyquist : _options.maxFreq, 0.0, nyquist);
    _options.minFreq = std::clamp(_options.minFreq, 0.0, _options.maxFreq);
    if (_options.maxFreq <= _options.minFreq)
        _options.maxFreq = _options.minFreq + 1.0;
}

void Feature::setupFbParams(const int nFft)
{
    _fbParams.sampleRate = _options.sampleRate;
    _fbParams.nFft = nFft;
    _fbParams.numFilters = _options.numFilters;
    _fbParams.minFreq = _options.minFreq;
    _fbParams.maxFreq = _options.maxFreq;
}

std::vector<double> Feature::magnitude(const std::vector<std::complex<float>>& spec, int nFreqs)
{
    std::vector<double> mag(std::min(static_cast<int>(spec.size()), nFreqs));
    for (std::size_t i = 0; i < mag.size(); ++i)
    {
        mag[i] = std::abs(spec[i]);
    }
    return mag;
}

std::vector<double> Feature::applyFilterbank(const std::vector<std::vector<double>>& filters,
                                             const std::vector<double>& mag)
{
    std::vector<double> out(filters.size(), 0.0);
    for (std::size_t m = 0; m < filters.size(); ++m)
    {
        const auto& f = filters[m];
        const std::size_t N = std::min(f.size(), mag.size());
        double sum = 0.0;
        for (std::size_t k = 0; k < N; ++k)
        {
            sum += mag[k] * f[k];
        }
        out[m] = sum;
    }
    return out;
}

void Feature::applyCompression(std::vector<double>& v, libvoicefeat::CompressionType type)
{
    using libvoicefeat::CompressionType;

    switch (type)
    {
    case CompressionType::Log:
        log(v);
        break;
    case CompressionType::CubeRoot:
        cubeRoot(v);
        break;
    case CompressionType::PowerNormalized:
        powerNormalized(v);
        break;
    default:
        throw std::runtime_error("Unknown compression type");
    }
}

void Feature::processFrame(const Frame& frame,
                           const std::vector<std::complex<float>>& spec,
                           const std::vector<std::vector<double>>& filters, const int nFreqs)
{
    auto mag = magnitude(spec, nFreqs);
    if (static_cast<int>(mag.size()) < nFreqs)
        mag.resize(nFreqs, 0.0);

    auto bandEnergies = applyFilterbank(filters, mag);
    applyCompression(bandEnergies, _options.compressionType);

    std::vector<double> cepstraDouble;

    switch (_cepstralType)
    {
    case CepstralType::MFCC:
    case CepstralType::LFCC:
    case CepstralType::GFCC:
    case CepstralType::PNCC:
        cepstraDouble = dctII(bandEnergies, _options.numCoeffs);
        break;

    case CepstralType::PLP:
        cepstraDouble = plpCepstraPlaceholder(bandEnergies, _options.numCoeffs);
        break;

    default:
        throw std::runtime_error("Unsupported cepstral type");
    }

    FeatureVector coeffs(cepstraDouble.size(), 0.0f);
    for (std::size_t i = 0; i < cepstraDouble.size(); ++i)
    {
        coeffs[i] = static_cast<float>(cepstraDouble[i]);
    }

    if (_options.includeEnergy && !coeffs.empty())
    {
        double energy = 0.0;
        for (float s : frame.data)
        {
            const double v = s;
            energy += v * v;
        }
        const double logEnergy = std::log(energy + constants::K_LOG_EPS);
        coeffs[0] = static_cast<float>(logEnergy);
    }

    _computed.push_back(std::move(coeffs));
}

void Feature::log(std::vector<double>& v)
{
    for (auto& x : v)
        x = std::log(x + constants::K_LOG_EPS);
}

void Feature::cubeRoot(std::vector<double>& v)
{
    for (auto& x : v)
        x = std::cbrt(std::max(x, 0.0));
}

void Feature::powerNormalized(std::vector<double>& v)
{
    meanPowerNormalization(v);

    asymmetricNonlinear(v);

    spectralFloor(v);
}

void Feature::meanPowerNormalization(std::vector<double>& v)
{
    //    y(k) = x(k) / (mean(x) + eps)
    double meanPower = 0.0;
    for (double x : v)
        meanPower += x;

    meanPower /= std::max<size_t>(1, v.size());
    if (meanPower < constants::K_LOG_EPS)
        meanPower = constants::K_LOG_EPS;

    for (auto& x : v)
        x = x / meanPower;
}

void Feature::asymmetricNonlinear(std::vector<double>& v)
{
    //    PNCC uses: f(x) = log(1 + alpha * x)  (alpha ≈ 2-5)
    constexpr double alpha = 5.0;

    for (auto& x : v)
    {
        if (x < 0.0)
            x = 0.0;
        x = std::log(1.0 + alpha * x);
    }
}

void Feature::spectralFloor(std::vector<double>& v)
{
    constexpr double floorVal = -5.0;
    for (auto& x : v)
    {
        if (x < floorVal)
            x = floorVal;
    }
}

std::vector<double> Feature::dctII(const std::vector<double>& v, int numCoeffs)
{
    const int N = static_cast<int>(v.size());
    const int K = std::max(1, std::min(numCoeffs, N));
    std::vector<double> out(K, 0.0);
    for (int k = 0; k < K; ++k)
    {
        double sum = 0.0;
        for (int n = 0; n < N; ++n)
        {
            const double angle = constants::PI * k * (2.0 * n + 1.0) / (2.0 * N);
            sum += v[n] * std::cos(angle);
        }
        out[k] = sum;
    }
    return out;
}

std::vector<double> Feature::plpCepstraPlaceholder(const std::vector<double>& barkEnergies, int numCoeffs)
{
    // TODO: IMPLEMENT REAL PLP
    return dctII(barkEnergies, numCoeffs);
}

void Feature::applyPreEmphasis(std::vector<float>& samples, float coeff)
{
    if (samples.empty())
    {
        return;
    }

    std::vector<float> emphasized(samples.size());
    emphasized[0] = samples[0];
    for (std::size_t i = 1; i < samples.size(); ++i)
    {
        emphasized[i] = samples[i] - coeff * samples[i - 1];
    }
    samples.swap(emphasized);
}
