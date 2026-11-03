#!/usr/bin/env bash
# Prints the OpenDM version declared by project(VERSION ...) in CMakeLists.txt.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sed -nE 's/^project\(OpenDM VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' "${ROOT}/CMakeLists.txt"
