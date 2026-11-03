# Packaging OpenDM

Every package is built from the same CMake install tree (`cmake --install`), so
binaries, desktop entry, icons, AppStream metadata, UI assets and the browser
native-messaging manifests end up in the same places on every distribution:

| Path | Purpose |
|---|---|
| `/usr/bin/opendm`, `/usr/bin/opendm-native-host` | Application and browser messaging host |
| `/usr/lib/mozilla/native-messaging-hosts/com.tonec.idm.json` | Firefox manifest |
| `/etc/opt/chrome/native-messaging-hosts/com.tonec.idm.json` | Google Chrome manifest |
| `/etc/chromium/native-messaging-hosts/com.tonec.idm.json` | Chromium manifest |
| `/usr/share/opendm/` | Stylesheet, UI assets, browser extension |

The version always comes from `project(OpenDM VERSION X.Y.Z)` in
`CMakeLists.txt` (`scripts/version.sh` prints it). Built packages are written
to `dist/`.

## Debian / Ubuntu (`.deb`)

```bash
sudo apt-get install -y cmake ninja-build g++ pkg-config dpkg-dev \
    libgtk-3-dev libcurl4-openssl-dev zlib1g-dev libssl-dev nlohmann-json3-dev
./scripts/package_deb.sh
sudo apt install ./dist/opendm_*_amd64.deb
```

Runtime dependencies are computed with `dpkg-shlibdeps`, so the package
depends on what the build host actually linked against. Build on the
**oldest** release you want to support: a package built on Ubuntu 22.04
installs on Ubuntu 22.04/24.04 and Debian 12/13.

Optional environment overrides: `DEB_REVISION` (default `1`) and
`DEB_MAINTAINER`.

## Fedora / openSUSE / RHEL-likes (`.rpm`)

```bash
# Fedora
sudo dnf install -y rpm-build git cmake make gcc-c++ gtk3-devel libcurl-devel \
    zlib-devel json-devel desktop-file-utils
# openSUSE
sudo zypper install rpm-build git cmake make gcc-c++ gtk3-devel libcurl-devel \
    zlib-devel nlohmann_json-devel desktop-file-utils

./scripts/package_rpm.sh
sudo dnf install ./dist/opendm-*.x86_64.rpm      # or: sudo zypper install ./dist/...
```

The spec lives in `packaging/rpm/opendm.spec` and uses `pkgconfig(...)` /
`cmake(...)` build requirements, so the same spec resolves on Fedora, openSUSE
and RHEL-likes (EPEL provides `json-devel` there). The script builds a source
tarball from the tracked files, writes a `%changelog` entry for the current
version, and runs `rpmbuild -ba`. The binary and source RPMs are copied to
`dist/`.

`%check` runs `desktop-file-validate` and the test suite. `YouTubeEngineTest`
is skipped because it needs live network access, which build systems such as
mock/Koji/OBS don't provide.

## Arch Linux and derivatives (`.pkg.tar.zst`)

```bash
sudo pacman -S --needed base-devel cmake ninja git gtk3 curl zlib nlohmann-json
./scripts/package_arch.sh          # run as a regular user; makepkg refuses root
sudo pacman -U dist/opendm-*-x86_64.pkg.tar.zst
```

`packaging/arch/PKGBUILD` follows the Arch packaging guidelines
(`CMAKE_BUILD_TYPE=None`, so makepkg's flags apply) and is `namcap`-clean. Its
`source` points at the GitHub tag archive (`v<version>`), as an AUR package
would. `scripts/package_arch.sh` sets `pkgver` from CMake and provides that
tarball locally from the working tree, so no published tag is needed. Extra
arguments are passed to `makepkg` (for example `--syncdeps`).
