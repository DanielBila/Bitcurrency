#!/usr/bin/env bash
# Instala o Bitcurrency Core.
#   ./install.sh            instala só para você, em ~/.local (não precisa de senha)
#   sudo ./install.sh       instala para todos os usuários, em /usr/local
#   ./install.sh --prefix D instala em outra pasta
set -euo pipefail

SRC="$(cd "$(dirname "$0")" && pwd)"
if [ "$(id -u)" -eq 0 ]; then PREFIX=/usr/local; else PREFIX="$HOME/.local"; fi
[ "${1:-}" = "--prefix" ] && PREFIX="$2"

echo "Instalando Bitcurrency Core em $PREFIX ..."
for f in "$SRC"/bin/*; do install -Dm755 "$f" "$PREFIX/bin/$(basename "$f")"; done
(cd "$SRC/share" && find . -type f) | while read -r f; do
    install -Dm644 "$SRC/share/$f" "$PREFIX/share/$f"
done

command -v update-desktop-database >/dev/null && update-desktop-database -q "$PREFIX/share/applications" || true
command -v gtk-update-icon-cache >/dev/null && gtk-update-icon-cache -q -t "$PREFIX/share/icons/hicolor" || true
command -v xdg-mime >/dev/null && [ "$(id -u)" -ne 0 ] && xdg-mime default bitcurrency-qt.desktop x-scheme-handler/bitcurrency || true

echo
echo "Pronto! Abra \"Bitcurrency Core\" no menu de aplicativos, ou rode: bitcurrency-qt"
case ":$PATH:" in *":$PREFIX/bin:"*) ;; *)
    echo "Atenção: $PREFIX/bin não está no PATH. Faça logout/login ou adicione ao ~/.profile:"
    echo "    export PATH=\"$PREFIX/bin:\$PATH\"" ;;
esac
