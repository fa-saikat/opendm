#!/usr/bin/env bash
# Builds binary and source RPMs for Fedora / RHEL-likes / openSUSE.
#
# Build dependencies (Fedora):
#   rpm-build cmake make gcc-c++ gtk3-devel libcurl-devel zlib-devel
#   json-devel desktop-file-utils
#
# Output: dist/opendm-<version>-<release>.<dist>.<arch>.rpm (+ .src.rpm)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

VERSION="$("${ROOT}/scripts/version.sh")"
PACKAGER="${RPM_PACKAGER:-FA Saikat <fahimabrar.saikat@gmail.com>}"
TOPDIR="${ROOT}/build/rpm"
DIST_DIR="${ROOT}/dist"

echo "=== Packaging OpenDM ${VERSION} as RPM ==="

rm -rf "${TOPDIR}"
mkdir -p "${TOPDIR}"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# 1. Source tarball from the tracked files of the working tree
if git -C "${ROOT}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    git -C "${ROOT}" ls-files -z \
        | tar --null -T - --transform "s,^,opendm-${VERSION}/," -czf "${TOPDIR}/SOURCES/opendm-${VERSION}.tar.gz"
else
    tar --exclude=./build --exclude=./dist --exclude=./.git \
        --transform "s,^\./,opendm-${VERSION}/," -czf "${TOPDIR}/SOURCES/opendm-${VERSION}.tar.gz" .
fi

# 2. Spec with a changelog entry for this version
{
    sed '/^%changelog/,$d' packaging/rpm/opendm.spec
    echo "%changelog"
    echo "* $(LC_ALL=C date '+%a %b %d %Y') ${PACKAGER} - ${VERSION}-1"
    echo "- Release ${VERSION}"
} > "${TOPDIR}/SPECS/opendm.spec"

# 3. Build
rpmbuild -ba \
    --define "_topdir ${TOPDIR}" \
    --define "opendm_version ${VERSION}" \
    "${TOPDIR}/SPECS/opendm.spec"

mkdir -p "${DIST_DIR}"
find "${TOPDIR}/RPMS" "${TOPDIR}/SRPMS" -name '*.rpm' ! -name '*-debuginfo-*' ! -name '*-debugsource-*' \
    -exec cp -v {} "${DIST_DIR}/" \;
