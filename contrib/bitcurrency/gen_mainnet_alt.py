#!/usr/bin/env python3
"""Generate test/functional/data/mainnet_alt.json for Bitcurrency.

Builds the first retarget period (2016 blocks) on top of the Bitcurrency mainnet
genesis exactly as test/functional/mining_mainnet.py reconstructs it, and grinds
each nonce with bitcurrency-util. Blocks are 30 s apart, so the first retarget
raises the difficulty by the maximum factor of 4.

Usage: contrib/bitcurrency/gen_mainnet_alt.py [BUILD_DIR]
"""
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "test" / "functional"))

from test_framework.blocktools import create_coinbase  # noqa: E402
from test_framework.messages import CBlock, CBlockHeader, SEQUENCE_FINAL, from_hex  # noqa: E402

GENESIS_HASH = "000008e43e5a666655a25fcca876dd75be95b7983a8ad01ea940c5ae9f341971"
GENESIS_TIME = 1791849600
POW_LIMIT_BITS = 0x1e0ffff0
MAX_BITS = 0x1e03fffc  # pow limit / 4
SPACING = 30
HALVING = 840000
COINBASE_SCRIPT_PUBKEY = "76a914eadbac7f36c37e39361168b7aaee3cb24a25312d88ac"

build = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "build"
util = next(p for p in (build / "bin" / "bitcurrency-util", build / "bin" / "bitcoin-util") if p.exists())

timestamps, nonces = [], []
prev = int(GENESIS_HASH, 16)
for height in range(1, 2017):
    block = CBlock()
    block.nVersion = 0x20000000
    block.hashPrevBlock = prev
    block.nTime = GENESIS_TIME + height * SPACING
    block.nBits = POW_LIMIT_BITS if height < 2016 else MAX_BITS
    block.nNonce = 0
    block.vtx = [create_coinbase(height=height, script_pubkey=bytes.fromhex(COINBASE_SCRIPT_PUBKEY), halving_period=HALVING)]
    block.vtx[0].nLockTime = 0
    block.vtx[0].vin[0].nSequence = SEQUENCE_FINAL
    block.hashMerkleRoot = block.calc_merkle_root()
    header_hex = CBlockHeader(block).serialize().hex()
    solved = subprocess.run([util, "grind", header_hex], check=True, capture_output=True, text=True).stdout.strip()
    header = from_hex(CBlockHeader(), solved)
    timestamps.append(header.nTime)
    nonces.append(header.nNonce)
    prev = header.hash_int
    if height % 100 == 0:
        print(f"height {height}", file=sys.stderr, flush=True)

out = ROOT / "test" / "functional" / "data" / "mainnet_alt.json"
out.write_text(json.dumps({"timestamps": timestamps, "nonces": nonces}, indent=4) + "\n")
print(f"wrote {out}")
