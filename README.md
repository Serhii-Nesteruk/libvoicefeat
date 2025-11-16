# 🎧 libvoicefeat — Lightweight C++ Library for Voice Feature Extraction (MFCC, LFCC, GFCC, PNCC-ready)

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)]()
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)]()

**libvoicefeat** is a fast and lightweight **C++17** DSP library focused on
robust extraction of **voice features**, starting with
🎵 **Mel-Frequency Cepstral Coefficients (MFCC)**
and architected for future extensions such as **LFCC**, **GFCC**, **PNCC**, and **PLP**.

This makes it suitable for:

- 🔐 **anti-spoofing / deepfake detection**
- 🧑‍🏫 **speaker verification & identification**
- 🧠 **ASR pipelines**
- 🎤 **general speech feature engineering**

---

## 🚀 Features

### 🎧 Audio Input
- WAV (PCM)
- MP3 (via embedded minimp3)

### 🎚 DSP Processing
- Framing & Hamming window
- Pre-emphasis
- STFT and DFT transformers

### 🎛 Cepstral Features (current)
- MFCC extraction
- Modular filterbanks (Mel, Linear, Bark, Gammatone)
- Slaney & HTK mel scales
- Log energy
- DCT-II

### 🔧 Feature Enhancements
- Δ (Delta) coefficients
- ΔΔ (Delta-Delta) coefficients

---

## 2️⃣ Example

```cpp
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
```

---

## ⚙️ Audio Precision Notice

All audio readers in **libvoicefeat** currently decode input into **16-bit PCM**
(standard int16_t, normalized to [-1.0, 1.0]).

This precision is fully sufficient for:

- speech recognition
- speaker identification
- ASR training
- real-time inference

Because speech occupies 0–8 kHz, and 16-bit PCM provides a
96 dB dynamic range, which exceeds microphone SNR in most datasets.

### Not optimal for:
- studio-grade processing
- high-precision acoustic research
- >24-bit dynamic measurements

Planned: support for 24-bit / 32-bit float WAV & MP3 decoding.

---

## 🧪 Build & Usage

### 1️⃣ Clone & Build

```bash
git clone https://github.com/Serhii-Nesteruk/libvoicefeat.git
cd libvoicefeat
mkdir build && cd build
cmake ..
make -j
```

### 2️⃣ Linking from your project

```cmake
find_package(libvoicefeat REQUIRED)
target_link_libraries(your_app PRIVATE libvoicefeat::libvoicefeat)
```

---

## 📄 License
MIT — free for commercial and research use.