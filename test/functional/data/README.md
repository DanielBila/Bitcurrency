# Various test vectors

## mainnet_alt.json

Bitcurrency: the first retarget period (2016 blocks) on top of the Bitcurrency
mainnet genesis, with blocks 30 seconds apart so the first retarget raises the
difficulty by the maximum factor of 4. Each block is built exactly as
`mining_mainnet.py` reconstructs it (a coinbase-only block paying to the address
below), so only the timestamps and nonces need to be stored.

Regenerate it with:

```sh
contrib/bitcurrency/gen_mainnet_alt.py [BUILD_DIR]
```

The payout address is derived from first BIP32 test vector master key:

```
pkh(xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVvvNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHi/44h/0h/0h/<0;1>/*)#fkjtr0yn
```
