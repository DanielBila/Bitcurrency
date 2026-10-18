#!/usr/bin/env bash
# Runs inside the Docker image (see build.sh). Mirrors the relevant parts of
# contrib/guix/libexec/build.sh for the x86_64-pc-linux-gnu host.
set -euo pipefail

HOST=x86_64-pc-linux-gnu
JOBS="${JOBS:-$(nproc)}"
HERE=contrib/bitcurrency/linux-release
BUILDDIR=build-linux-release

VERSION="$(sed -n 's/^set(CLIENT_VERSION_\(MAJOR\|MINOR\|BUILD\) \([0-9]*\))/\2/p' CMakeLists.txt | paste -sd.)"
NAME="bitcurrency-${VERSION}"
STAGE="$BUILDDIR/stage/$NAME"
DIST=dist

echo "==> Building dependencies ($HOST, $JOBS jobs)"
make -C depends --jobs="$JOBS" HOST="$HOST" NO_IPC=1 NO_USDT=1

echo "==> Configuring"
LDFLAGS="-Wl,--as-needed -Wl,-O2" \
cmake -S . -B "$BUILDDIR" \
    --toolchain "depends/$HOST/toolchain.cmake" \
    -DREDUCE_EXPORTS=ON -DBUILD_BENCH=OFF -DBUILD_GUI_TESTS=OFF \
    -DBUILD_TESTS=OFF -DBUILD_FUZZ_BINARY=OFF -DCMAKE_SKIP_RPATH=TRUE \
    -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc"

echo "==> Compiling"
cmake --build "$BUILDDIR" -j "$JOBS"

echo "==> Staging"
rm -rf "$STAGE"
cmake --install "$BUILDDIR" --strip --prefix "$STAGE"
install -Dm644 share/pixmaps/bitcoin256.png "$STAGE/share/icons/hicolor/256x256/apps/bitcurrency.png"
install -Dm644 share/pixmaps/bitcoin128.png "$STAGE/share/icons/hicolor/128x128/apps/bitcurrency.png"
install -Dm644 share/pixmaps/bitcoin64.png  "$STAGE/share/icons/hicolor/64x64/apps/bitcurrency.png"
install -Dm644 share/pixmaps/bitcoin32.png  "$STAGE/share/icons/hicolor/32x32/apps/bitcurrency.png"
install -Dm644 "$HERE/bitcurrency-qt.desktop" "$STAGE/share/applications/bitcurrency-qt.desktop"
install -Dm644 share/examples/bitcoin.conf "$STAGE/share/doc/bitcurrency/bitcurrency.conf.example"
install -Dm644 COPYING "$STAGE/COPYING"
install -Dm644 "$HERE/LEIA-ME.txt" "$STAGE/LEIA-ME.txt"
install -Dm755 "$HERE/install.sh" "$STAGE/install.sh"
install -Dm755 "$HERE/uninstall.sh" "$STAGE/uninstall.sh"

echo "==> Checking glibc symbol versions"
MAX_GLIBC="$(objdump -T "$STAGE"/bin/* | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1)"
echo "Highest glibc symbol version required: $MAX_GLIBC"

echo "==> Packaging"
mkdir -p "$DIST"
tar -C "$BUILDDIR/stage" --owner=0 --group=0 --numeric-owner -czf "$DIST/$NAME-x86_64-linux-gnu.tar.gz" "$NAME"
(cd "$DIST" && sha256sum "$NAME-x86_64-linux-gnu.tar.gz" > "$NAME-x86_64-linux-gnu.tar.gz.sha256")
echo "==> Done: $DIST/$NAME-x86_64-linux-gnu.tar.gz"
