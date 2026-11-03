#!/usr/bin/env bash
# Builds a binary .deb for Debian / Ubuntu and derivatives.
#
# Build dependencies (Debian/Ubuntu):
#   cmake ninja-build g++ pkg-config dpkg-dev libgtk-3-dev
#   libcurl4-openssl-dev zlib1g-dev libssl-dev nlohmann-json3-dev
#
# Output: dist/opendm_<version>-<revision>_<arch>.deb
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

PACKAGE_NAME="opendm"
UPSTREAM_VERSION="$("${ROOT}/scripts/version.sh")"
DEB_REVISION="${DEB_REVISION:-1}"
VERSION="${UPSTREAM_VERSION}-${DEB_REVISION}"
ARCH="$(dpkg --print-architecture)"
MAINTAINER="${DEB_MAINTAINER:-FA Saikat <fahimabrar.saikat@gmail.com>}"
HOMEPAGE="https://github.com/fa-saikat/opendm"

BUILD_DIR="${ROOT}/build/deb"
STAGE_DIR="${ROOT}/build/deb_staging/${PACKAGE_NAME}_${VERSION}_${ARCH}"
DIST_DIR="${ROOT}/dist"
DOC_DIR="${STAGE_DIR}/usr/share/doc/${PACKAGE_NAME}"

echo "=== Packaging OpenDM ${VERSION} (${ARCH}) for Debian / Ubuntu ==="

# 1. Build
cmake -S "${ROOT}" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "${BUILD_DIR}" -j"$(nproc)"

# 2. Stage the install tree
rm -rf "${STAGE_DIR}"
DESTDIR="${STAGE_DIR}" cmake --install "${BUILD_DIR}"
strip --strip-unneeded "${STAGE_DIR}/usr/bin/opendm" "${STAGE_DIR}/usr/bin/opendm-native-host"

# 3. Documentation, license and attribution (GPL-3.0 + NOTICE section 7 terms)
install -d "${DOC_DIR}"
install -m 644 README.md AUTHORS NOTICE "${DOC_DIR}/"
cat > "${DOC_DIR}/copyright" <<EOF
Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: OpenDM
Upstream-Contact: Nader Mahbub Khan <muhammadnadermahbubkhan@gmail.com>
Source: ${HOMEPAGE}
Comment: Modified fork of OpenDM, created by Nader Mahbub Khan
 (https://github.com/nadermkhan/opendm). See /usr/share/doc/${PACKAGE_NAME}/NOTICE
 and /usr/share/doc/${PACKAGE_NAME}/AUTHORS.

Files: *
Copyright: 2026 Nader Mahbub Khan <muhammadnadermahbubkhan@gmail.com>
License: GPL-3.0-or-later

License: GPL-3.0-or-later
$(sed -e 's/^$/./' -e 's/^/ /' LICENSE)
EOF
printf '%s (%s) unstable; urgency=medium\n\n  * Release %s.\n\n -- %s  %s\n' \
    "${PACKAGE_NAME}" "${VERSION}" "${UPSTREAM_VERSION}" "${MAINTAINER}" "$(date -R)" \
    | gzip -9n > "${DOC_DIR}/changelog.Debian.gz"

# 4. Runtime dependencies, computed from the linked shared libraries
SHLIBS_TMP="$(mktemp -d)"
trap 'rm -rf "${SHLIBS_TMP}"' EXIT
mkdir -p "${SHLIBS_TMP}/debian"
printf 'Source: %s\n\nPackage: %s\nArchitecture: any\n' "${PACKAGE_NAME}" "${PACKAGE_NAME}" \
    > "${SHLIBS_TMP}/debian/control"
DEPENDS="$(cd "${SHLIBS_TMP}" && dpkg-shlibdeps -O \
    "${STAGE_DIR}/usr/bin/opendm" "${STAGE_DIR}/usr/bin/opendm-native-host" 2>/dev/null \
    | sed -n 's/^shlibs:Depends=//p')"
if [ -z "${DEPENDS}" ]; then
    echo "warning: dpkg-shlibdeps failed; falling back to a static dependency list" >&2
    DEPENDS="libc6, libstdc++6, libgtk-3-0t64 | libgtk-3-0, libcurl4t64 | libcurl4, zlib1g"
fi
echo "Depends: ${DEPENDS}"

# 5. Control files
INSTALLED_SIZE="$(du -sk --exclude=DEBIAN "${STAGE_DIR}" | cut -f1)"
install -d "${STAGE_DIR}/DEBIAN"
cat > "${STAGE_DIR}/DEBIAN/control" <<EOF
Package: ${PACKAGE_NAME}
Version: ${VERSION}
Section: net
Priority: optional
Architecture: ${ARCH}
Depends: ${DEPENDS}
Recommends: ffmpeg
Installed-Size: ${INSTALLED_SIZE}
Maintainer: ${MAINTAINER}
Homepage: ${HOMEPAGE}
Description: High-speed segmented download manager (GTK3)
 OpenDM is a native Linux download accelerator with dynamic multipart
 segmentation, browser integration via native messaging, remote ZIP
 inspection, and YouTube stream extraction with automatic media muxing.
 .
 This is a modified fork of OpenDM, created by Nader Mahbub Khan
 (https://github.com/nadermkhan/opendm), ported to GTK3.
EOF

for script in postinst postrm; do
    cat > "${STAGE_DIR}/DEBIAN/${script}" <<'EOF'
#!/bin/sh
set -e
case "$1" in
    configure|remove|purge)
        if command -v update-desktop-database >/dev/null 2>&1; then
            update-desktop-database -q /usr/share/applications || true
        fi
        if command -v gtk-update-icon-cache >/dev/null 2>&1; then
            gtk-update-icon-cache -q -t -f /usr/share/icons/hicolor || true
        fi
        ;;
esac
exit 0
EOF
    chmod 755 "${STAGE_DIR}/DEBIAN/${script}"
done

# 6. Build the package
mkdir -p "${DIST_DIR}"
OUTPUT_DEB="${DIST_DIR}/${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"
dpkg-deb --build --root-owner-group "${STAGE_DIR}" "${OUTPUT_DEB}"

echo ""
echo "=== Package Build Successful ==="
echo "Artifact: ${OUTPUT_DEB}"
dpkg-deb -I "${OUTPUT_DEB}"
