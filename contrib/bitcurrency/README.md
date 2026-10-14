# Bitcurrency (BCY)

Fork do Bitcoin Core com uma blockchain própria.

| Parâmetro | Valor |
|---|---|
| Tempo de bloco | 2,5 minutos |
| Reajuste de dificuldade | a cada 2016 blocos (~3,5 dias) |
| Recompensa inicial | 50 BCY |
| Halving | a cada 840.000 blocos (~4 anos) |
| Suprimento máximo | 84.000.000 BCY |
| Algoritmo | SHA-256d |
| Porta P2P / RPC (mainnet) | 7333 / 7332 |
| Porta P2P / RPC (testnet3, testnet4, signet, regtest) | 17333/17332, 47333/47332, 37333/37332, 17444/17443 |
| Endereços | `B...` (legacy), `b...` (P2SH), `bcy1...` (SegWit/Taproot) |
| Pasta de dados | `~/.bitcurrency` (configuração em `bitcurrency.conf`) |
| Gênese (mainnet) | `000008e43e5a666655a25fcca876dd75be95b7983a8ad01ea940c5ae9f341971` |
| Proteção timewarp (BIP94) | ativa desde o gênese |
| Derivação HD da carteira | `m/84h/7333h/...` (coin type SLIP-44 7333; testes usam `1h`) |

## Compilar

```bash
cmake -B build -DENABLE_IPC=OFF -DBUILD_GUI=OFF -DBUILD_TESTS=OFF -DBUILD_UTIL=ON
cmake --build build -j4
```

Os executáveis ficam em `build/bin/` (`bitcurrencyd`, `bitcurrency-cli`, `bitcurrency-wallet`, `bitcurrency-tx` e `bitcurrency-util`).

## Rodar um nó e minerar

```bash
build/bin/bitcurrencyd -daemon -fallbackfee=0.0001
build/bin/bitcurrency-cli createwallet minha
ADDR=$(build/bin/bitcurrency-cli getnewaddress)
# o 3º argumento (maxtries) precisa ser alto: o padrão de 1.000.000 costuma não bastar para 1 bloco
build/bin/bitcurrency-cli generatetoaddress 101 "$ADDR" 1000000000
build/bin/bitcurrency-cli getbalance
build/bin/bitcurrency-cli stop
```

Moedas mineradas só podem ser gastas depois de 100 confirmações.

## Lançar a rede (primeiro bloco)

O `getblocktemplate` (usado por softwares de mineração e pools) só funciona na mainnet quando o nó
tem pelo menos um peer **e** não está em "sincronização inicial". O nó se considera sincronizando
enquanto o último bloco tiver mais de 24 h (`-maxtipage`). Como o gênese é de 13/10/2026, uma rede
nova fica presa nesse estado até alguém minerar um bloco.

Para lançar, minere o primeiro bloco com o minerador interno, que não tem essa restrição:

```bash
build/bin/bitcurrency-cli generatetoaddress 1 "$ADDR" 1000000000
```

Depois disso o nó sai do estado de sincronização e pools/mineradores externos funcionam. Se a rede
ficar mais de 24 h sem blocos, repita o procedimento (ou inicie um nó com `-maxtipage=999999999`).

## Conectar nós

Ainda não há DNS seeds. Conecte os nós manualmente:

```bash
build/bin/bitcurrencyd -daemon -addnode=IP_DO_OUTRO_NO:7333
```

Para uma rede pública, adicione seus servidores em `vSeeds` (`src/kernel/chainparams.cpp`).

## Minerar um novo gênese

Se mudar qualquer dado do gênese (frase, horário, chave, nBits), gere um novo bloco e atualize o
`CreateGenesisBlock(...)` e os dois `assert` em `src/kernel/chainparams.cpp`:

```bash
contrib/bitcurrency/genesis.py --time $(date +%s) --bits 0x1e0ffff0 \
    --timestamp "Sua frase" --pubkey 04...
```

## Signet

A signet da Bitcurrency só aceita blocos assinados pela chave do operador. O desafio padrão
(`SigNetParams` em `src/kernel/chainparams.cpp`) é o P2WPKH do endereço `tbcy1q7pjgz5cwz3zn6wxtgurcn6k7rnm4v2udzxzrmf`, cuja
chave está na carteira `~/.bitcurrency/signet/wallets/signet-operator`. **Faça backup dessa
carteira**: sem ela ninguém mais consegue produzir blocos na signet.

Minerar (com a carteira do operador carregada):

```bash
CLI="build/bin/bitcurrency-cli -signet"
build/bin/bitcurrencyd -signet -daemon
$CLI loadwallet signet-operator
contrib/signet/miner --cli="$CLI" generate --grind-cmd="build/bin/bitcurrency-util grind" \
    --address=$($CLI getnewaddress) --min-nbits --ongoing
```

Outras pessoas só rodam `bitcurrencyd -signet` e conectam ao seu nó (`-addnode=IP:37333`).

Ao trocar o desafio, os bytes mágicos da signet mudam (são derivados do desafio), e carteiras
criadas antes passam a ser recusadas ("Data is not in recognized format"), porque o arquivo guarda
os bytes mágicos no `application_id` do SQLite.
