# libvoicefeat

libvoicefeat is a lightweight C++17 library for acoustic feature extraction from speech signals. The library is focused on classical DSP-based voice features and is intended for use in speaker recognition, speaker verification, and ASR-related tasks.

The current implementation provides MFCC feature extraction with common preprocessing and normalization stages and is designed to be used as a frontend component in speech processing pipelines.

---

## Features

### Audio input
- WAV (PCM)
- MP3 (via embedded minimp3 decoder)

### Signal processing
- Framing and Hamming window
- Pre-emphasis
- STFT / DFT

### Cepstral features
- MFCC extraction
- Configurable filterbanks (Mel, Linear, Bark, Gammatone)
- Slaney and HTK Mel scales
- Log energy
- DCT-II

### Preprocessing and normalization
- Voice Activity Detection (VAD)
- Cepstral Mean and Variance Normalization (CMVN)

### Feature extensions
- Delta coefficients
- Delta-Delta coefficients

---

## Example

```cpp
int main()
{
    const std::filesystem::path audioPath{"./audio.wav"};

    CepstralConfig config;
    config.type = CepstralType::MFCC;
    config.delta.useDeltas = true;
    config.delta.useDeltaDeltas = true;

    CepstralExtractor extractor(config);
    auto feature = extractor.extractFromFile(audioPath);
    auto matrix = feature.getComputedMatrix();

    return 0;
}
