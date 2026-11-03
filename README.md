# OpenDM — High-Speed Download Accelerator for Linux

<div align="center">

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=for-the-badge&logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![GTK3](https://img.shields.io/badge/GUI-GTK3-green.svg?style=for-the-badge&logo=gtk)](https://www.gtk.org/)
[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-orange.svg?style=for-the-badge)](LICENSE)
[![Author: Nader Mahbub Khan](https://img.shields.io/badge/Author-Nader%20Mahbub%20Khan-blue?style=for-the-badge&logo=github)](https://github.com/nadermkhan)
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-lightgrey.svg?style=for-the-badge&logo=linux)](https://kernel.org)
[![CI](https://img.shields.io/github/actions/workflow/status/fa-saikat/opendm/ci.yml?branch=main&style=for-the-badge&label=CI)](https://github.com/fa-saikat/opendm/actions/workflows/ci.yml)
[![Zero Telemetry](https://img.shields.io/badge/Telemetry-0%25%20(None)-success.svg?style=for-the-badge)](SECURITY.md)

**A high-performance, native C++20 / GTK3 recreation of Internet Download Manager (IDM).**  
*Created by [Nader Mahbub Khan](https://github.com/nadermkhan) ([nadermkhan/opendm](https://github.com/nadermkhan/opendm)).*  
*This repository is a **modified fork**, ported to GTK3 and packaged for major distributions. It is not the official upstream project.*

[About this fork](#about-this-fork) • [Install](#install-packages) • [Screenshots](#screenshots) • [Features](#features) • [Quick Start](#quick-start) • [Browser Integration](#browser-integration) • [Notice](#notice-from-the-original-author) • [License](#license--attribution)

</div>

---

## About this fork

OpenDM was created by **Nader Mahbub Khan** ([nadermkhan/opendm](https://github.com/nadermkhan/opendm)). All credit for the original design, download engine, browser integration and media pipeline goes to him; see [AUTHORS](AUTHORS) and [NOTICE](NOTICE).

This fork, maintained by [FA Saikat](https://github.com/fa-saikat), changes the following:

* **GTK3 user interface**: the GUI was ported from GTK4 to GTK 3 (>= 3.22), so it runs on GTK3-only desktops and distributions.
* **Distribution packages**: `.deb` (Debian/Ubuntu), `.rpm` (Fedora, openSUSE) and Arch Linux packages, built and install-tested in CI. See [docs/PACKAGING.md](docs/PACKAGING.md).
* **Release automation**: GitHub Releases are published only when a `release/X.Y.Z` branch is merged into `main`.
* **Fixes**: native-messaging manifest install paths, an out-of-bounds bug in the MP4 muxer, and missing build-dependency declarations.

## Notice (from the original author)

It currently works only for Linux (Debian) and tested working in Firefox. Not tested in Chrome and contributions are welcomed. Fork it and improve it. ☺️

---

## Install (packages)

Download the package for your distribution from the [Releases](https://github.com/fa-saikat/opendm/releases) page:

```bash
# Ubuntu 22.04+ / Debian 12+ and derivatives
sudo apt install ./opendm_*_amd64.deb
# Fedora
sudo dnf install ./opendm-*.fc*.x86_64.rpm
# openSUSE Tumbleweed
sudo zypper install ./opendm-*.suse.x86_64.rpm
# Arch Linux and derivatives
sudo pacman -U opendm-*-x86_64.pkg.tar.zst
```

Packages install the browser native-messaging manifests system-wide for Firefox, Chrome and Chromium.


---

## Screenshots

<div align="center">

### Main Interface
![OpenDM Main Window](docs/screenshots/main_window.png)

### Download File Info Dialog
![OpenDM Download File Info](docs/screenshots/download_file_info.png)

### Download Progress & Connection Visualizer
![OpenDM Download Progress](docs/screenshots/download_progress.png)

</div>

---

## Features

* **Dynamic Multipart Socket Segmentation**: Splits active downloads dynamically mid-flight up to 32 concurrent threads without connection teardown.
* **Browser Integration**: Native Messaging host compatible with Chromium (Chrome, Brave, Edge) and Firefox.
* **Pre-Download File Info Prompt**: Automatically inspects file size, categorizes destinations, and supports folder synchronization.
* **Remote ZIP Inspection**: Uses HTTP Range queries to read archive central directories and download individual files without fetching the entire archive.
* **Native ISOBMFF Muxer & fMP4 Defragmenter**: Multiplexes separate video and audio DASH streams into fast-start MP4 containers without external re-encoding.
* **Bandwidth Pacing**: Token-bucket speed limiting per session or globally across all downloads.
* **Batch Import & Queue**: Supports IDM `.ef2` batch files, text URL lists, and queue scheduling.
* **Zero Telemetry**: No tracking, no ads, no remote data collection.

---

## Quick Start

### 1. Install Dependencies

**Debian / Ubuntu / Linux Mint / Pop!_OS:**
```bash
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ pkg-config \
    libgtk-3-dev libcurl4-openssl-dev zlib1g-dev libssl-dev \
    nlohmann-json3-dev
```

**Fedora / RHEL:**
```bash
sudo dnf install cmake ninja-build gcc-c++ gtk3-devel libcurl-devel zlib-devel openssl-devel json-devel
```

**Arch Linux:**
```bash
sudo pacman -S cmake ninja gcc gtk3 curl zlib openssl nlohmann-json
```

### 2. Build & Verify

```bash
git clone https://github.com/fa-saikat/opendm.git
cd opendm

cmake -B build -S . -G Ninja
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

### 3. Install Desktop Integration

```bash
./install_desktop.sh
```

This installs:
* Desktop launcher (`opendm.desktop`) and application icons.
* Command-line binaries: `~/.local/bin/opendm` and `~/.local/bin/opendm-native-host`.
* Native Messaging manifests for Chrome, Chromium, Brave, Edge, and Firefox.

---

## Browser Integration

**Installed from a package?** The `.deb`, `.rpm` and Arch packages already register the native messaging host system-wide for Firefox, Google Chrome and Chromium. For Brave or Microsoft Edge, also run `./install_native_host.sh` from a checkout of this repository; it finds the packaged `/usr/bin/opendm-native-host` automatically. Then load the extension (steps 2–5).

**Built from source:**

1. Run `./install_desktop.sh` to install Native Messaging manifests.
2. In your browser (Chrome / Brave / Edge), navigate to `chrome://extensions/`.
3. Enable **Developer mode** (top right toggle).
4. Click **Load unpacked** and select the [`extension_extracted/`](extension_extracted/) directory from this repository.
5. In Firefox, navigate to `about:debugging#/runtime/this-firefox` and select **Load Temporary Add-on** pointing to `extension_extracted/manifest.json`.

---

## Command Line Usage

```bash
# Download with 16 connections
opendm -d https://speed.hetzner.de/100MB.bin -p ~/Downloads -c 16

# Add to queue without starting immediately
opendm -a -d https://speed.hetzner.de/100MB.bin -p ~/Downloads

# Apply speed limit (e.g. 500 KB/s)
opendm -d https://speed.hetzner.de/100MB.bin -l 500

# Import IDM batch file (.ef2)
opendm -i /path/to/downloads.ef2 -s

# Silent / Headless download
opendm -n -d https://example.com/archive.zip -p /tmp
```

---

## Project Structure

```
opendm/
├── src/
│   ├── core/         # ChunkManager, DownloadSession, MediaMuxer, ZipPreviewer
│   ├── ipc/          # Local IPC server and browser Native Messaging host
│   ├── ui/           # GTK3 user interface and custom widgets
│   └── main.cpp      # CLI argument parsing and GTK application entry point
├── tests/            # Automated Catch2/CTest suites
├── extension_extracted/ # Browser WebExtension
└── install_desktop.sh# Desktop installation script
```

---

## Authors

* **Nader Mahbub Khan**: creator and original author
  * GitHub: [@nadermkhan](https://github.com/nadermkhan)
  * Email: muhammadnadermahbubkhan@gmail.com
  * Repository: [https://github.com/nadermkhan/opendm](https://github.com/nadermkhan/opendm)
* **FA Saikat**: maintainer of this fork (GTK3 port, packaging, CI)
  * GitHub: [@fa-saikat](https://github.com/fa-saikat)
  * Repository: [https://github.com/fa-saikat/opendm](https://github.com/fa-saikat/opendm)

---

## License & Attribution

* **License**: Licensed under the [GNU General Public License v3.0 (GPL-3.0)](LICENSE).
* **Attribution (GPL-3.0 Section 7)**: All distributions, copies, and forks must retain the [NOTICE](NOTICE) file and prominently credit the creator:  
  `Created by Nader Mahbub Khan (https://github.com/nadermkhan/opendm)`
* **Disclaimer**: OpenDM is an independent clean-room implementation. It is not affiliated with, endorsed by, or sponsored by Tonec Inc. Internet Download Manager (IDM) is a registered trademark of Tonec Inc.
