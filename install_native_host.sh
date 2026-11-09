#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Locate the native messaging host. Order: explicit override, per-user
# install (install_desktop.sh), local build tree, then the system install
# done by the .deb/.rpm/Arch packages.
HOST_BIN=""
for candidate in \
    "${OPENDM_NATIVE_HOST:-}" \
    "${HOME}/.local/bin/opendm-native-host" \
    "${SCRIPT_DIR}/build/opendm-native-host" \
    "$(command -v opendm-native-host 2>/dev/null || true)" \
    /usr/bin/opendm-native-host \
    /usr/local/bin/opendm-native-host; do
    if [ -n "${candidate}" ] && [ -x "${candidate}" ]; then
        HOST_BIN="${candidate}"
        break
    fi
done

if [ -z "${HOST_BIN}" ]; then
    echo "Error: opendm-native-host not found."
    echo "Install the OpenDM package for your distribution, or build from source and run ./install_desktop.sh."
    echo "You can also point to the binary with OPENDM_NATIVE_HOST=/path/to/opendm-native-host."
    exit 1
fi
echo "Using native messaging host: ${HOST_BIN}"

if [ "${HOST_BIN}" = "/usr/bin/opendm-native-host" ]; then
    echo "Note: the OpenDM package already registers the host system-wide for Firefox,"
    echo "Google Chrome and Chromium. This script adds per-user manifests, which also"
    echo "cover Brave and Microsoft Edge."
fi

MANIFEST_CONTENT_CHROME=$(cat <<EOF
{
  "name": "io.github.opendm",
  "description": "OpenDM native messaging host",
  "path": "${HOST_BIN}",
  "type": "stdio",
  "allowed_origins": [
    "chrome-extension://nnkpmdifclcaiakeomkmbddaekiaokfa/"
  ]
}
EOF
)

MANIFEST_CONTENT_FIREFOX=$(cat <<EOF
{
  "name": "io.github.opendm",
  "description": "OpenDM native messaging host",
  "path": "${HOST_BIN}",
  "type": "stdio",
  "allowed_extensions": [
    "opendm@fa-saikat.github.io"
  ]
}
EOF
)

# Chrome / Chromium / Brave / Edge paths
CHROME_TARGETS=(
    "${HOME}/.config/google-chrome/NativeMessagingHosts"
    "${HOME}/.config/chromium/NativeMessagingHosts"
    "${HOME}/.config/BraveSoftware/Brave-Browser/NativeMessagingHosts"
    "${HOME}/.config/microsoft-edge/NativeMessagingHosts"
)

# Only register browsers that have a profile directory (i.e. are installed and used).
for dir in "${CHROME_TARGETS[@]}"; do
    [ -d "$(dirname "${dir}")" ] || continue
    mkdir -p "${dir}"
    rm -f "${dir}/com.tonec.idm.json"  # left behind by OpenDM 1.0.0
    echo "${MANIFEST_CONTENT_CHROME}" > "${dir}/io.github.opendm.json"
    echo "Installed Chrome manifest to ${dir}/io.github.opendm.json"
done

# Firefox paths
FIREFOX_TARGETS=(
    "${HOME}/.mozilla/native-messaging-hosts"
    "${HOME}/.config/mozilla/native-messaging-hosts"
)

for dir in "${FIREFOX_TARGETS[@]}"; do
    [ -d "$(dirname "${dir}")" ] || continue
    mkdir -p "${dir}"
    rm -f "${dir}/com.tonec.idm.json"  # left behind by OpenDM 1.0.0
    echo "${MANIFEST_CONTENT_FIREFOX}" > "${dir}/io.github.opendm.json"
    echo "Installed Firefox manifest to ${dir}/io.github.opendm.json"
done

echo "Native messaging manifests successfully registered."
echo "Next, load the OpenDM extension in your browser; see the Browser Integration section of README.md."
