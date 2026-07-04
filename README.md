# Yuna

**Yuna** adalah aplikasi *image enhancement* dan *super resolution* yang dibangun menggunakan C++. Aplikasi ini menggabungkan algoritma **Lanczos** untuk interpolasi gambar dengan metode **PDE-based Super Resolution** sebagai *post-processing* untuk menghasilkan gambar berkualitas tinggi.


## Fitur Utama

- **Lanczos Interpolation** — Algoritma interpolasi gambar yang menghasilkan *upscaling* dengan kualitas tinggi dan detail yang tajam.
- **PDE-based Super Resolution** — Integrasi *Partial Differential Equation* (PDE) sebagai *post-processing* untuk meningkatkan resolusi gambar secara lebih lanjut.
- **PSNR Evaluation** — Dilengkapi dengan evaluator PSNR (*Peak Signal-to-Noise Ratio*) untuk mengukur kemiripan antara gambar hasil *enhancement* dengan gambar asli.
- **GUI (Windows API)** — Antarmuka grafis yang dibangun menggunakan Windows API, bermigrasi dari versi CLI sebelumnya.
- **Installer** — Tersedia installer berbasis Inno Setup untuk kemudahan distribusi dan instalasi.
- **Custom Icon & Branding** — Ikon Yuna tertanam langsung di dalam *executable*.

## Tech Stack

- **C++**
- **C**
- **Cmake**
- **Inno Setup**


# Development Guide

### Prasyarat
- **CMake** (versi 3.x atau lebih baru)
- **C++ Compiler** yang mendukung C++20 (MSVC, GCC, atau Clang)
- **Windows SDK** (untuk GUI berbasis Windows API) 

```bash
# Clone repository
git clone -b development https://github.com/kayabaakihiko13/Yuna.git
cd Yuna

# Buat build directory
mkdir build && cd build

# Generate build files dengan CMake
cmake ..

# Build project
cmake --build .
```

# Evaluasi Gambar

Project ini menggunakan PSNR (Peak Signal-to-Noise Ratio) sebagai metrik untuk mengukur kualitas gambar hasil enhancement. Semakin tinggi nilai PSNR, semakin mirip gambar hasil processing dengan gambar asli.

```
PSNR = 20 × log₁₀(MAX_I / √MSE)
```

Dimana:
- MAX_I = Nilai piksel maksimum
- MSE = Mean Squared Error antara gambar asli dan gambar hasil processing

## Author

Iqbal Ramadhan Anniswa 