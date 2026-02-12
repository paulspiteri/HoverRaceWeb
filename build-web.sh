#!/usr/bin/env bash
set -euo pipefail
emcmake cmake -B build-web -S .
cmake --build build-web
