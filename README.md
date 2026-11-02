# QSSTVMania

**QSSTVMania** is a modernized fork of [QSSTV by Johan Maes (ON4QZ)](https://github.com/ON4QZ/QSSTV), a program for receiving and transmitting SSTV and HAMDRM (sometimes called DSSTV). It is compatible with most of MMSSTV and EasyPal.

Maintained by **Greg Cheng — KC3SMW**
🔗 https://github.com/chengmania/QSSTVMania

---

## What's New in QSSTVMania 9.5.12 (April 2026)

### 🎨 UI Refresh
- Full dark flat theme with ham radio green accent color
- Clean monochrome SVG toolbar icons (Start, Stop, Resync, Save, Erase)
- Green-bordered RX image panel with dark background
- Improved settings panel spacing and layout
- Modern splash screen with callsign and fork attribution
- Styled tab bar, menus, scrollbars, and controls

### 🐛 RX Dropout Bug Fix
- Fixed mid-image decode freeze when receiving over FM repeaters
- Increased sync loss tolerance across all sensitivity levels so brief audio dropouts (PTT gaps, repeater tails, FM audio processing) no longer permanently stall the decoder
- Decoder now recovers and continues decoding after short signal interruptions

### 🏷️ Rebranding
- Application renamed to QSSTVMania
- Version bumped to 9.5.12
- Default image/audio directories updated to `~/qsstvmania/`
- Updated About dialog and splash screen with fork attribution

---

## Installation

### Dependencies

For apt based distros you can install dependencies as follows:

```bash
sudo apt install pkg-config g++ libfftw3-dev \
  qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools \
  libqt5svg5-dev libhamlib++-dev libasound2-dev \
  libpulse-dev libopenjp2-7 libopenjp2-7-dev \
  libv4l-dev build-essential
```

### macOS Dependencies

For macOS users, you can install dependencies using Homebrew:

```bash
brew install qt@5 fftw hamlib openjpeg pulseaudio qwt pkg-config
```

Note: You must have PulseAudio running for sound to work:

```bash
brew services start pulseaudio
```

---

## Compile and Install

```bash
mkdir src/build
cd src/build

# For Linux
qmake ..

# For macOS
/opt/homebrew/opt/qt@5/bin/qmake ..

make -j$(nproc)
sudo make install
```

> **Note:** Replace `$(nproc)` with the number of CPU cores you want to use for parallel compilation, e.g. `make -j4`.

---

## Debug Compile

If you have problems compiling the software, please provide:

- Linux Distribution and Version (e.g. Ubuntu 22.04)
- Qt Version (e.g. Qt 5.15.3)
- Full output of the compile process showing the error

To compile with debug symbols, first install the required tools:

```bash
sudo apt-get install doxygen libqwt-qt5-dev
```

Then either open `qsstv.pro` in **QtCreator** as a new project, or run:

```bash
qmake CONFIG+=debug
make -j$(nproc)
```

You can then use an external debugger such as `gdb`.

---

## Original Project

QSSTVMania is forked from **QSSTV** by Johan Maes (ON4QZ).

- Original repository: https://github.com/ON4QZ/QSSTV
- Original documentation: https://www.qsl.net/o/on4qz/qsstv/manual
- HAMDRM software based on RX/TXAMADRM by PA0MBO

---

## License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
