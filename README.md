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
int main()
{
    try
    {
        const std::filesystem::path audioPath{"./data/common_voice_en_42698961.mp3"};
        libmfcc::MfccConfig config;
        config.useDeltas = true;
        config.useDeltaDeltas = true;

        auto mfcc = libmfcc::compute_file_mfcc(audioPath, config);
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
