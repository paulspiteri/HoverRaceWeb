#!/usr/bin/env bash
set -euo pipefail
cmake -B build -S .
cmake --build build
