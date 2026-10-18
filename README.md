Bitcurrency (BCY)
=================

Bitcurrency é uma criptomoeda própria, com blockchain independente, baseada no
[Bitcoin Core](https://github.com/bitcoin/bitcoin) v31.1. Ela usa o mesmo modelo
de segurança do Bitcoin (prova de trabalho SHA-256d, validação completa de blocos
e transações, carteira com SegWit e Taproot), mas tem rede, gênese, endereços e
regras econômicas próprias. Moedas BCY não têm relação com bitcoins: as duas redes
não se conectam.

O pacote inclui:

- **bitcurrency-qt** — carteira com interface gráfica (enviar, receber, histórico);
- **bitcurrencyd** — o nó, que conecta à rede, valida e guarda a blockchain;
- **bitcurrency-cli** — linha de comando para controlar o nó e minerar.

Características
---------------

| Parâmetro | Valor |
|---|---|
| Símbolo | BCY |
| Tempo de bloco | 2,5 minutos |
| Reajuste de dificuldade | a cada 2016 blocos (~3,5 dias) |
| Recompensa inicial | 50 BCY por bloco |
| Halving | a cada 840.000 blocos (~4 anos) |
| Suprimento máximo | 84.000.000 BCY |
| Algoritmo | SHA-256d (prova de trabalho) |
| Portas (mainnet) | 7333 (P2P) / 7332 (RPC) |
| Endereços | `bcy1...` (SegWit/Taproot), `B...` (legacy), `b...` (P2SH) |
| Proteção timewarp (BIP94) | ativa desde o gênese |
| Pasta de dados | `~/.bitcurrency` (configuração em `bitcurrency.conf`) |

Baixar e instalar (Linux)
-------------------------

Baixe `bitcurrency-31.1.0-x86_64-linux-gnu.tar.gz` na página de
[Releases](https://github.com/DanielBila/Bitcurrency/releases/latest). Ele funciona em
Linux 64 bits com glibc 2.35 ou mais nova (Ubuntu 22.04+, Debian 12+, Fedora 36+ etc.).

```bash
tar xzf bitcurrency-31.1.0-x86_64-linux-gnu.tar.gz
cd bitcurrency-31.1.0
./install.sh
```

Depois abra **Bitcurrency Core** no menu de aplicativos. Para remover, rode `./uninstall.sh`
(a carteira e a blockchain em `~/.bitcurrency` são mantidas).

Primeiros passos
----------------

A rede ainda não tem DNS seeds, então os nós precisam ser conectados manualmente. Crie
`~/.bitcurrency/bitcurrency.conf`:

```ini
server=1
# rede nova: ainda não há dados para estimar taxas
fallbackfee=0.0001
# IP de outro nó Bitcurrency (um por linha)
addnode=IP_DO_OUTRO_NO:7333
```

Libere a porta 7333 no firewall. Para minerar pela linha de comando:

```bash
bitcurrency-cli createwallet minha
bitcurrency-cli generatetoaddress 1 $(bitcurrency-cli getnewaddress) 1000000000
```

Moedas mineradas só podem ser gastas depois de 100 confirmações, e transações só são
confirmadas quando alguém da rede minera um bloco.

Compilar
--------

- **Instalador portátil para Linux** (Docker, gera o `.tar.gz` em `dist/`):
  ```bash
  contrib/bitcurrency/linux-release/build.sh
  ```
  A primeira vez leva de 1 a 2 horas (compila o Qt); as seguintes, bem menos.
- **Compilação local** para desenvolvimento: veja `doc/build-unix.md` e o
  [guia da Bitcurrency](contrib/bitcurrency/README.md).

O [guia da Bitcurrency](contrib/bitcurrency/README.md) também explica como conectar vários
dispositivos, lançar a rede, gerar um novo gênese e operar a signet.

Licença
-------

Bitcurrency é distribuído sob a licença MIT, assim como o Bitcoin Core, do qual deriva.
Veja [COPYING](COPYING) ou https://opensource.org/license/MIT. Créditos ao código original
aos desenvolvedores do Bitcoin Core.
