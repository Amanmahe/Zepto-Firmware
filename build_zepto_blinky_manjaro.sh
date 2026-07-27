#!/usr/bin/env bash
#
# Build every app in apps/ for BeagleConnect Zepto (mspm0l1117) on Manjaro/Arch.
#
# Usage:
#   chmod +x build_zepto_blinky_manjaro.sh
#   ./build_zepto_blinky_manjaro.sh            # build all apps
#   ./build_zepto_blinky_manjaro.sh blinky      # build just apps/blinky
#
# Layout expected (same as the GitHub Actions workflow):
#   apps/<name>/{CMakeLists.txt,prj.conf,src/...}
#   patches/*.patch   (applied to Zephyr itself, in name order)

set -euo pipefail

WORKDIR="${HOME}/zephyr-zepto"
ZEPHYR_SDK_VERSION="1.0.1"
BOARD="beagleconnect_zepto/mspm0l1117"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "==> 1. Installing system packages (pacman)"
sudo pacman -Sy --needed --noconfirm \
    base-devel cmake ninja dtc python python-pip python-virtualenv \
    gperf ccache dfu-util wget xz git

echo "==> 2. Creating workspace at ${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

echo "==> 3. Setting up Python venv + west"
python -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install west

echo "==> 4. Initializing Zephyr workspace (shallow clone)"
if [ ! -d zephyrproject/.west ]; then
    west init --mr main -o=--depth=1 --mf west.yml zephyrproject
fi
cd zephyrproject

echo "==> 5. Fetching only the modules this board needs (hal_ti, cmsis, cmsis_6)"
west update --narrow -o=--depth=1 hal_ti cmsis cmsis_6

echo "==> 6. Installing Zephyr's Python requirements"
pip install -r zephyr/scripts/requirements.txt

echo "==> 7. Downloading the Zephyr SDK (minimal bundle + arm-zephyr-eabi toolchain only, ~170MB)"
cd "${WORKDIR}"
if [ ! -d "zephyr-sdk-${ZEPHYR_SDK_VERSION}/gnu/arm-zephyr-eabi" ]; then
    wget -q --show-progress \
        "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz" \
        -O sdk-minimal.tar.xz
    mkdir -p "zephyr-sdk-${ZEPHYR_SDK_VERSION}"
    tar xf sdk-minimal.tar.xz -C "${WORKDIR}"

    wget -q --show-progress \
        "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/toolchain_gnu_linux-x86_64_arm-zephyr-eabi.tar.xz" \
        -O arm-toolchain.tar.xz
    mkdir -p "zephyr-sdk-${ZEPHYR_SDK_VERSION}/gnu/arm-zephyr-eabi"
    tar xf arm-toolchain.tar.xz -C "zephyr-sdk-${ZEPHYR_SDK_VERSION}/gnu/arm-zephyr-eabi" --strip-components=1
    rm -f sdk-minimal.tar.xz arm-toolchain.tar.xz
fi

export ZEPHYR_SDK_INSTALL_DIR="${WORKDIR}/zephyr-sdk-${ZEPHYR_SDK_VERSION}"
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr

echo "==> 8. Applying local Zephyr patches (patches/*.patch, in name order)"
# As of mid-2026, stock Zephyr main fails to build beagleconnect_zepto because
# drivers/gpio/gpio_mspm0.c references IOMUX_PINCM macros that don't exist in
# the current hal_ti header for this chip — see patches/0001-*.patch.
# Re-check upstream and remove the patch once that lands:
# https://github.com/zephyrproject-rtos/zephyr
cd "${WORKDIR}/zephyrproject/zephyr"
shopt -s nullglob
for p in "${SCRIPT_DIR}"/patches/*.patch; do
    if git apply --reverse --check "$p" 2>/dev/null; then
        echo "    $(basename "$p") already applied, skipping"
    else
        echo "    applying $(basename "$p")"
        git apply "$p"
    fi
done

echo "==> 9. Discovering apps under apps/"
cd "${SCRIPT_DIR}"
if [ "$#" -ge 1 ]; then
    apps=("$@")
else
    apps=()
    for d in apps/*/; do
        apps+=("$(basename "$d")")
    done
fi
echo "    apps to build: ${apps[*]}"

cd "${WORKDIR}/zephyrproject"
for app in "${apps[@]}"; do
    echo "==> 10. Building '${app}' for ${BOARD}"
    west build -b "${BOARD}" "${SCRIPT_DIR}/apps/${app}" -d "build_${app}" --pristine

    echo "    Generating plain-text hex dump for ${app}"
    python - "build_${app}" <<'EOF'
import sys
build_dir = sys.argv[1]
with open(f'{build_dir}/zephyr/zephyr.bin', 'rb') as f:
    data = f.read()
with open(f'{build_dir}/zephyr/zephyr.txt', 'w') as out:
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hexpart = ' '.join(f'{b:02x}' for b in chunk)
        ascii_part = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk)
        out.write(f'{i:08x}  {hexpart:<47}  {ascii_part}\n')
EOF

    echo "    Copying readable source next to the firmware"
    mkdir -p "build_${app}/zephyr/release-src/patches"
    cp -r "${SCRIPT_DIR}/apps/${app}/src" "build_${app}/zephyr/release-src/app-src"
    cp "${SCRIPT_DIR}/apps/${app}/prj.conf" "build_${app}/zephyr/release-src/" 2>/dev/null || true
    cp "${SCRIPT_DIR}"/patches/*.patch "build_${app}/zephyr/release-src/patches/" 2>/dev/null || true
done

echo
echo "==> Done. Output per app:"
for app in "${apps[@]}"; do
    echo "  ${app}:"
    echo "    ${WORKDIR}/zephyrproject/build_${app}/zephyr/zephyr.bin"
    echo "    ${WORKDIR}/zephyrproject/build_${app}/zephyr/zephyr.hex"
    echo "    ${WORKDIR}/zephyrproject/build_${app}/zephyr/zephyr.txt"
    echo "    ${WORKDIR}/zephyrproject/build_${app}/zephyr/release-src/  (readable source)"
done
echo
echo "To flash over I2C/UART or the Pi/BeagleY-AI HAT connector: use the .bin with"
echo "the BeagleBoard Imaging Utility (https://beagleboard.github.io/bb-imager-rs/)."
echo
echo "If you have an XDS110 debug probe wired to SWD, you can instead flash directly:"
echo "    cd ${WORKDIR}/zephyrproject"
echo "    source ${WORKDIR}/.venv/bin/activate  # if in a new shell"
echo "    export ZEPHYR_SDK_INSTALL_DIR=${WORKDIR}/zephyr-sdk-${ZEPHYR_SDK_VERSION}"
echo "    export ZEPHYR_TOOLCHAIN_VARIANT=zephyr"
echo "    west build -b ${BOARD} ${SCRIPT_DIR}/apps/<name> -d build_<name>"
echo "    west flash -d build_<name>"
