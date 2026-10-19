Bitcurrency (BCY)
=================

Bitcurrency é uma criptomoeda com blockchain própria, construída sobre o
[Bitcoin Core](https://github.com/bitcoin/bitcoin) v31.1. Ela herda o modelo de segurança
mais testado do mercado (prova de trabalho SHA-256d, validação completa de blocos e
transações, SegWit e Taproot) e traz rede, bloco gênese, endereços e política monetária
próprios: blocos 4× mais rápidos e um suprimento máximo de 84 milhões de BCY.

A Bitcurrency é uma rede independente: os nós BCY só se conectam entre si.

Componentes
-----------

- **bitcurrency-qt**: carteira com interface gráfica (enviar, receber, histórico, URIs `bitcurrency:`);
- **bitcurrencyd**: o nó completo, que conecta à rede, valida e armazena a blockchain;
- **bitcurrency-cli**: interface de linha de comando (RPC) para controlar o nó e minerar.

Especificações
--------------

| Parâmetro | Valor |
|---|---|
| Símbolo | BCY |
| Tempo de bloco | 2,5 minutos |
| Reajuste de dificuldade | a cada 2016 blocos (~3,5 dias) |
| Recompensa inicial | 50 BCY por bloco |
| Halving | a cada 840.000 blocos (~4 anos) |
| Suprimento máximo | 84.000.000 BCY |
| Algoritmo | SHA-256d (prova de trabalho) |
| Proteção timewarp (BIP94) | ativa desde o gênese |
| Endereços | `bcy1...` (SegWit/Taproot), `B...` (legacy), `b...` (P2SH) |
| Derivação HD | `m/84h/7333h/...` (coin type 7333) |
| Portas (mainnet) | 7333 (P2P) / 7332 (RPC) |
| Gênese (mainnet) | `000008e43e5a666655a25fcca876dd75be95b7983a8ad01ea940c5ae9f341971` |
| Pasta de dados | `~/.bitcurrency` (configuração em `bitcurrency.conf`) |

Instalar (Linux)
----------------

Abra o terminal (Ctrl + Alt + T) e rode:

```bash
curl -fsSL https://raw.githubusercontent.com/DanielBila/Bitcurrency/bitcurrency/contrib/bitcurrency/instalar.sh | bash
```

O script baixa o pacote da página de
[Releases](https://github.com/DanielBila/Bitcurrency/releases/latest), confere o sha256,
instala em `~/.local` e abre o **Bitcurrency Core**. Funciona em Linux 64 bits com glibc 2.35
ou mais nova (Ubuntu 22.04+, Debian 12+, Fedora 36+ etc.). Para já conectar a um nó, passe o
IP dele no fim: `... | bash -s -- IP_DO_NO`.

Para remover, rode `~/.local/bin/bitcurrency-desinstalar` (a carteira e a blockchain em
`~/.bitcurrency` são mantidas).

Primeiros passos
----------------

Crie `~/.bitcurrency/bitcurrency.conf` indicando os nós aos quais se conectar:

```ini
server=1
fallbackfee=0.0001
# um nó Bitcurrency por linha
addnode=IP_DO_NO:7333
```

Libere a porta 7333 no firewall para aceitar conexões de outros nós. Para minerar pela linha
de comando:

```bash
bitcurrency-cli createwallet minha
bitcurrency-cli generatetoaddress 1 $(bitcurrency-cli getnewaddress) 1000000000
```

Moedas mineradas podem ser gastas depois de 100 confirmações.

Compilar a partir do código
---------------------------

- **Pacote portátil para Linux** (via Docker, gera o `.tar.gz` em `dist/`):
  ```bash
  contrib/bitcurrency/linux-release/build.sh
  ```
- **Compilação local** para desenvolvimento: veja `doc/build-unix.md` e o
  [guia técnico da Bitcurrency](contrib/bitcurrency/README.md).

O [guia técnico](contrib/bitcurrency/README.md) cobre a operação da rede em detalhes: conectar
vários nós, iniciar a mineração, gerar um novo bloco gênese e operar a signet.

Licença
-------

Bitcurrency é distribuído sob a licença MIT, assim como o Bitcoin Core, do qual deriva.
Veja [COPYING](COPYING) ou https://opensource.org/license/MIT. Créditos ao código original
aos desenvolvedores do Bitcoin Core.
