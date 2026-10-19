#!/usr/bin/env bash
# Instala o Bitcurrency Core no Linux com um único comando:
#   curl -fsSL https://raw.githubusercontent.com/DanielBila/Bitcurrency/bitcurrency/contrib/bitcurrency/instalar.sh | bash
# Para já conectar a um nó da rede, passe o IP dele:
#   curl -fsSL https://raw.githubusercontent.com/DanielBila/Bitcurrency/bitcurrency/contrib/bitcurrency/instalar.sh | bash -s -- IP_DO_NO
set -euo pipefail

VERSAO=31.1.0
PACOTE="bitcurrency-$VERSAO-x86_64-linux-gnu.tar.gz"
URL="https://github.com/DanielBila/Bitcurrency/releases/download/v$VERSAO"
NO="${1:-}"

passo() { printf '\n\033[1m==> %s\033[0m\n' "$*"; }
erro() { printf '\n\033[31mErro: %s\033[0m\n' "$*" >&2; exit 1; }

passo "Verificando o sistema"
[ "$(uname -m)" = x86_64 ] || erro "este pacote é para computadores de 64 bits (x86_64); o seu é $(uname -m)."
glibc=$(getconf GNU_LIBC_VERSION 2>/dev/null | awk '{print $2}')
if [ -n "$glibc" ] && [ "$(printf '%s\n2.35\n' "$glibc" | sort -V | head -1)" != 2.35 ]; then
    erro "o seu Linux é antigo demais (glibc $glibc). É preciso glibc 2.35 ou mais nova, como no Ubuntu 22.04+."
fi
echo "OK"

# A carteira gráfica precisa da libxcb-cursor; sem ela a janela não abre.
libs=$(ldconfig -p 2>/dev/null || /sbin/ldconfig -p 2>/dev/null || true)
if ! grep -q 'libxcb-cursor\.so\.0' <<<"$libs"; then
    passo "Instalando a biblioteca da interface gráfica (pode pedir a sua senha)"
    if command -v apt-get >/dev/null; then
        sudo apt-get install -y libxcb-cursor0
    elif command -v dnf >/dev/null; then
        sudo dnf install -y xcb-util-cursor
    elif command -v pacman >/dev/null; then
        sudo pacman -S --needed --noconfirm xcb-util-cursor
    elif command -v zypper >/dev/null; then
        sudo zypper install -y libxcb-cursor0
    else
        echo "Aviso: instale a biblioteca libxcb-cursor0 pelo gerenciador de pacotes do seu sistema."
    fi
fi

baixar() {
    if command -v curl >/dev/null; then curl -fL# --retry 3 -o "$2" "$1"
    elif command -v wget >/dev/null; then wget -q --show-progress -O "$2" "$1"
    else erro "instale o curl ou o wget e rode de novo."
    fi
}

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
cd "$TMP"

passo "Baixando o Bitcurrency Core $VERSAO (31 MB)"
baixar "$URL/$PACOTE" "$PACOTE"
baixar "$URL/$PACOTE.sha256" "$PACOTE.sha256"

passo "Conferindo o download"
sha256sum -c "$PACOTE.sha256" || erro "o arquivo baixado está corrompido. Rode o comando de novo."

passo "Instalando em ~/.local"
tar xzf "$PACOTE"
mkdir -p "$HOME/.config"
"./bitcurrency-$VERSAO/install.sh" >/dev/null
# Caminho absoluto: o menu esconde atalhos cujo programa não está no PATH da sessão
sed -i "s|^Exec=bitcurrency-qt |Exec=$HOME/.local/bin/bitcurrency-qt |" "$HOME/.local/share/applications/bitcurrency-qt.desktop"
{
    echo '#!/usr/bin/env bash'
    echo '# Remove o Bitcurrency Core. NÃO apaga a carteira nem a blockchain (~/.bitcurrency).'
    (cd "./bitcurrency-$VERSAO" && find bin share -type f) | sed "s|^|rm -f \"\$HOME/.local/|; s|\$|\"|"
    echo 'rm -f "$HOME/.local/bin/bitcurrency-desinstalar"'
    echo 'echo "Bitcurrency Core removido. Seus dados em ~/.bitcurrency foram mantidos."'
} > "$HOME/.local/bin/bitcurrency-desinstalar"
chmod +x "$HOME/.local/bin/bitcurrency-desinstalar"
echo "OK"

CONF="$HOME/.bitcurrency/bitcurrency.conf"
mkdir -p "$HOME/.bitcurrency"
if [ ! -f "$CONF" ]; then
    printf 'server=1\nfallbackfee=0.0001\n' > "$CONF"
fi
if [ -n "$NO" ] && ! grep -q "^addnode=$NO" "$CONF"; then
    passo "Configurando a conexão com o nó $NO"
    echo "addnode=$NO:7333" >> "$CONF"
fi

passo "Pronto!"
echo "Abra \"Bitcurrency Core\" no menu de aplicativos."
echo "Ou pelo terminal: ~/.local/bin/bitcurrency-qt"
[ -n "$NO" ] || echo "Para conectar a um nó da rede, adicione  addnode=IP_DO_NO:7333  ao arquivo $CONF"
echo "Para desinstalar: ~/.local/bin/bitcurrency-desinstalar"

if [ -n "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ]; then
    passo "Abrindo o Bitcurrency Core"
    nohup "$HOME/.local/bin/bitcurrency-qt" >/dev/null 2>&1 &
fi
