#!/usr/bin/env bash
# Example build script for OpenSSL on wasm32-wasip1
# This script demonstrates how to build OpenSSL for WebAssembly
# using the WASI SDK.
#
# Prerequisites:
#   - WASI SDK installed (https://github.com/WebAssembly/wasi-sdk/releases)
#   - Perl 5 or later
#   - Make
#
# Usage:
#   ./build-wasm32-wasip1.sh [install-prefix]
#
# Example:
#   ./build-wasm32-wasip1.sh /tmp/openssl-wasm

set -e  # Exit on error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
INSTALL_PREFIX="${1:-/tmp/openssl-wasm}"
OPENSSLDIR="${INSTALL_PREFIX}/ssl"

echo -e "${GREEN}OpenSSL WASM32-wasip1 Build Script${NC}"
echo "=================================="
echo ""

# Check if WASI SDK is available
if ! command -v wasm32-wasip1-clang &> /dev/null && ! command -v wasm32-wasi-clang &> /dev/null; then
    echo -e "${RED}Error: WASI SDK not found!${NC}"
    echo ""
    echo "Please install WASI SDK from:"
    echo "  https://github.com/WebAssembly/wasi-sdk/releases"
    echo ""
    echo "Then add it to your PATH:"
    echo "  export WASI_SDK_PATH=/path/to/wasi-sdk"
    echo "  export PATH=\$WASI_SDK_PATH/bin:\$PATH"
    exit 1
fi

# Determine which compiler to use
if command -v wasm32-wasip1-clang &> /dev/null; then
    TARGET="wasm32-wasip1"
    COMPILER="wasm32-wasip1-clang"
else
    TARGET="wasm32-wasi"
    COMPILER="wasm32-wasi-clang"
fi

echo -e "${GREEN}Using compiler:${NC} $COMPILER"
echo -e "${GREEN}Install prefix:${NC} $INSTALL_PREFIX"
echo -e "${GREEN}OpenSSL dir:${NC} $OPENSSLDIR"
echo ""

# Clean any previous build
echo -e "${YELLOW}Cleaning previous build...${NC}"
if [ -f Makefile ]; then
    make distclean 2>/dev/null || true
fi

# Configure OpenSSL for WASM
echo -e "${YELLOW}Configuring OpenSSL for ${TARGET}...${NC}"
./Configure "$TARGET" \
    --prefix="$INSTALL_PREFIX" \
    --openssldir="$OPENSSLDIR" \
    no-threads \
    no-shared \
    no-asm \
    no-sock \
    no-ui-console

echo ""
echo -e "${GREEN}Configuration complete!${NC}"
echo ""

# Display configuration
echo -e "${YELLOW}Configuration summary:${NC}"
perl configdata.pm --dump | grep -E "target =|CC =|disable =|prefix =|openssldir =" || true
echo ""

# Build
echo -e "${YELLOW}Building OpenSSL...${NC}"
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo ""

# Optional: Run tests (some tests may not work in WASM environment)
read -p "Do you want to run tests? (y/N) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}Running tests...${NC}"
    make test || echo -e "${YELLOW}Warning: Some tests may fail in WASM environment${NC}"
fi

# Install
echo ""
read -p "Do you want to install to $INSTALL_PREFIX? (y/N) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}Installing OpenSSL...${NC}"
    make install
    echo ""
    echo -e "${GREEN}Installation complete!${NC}"
    echo ""
    echo "OpenSSL has been installed to:"
    echo "  Libraries: $INSTALL_PREFIX/lib"
    echo "  Headers:   $INSTALL_PREFIX/include"
    echo ""
    echo "To use in your WASM project:"
    echo "  ${COMPILER} -O3 \\"
    echo "    -I$INSTALL_PREFIX/include \\"
    echo "    -L$INSTALL_PREFIX/lib \\"
    echo "    -o myapp.wasm \\"
    echo "    myapp.c \\"
    echo "    -lssl -lcrypto"
fi

echo ""
echo -e "${GREEN}Done!${NC}"
