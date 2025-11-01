#include <iostream>
#include <bits/ostream.tcc>

#include "window_functiion.h"
#include "frame_extractor.h"
#include "audio_reader.h"
#include "wav_audio_reader.h"
#include "mp3_audio_reader.h"

int main() {
    auto* reader = new Mp3AudioReader();
    AudioBuffer audio = reader->load("../data/common_voice_en_42698961.mp3");

    FixedFrameExtractor frameExtractor(400, 160);
    auto frames = frameExtractor.extract(audio);

    HammingWindow window(400);
    for (auto& f : frames) {
        window.apply(f.data);

        for (auto& el : f.data)
        {
            std::cout << el << std::endl;
        }

        //  STFT/MFCC
    }

    delete reader;
    // TODO: save frames / debug

    return 0;
}
