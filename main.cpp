#include "libvoicefeat/libvoicefeat.h"

#include <iostream>
#include <exception>
#include <filesystem>

#include "libvoicefeat/features/feature_builder.h"

using namespace libvoicefeat::features;
using namespace libvoicefeat;

void showMfccMatrix(const libvoicefeat::FeatureMatrix& matrix)
{
    for (auto& row : matrix)
    {
        for (auto& col : row)
        {
            std::cout << col << " ";
        }
        std::cout << std::endl;
    }
}

int main()
{
    try
    {
        const std::filesystem::path audioPath{"./data/common_voice_en_42698961.wav"};

        CepstralConfig config;
        config.delta.useDeltas = true;
        config.delta.useDeltaDeltas = true;
        config.type = CepstralType::MFCC;

        CepstralExtractor extractor(config);

        auto mfccFeature = extractor.extractFromFile(audioPath);
        auto mfccMatrix = mfccFeature.getComputedMatrix();
        std::cout << "Frames: " << mfccMatrix.size() << std::endl;
        std::cout << "Coefficients per frame: " << (mfccMatrix.empty() ? 0 : mfccMatrix.front().size()) << std::endl;

        // showMfccMatrix(mfcc);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to compute MFCC: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
