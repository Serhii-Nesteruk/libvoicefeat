#include "libvoicefeat/libvoicefeat.h"

#include <iostream>
#include <exception>
#include <filesystem>

int main()
{
    try
    {
        const std::filesystem::path audioPath{"./data/common_voice_en_42698961.mp3"};
        libvoicefeat::CepstralConfig config;
        config.useDeltas = true;
        config.useDeltaDeltas = true;

        auto mfcc = libvoicefeat::compute_file_mfcc(audioPath, config);
        std::cout << "Frames: " << mfcc.size() << std::endl;
        std::cout << "Coefficients per frame: " << (mfcc.empty() ? 0 : mfcc.front().size()) << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to compute MFCC: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
