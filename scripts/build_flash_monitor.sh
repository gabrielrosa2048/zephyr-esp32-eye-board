#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
WEST_ROOT="$(cd "${PROJECT_DIR}/../.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
APP_DIR="${PROJECT_DIR}/app"
BOARD="esp32s3_devkitc/esp32s3/procpu"

source "${WEST_ROOT}/.venv/bin/activate"

case "${1:-all}" in
  build)
    rm -rf "${BUILD_DIR}"
    west build -b "${BOARD}" -d "${BUILD_DIR}" "${APP_DIR}"
    ;;
  flash)
    west flash -d "${BUILD_DIR}"
    ;;
  monitor)
    west espressif monitor -d "${BUILD_DIR}"
    ;;
  all)
    rm -rf "${BUILD_DIR}"
    west build -b "${BOARD}" -d "${BUILD_DIR}" "${APP_DIR}"
    west flash -d "${BUILD_DIR}"
    west espressif monitor
    ;;
  *)
    echo "Uso: $0 [build|flash|monitor|all]"
    exit 1
    ;;
esac
