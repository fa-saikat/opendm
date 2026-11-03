#!/usr/bin/env bash
# Builds an Arch Linux package (.pkg.tar.zst) with makepkg.
#
# Build dependencies (Arch): base-devel cmake ninja git gtk3 curl zlib nlohmann-json
# makepkg refuses to run as root; run this script as a regular user.
#
# Output: dist/opendm-<version>-<pkgrel>-<arch>.pkg.tar.zst
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

VERSION="$("${ROOT}/scripts/version.sh")"
WORK="${ROOT}/build/arch"
DIST_DIR="${ROOT}/dist"

echo "=== Packaging OpenDM ${VERSION} for Arch Linux ==="

rm -rf "${WORK}"
mkdir -p "${WORK}" "${DIST_DIR}"

# PKGBUILD with pkgver taken from CMake
sed -E "s/^pkgver=.*/pkgver=${VERSION}/" packaging/arch/PKGBUILD > "${WORK}/PKGBUILD"

# Local source tarball named exactly as the PKGBUILD source entry, so makepkg
# uses it instead of downloading the release tag.
if git -C "${ROOT}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    git -C "${ROOT}" ls-files -z \
        | tar --null -T - --transform "s,^,opendm-${VERSION}/," -czf "${WORK}/opendm-${VERSION}.tar.gz"
else
    tar --exclude=./build --exclude=./dist --exclude=./.git \
        --transform "s,^\./,opendm-${VERSION}/," -czf "${WORK}/opendm-${VERSION}.tar.gz" .
fi

cd "${WORK}"
PKGDEST="${DIST_DIR}" makepkg --cleanbuild --force --noconfirm "$@"
