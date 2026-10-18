# Build environment for portable Bitcurrency Linux release tarballs.
# Ubuntu 22.04 ships glibc 2.35, so binaries built here run on any distro
# with glibc >= 2.35 (Ubuntu 22.04+, Debian 12+, Mint 21+, Fedora 36+...).
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        bison ca-certificates cmake curl g++-12 gcc-12 git make ninja-build \
        patch pkg-config python3 xz-utils \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100 \
    && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-12 100 \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-12 100
