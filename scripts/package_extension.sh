#!/usr/bin/env bash
# Zips the OpenDM browser extension (browser-extension/) for Chrome and Firefox.
#
# The extension version must match project(VERSION) in CMakeLists.txt, so the
# extension and the native host it talks to are always released together.
#
# Requires: zip
# Output: dist/opendm-browser-extension-<version>.zip
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

VERSION="$("${ROOT}/scripts/version.sh")"
EXT_VERSION="$(sed -nE 's/^ *"version" *: *"([^"]+)".*/\1/p' browser-extension/manifest.json | head -n1)"

if [ "${EXT_VERSION}" != "${VERSION}" ]; then
    echo "error: browser-extension/manifest.json has version ${EXT_VERSION:-<none>}," >&2
    echo "       but CMakeLists.txt declares ${VERSION}. Bump both together." >&2
    exit 1
fi

mkdir -p dist
OUT="${ROOT}/dist/opendm-browser-extension-${VERSION}.zip"
rm -f "${OUT}"
(cd browser-extension && zip -q -X -r "${OUT}" . -x '.*')
echo "Wrote ${OUT}"
