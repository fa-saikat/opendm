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
