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
- 🎚 **Framing and windowing** (Hamming, Hann)
- 🎛 **Spectral transforms** (STFT / DFT)
- 🎧 **MFCC computation**
    - Mel filterbank
    - Log-scaled energy
    - DCT-II cepstral transformation
- 🧠 Designed for **speaker recognition, ASR, and ML feature pipelines**

---

## 🧪 Build & Usage

### 1️⃣ Build

```bash
git clone https://github.com/Serhii-Nesteruk/libmfcc.git
cd libmfcc
mkdir build && cd build
cmake ..
make 
```



### 2️⃣ Example
Temporary empty section

## 📊 MFCC Output

`computeMFCC()` returns a **matrix** of size `N_frames × N_coeffs`,  
where each row represents the cepstral coefficients for one audio frame.

**Example:**
```
Frame 0: [ -5.32, 0.94, 0.82, -0.48, ... ]
Frame 1: [ -4.91, 1.01, 0.73, -0.52, ... ]
...
```

## 🧠 Typical Applications

- 🎙 **Speaker identification**
- 🗣 **Automatic Speech Recognition (ASR)**
- 🧩 **Audio classification / emotion recognition**
- 🧾 **Preprocessing for ML/DL models**

---

## 🪪 License

**libmfcc** is released under the **MIT License**.  
The embedded `minimp3` decoder is distributed under the **Unlicense (Public Domain)**.

See:
- [`LICENSE`](./LICENSE)