#!/usr/bin/env bash
# Remove o Bitcurrency Core instalado pelo install.sh.
# NÃO apaga sua carteira nem a blockchain (~/.bitcurrency).
set -euo pipefail

SRC="$(cd "$(dirname "$0")" && pwd)"
if [ "$(id -u)" -eq 0 ]; then PREFIX=/usr/local; else PREFIX="$HOME/.local"; fi
[ "${1:-}" = "--prefix" ] && PREFIX="$2"

for f in "$SRC"/bin/*; do rm -f "$PREFIX/bin/$(basename "$f")"; done
(cd "$SRC/share" && find . -type f) | while read -r f; do rm -f "$PREFIX/share/$f"; done
echo "Bitcurrency Core removido de $PREFIX. Seus dados em ~/.bitcurrency foram mantidos."
