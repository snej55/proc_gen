#!/usr/bin/bash
source ~/vulkan/1.4.341.1/setup-env.sh
mkdir -p build

PROJECT_NAME="proc_gen"
DEPS_DIR="$HOME/_deps/$PROJECT_NAME"
cmake -S . -B build -G Ninja -DFETCH_CONTENT_BASE_DIR="$DEPS_DIR"
cmake --build build/ -j$(nproc)