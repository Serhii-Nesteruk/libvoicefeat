#include "libvoicefeat/features/filterbanks/filterbank.h"

#include "libvoicefeat/features/filterbanks/bark_filterbank.h"
#include "libvoicefeat/features/filterbanks/gammatone_filterbank.h"
#include "libvoicefeat/features/filterbanks/linear_filterbank.h"
#include "libvoicefeat/features/filterbanks/mel_filterbank.h"

#include <stdexcept>

namespace libvoicefeat::features
{
    std::unique_ptr<IFilterbank> createFilterbank(FilterbankType type, MelScale melScale)
    {
        switch (type)
        {
        case FilterbankType::Mel:
            return std::make_unique<MelFilterbank>(melScale);
        case FilterbankType::Linear:
            return std::make_unique<LinearFilterbank>();
        case FilterbankType::Gammatone:
            return std::make_unique<GammatoneFilterbank>();
        case FilterbankType::Bark:
            return std::make_unique<BarkFilterbank>();
        default:
            throw std::invalid_argument("Unsupported filterbank type");
        }
    }
}