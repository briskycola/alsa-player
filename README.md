# ALSA Audio Player
This is a minimal command-line audio player for Linux written in C++
using **ALSA** and **libsndfile**. This project demonstrates how to
load common audio formats and play them back through the system’s
default ALSA device with basic metadata display.

## Features
- Plays audio files via ALSA
- Supports multiple formats through **libsndfile** (WAV, FLAC, OGG, AIFF, etc.)
- Automatic configuration of:
  - Sample rate
  - Channel count
  - Buffer and period size
- Prints basic metadata:
  - Title
  - Track number
  - Artist
  - Album
- Simple, readable C++ design suitable for learning ALSA basics

## Building
You will need the following libraries installed:
- **ALSA**
- **libsndfile**

### Debian/Ubuntu
```bash
sudo apt install libasound2-dev libsndfile1-dev
```

### Arch Linux
```bash
sudo pacman -S alsa-lib libsndfile
```

### Fedora
```bash
sudo dnf install alsa-lib-devel libsndfile-devel
```

### openSUSE
```bash
sudo zypper install alsa-devel libsndfile-devel
```

### Alpine Linux
```bash
sudo apk add alsa-lib-dev libsndfile-dev
```

### Void Linux
```bash
sudo xbps-install -S alsa-lib-devel libsndfile-devel
```

To build the audio player, simply run
```bash
make
```
