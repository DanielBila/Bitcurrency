#!/usr/bin/env bash
# Builds a portable Bitcurrency release tarball for x86_64 Linux inside Docker.
# Usage: contrib/bitcurrency/linux-release/build.sh   (from anywhere)
# Env:   JOBS=N  number of parallel jobs (default: number of CPUs)
# Output: dist/bitcurrency-<version>-x86_64-linux-gnu.tar.gz
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
IMAGE=bitcurrency-linux-build:22.04

docker build -t "$IMAGE" "$HERE"
docker run --rm \
    --user "$(id -u):$(id -g)" \
    -e HOME=/tmp -e JOBS="${JOBS:-$(nproc)}" \
    -v "$ROOT":/src -w /src \
    "$IMAGE" contrib/bitcurrency/linux-release/build-inside.sh
