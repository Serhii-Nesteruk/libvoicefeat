# 🎧 libmfcc — Lightweight C++ Library for MFCC Feature Extraction

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)]()
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)]()

**libmfcc** is a lightweight and fast **C++17** library for computing  
🎵 **Mel-Frequency Cepstral Coefficients (MFCC)** — a core feature representation  
used in **voice identification**, **speech recognition**, and general **audio analysis**.

---

## 🚀 Features

- 🔊 **Audio input support**
  - WAV (PCM)
  - MP3 (via [minimp3](https://github.com/lieff/minimp3))
- 🎚 **Framing and windowing** (Hamming)
- 🎛 **Spectral transforms** (STFT / DFT)
- 🎧 **MFCC computation**
  - Mel filterbank
  - Log-scaled energy
  - DCT-II cepstral transformation
- 🧠 Designed for **speaker recognition, ASR, and ML feature pipelines**

---

### 2️⃣ Example
```c++
int main() {
using namespace libmfcc;

    std::filesystem::path inputFile("../../data/common_voice_en_42698961.mp3");
    audio::Mp3AudioReader reader;
    auto audio = reader.load(inputFile);

    dsp::FixedFrameExtractor extractor(400, 160);
    auto frames = extractor.extract(audio);

    dsp::HammingWindow window(400);
    for (auto& f : frames) window.apply(f.data);

    dsp::FFTTransformer fft;
    dsp::DFTTransformer dft;

    auto mfcc_fft = features::computeMFCC(frames, audio.sampleRate, fft);
    auto mfcc_dft = features::computeMFCC(frames, audio.sampleRate, dft);

    std::cout << "FFT MFCC frames: " << mfcc_fft.size() << "\n";
    std::cout << "DFT MFCC frames: " << mfcc_dft.size() << "\n";

    return 0;
}
```

## ⚙️ Audio Precision Notice

Currently, all audio readers in **libmfcc** decode input into **16-bit PCM** format  
(standard `int16_t` samples, normalized to the range **[-1.0, 1.0]**).

This precision is **fully sufficient for speech and voice recognition**,  
because human speech typically occupies frequencies up to ~8 kHz,  
and 16-bit quantization already provides a **96 dB dynamic range**,  
which is well above the signal-to-noise ratio of microphones used in speech datasets  
(e.g., Common Voice, LibriSpeech).

However, 16-bit depth is **limited** for tasks such as:

- 🎶 **Studio-grade audio processing**
- 🎻 **Music transcription or mastering**
- 🧪 **Acoustic analysis requiring >100 dB precision**

Support for **24-bit / 32-bit float** decoding is planned in future releases.

---

## 🧪 Build & Usage

### 1️⃣ Build

```bash
git clone https://github.com/Serhii-Nesteruk/libmfcc.git
cd libmfcc
mkdir build && cd build
cmake ..
make 
