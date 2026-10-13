#!/usr/bin/env python3
# Copyright (c) 2026 The Bitcurrency developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""End-to-end check of the first mainnet difficulty adjustment and BIP94.

Starts two private Bitcurrency mainnet nodes, mines past block 2016 and checks:
  * the first retarget happens at height 2016 and matches an independent computation,
  * a block with the old difficulty is rejected (bad-diffbits),
  * a timewarp block (timestamp > 600 s before its parent) is rejected (time-timewarp-attack),
  * a block exactly 600 s before its parent is accepted,
  * the second node follows the chain.

Usage: contrib/bitcurrency/test_retarget.py <build/bin dir> <empty work dir>
Takes a few hours on a CPU: ~2000 blocks at minimum difficulty.
"""

import json
import os
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "test" / "functional"))
from test_framework.blocktools import create_block, create_coinbase  # noqa: E402
from test_framework.messages import CBlockHeader, from_hex  # noqa: E402

BIN = Path(sys.argv[1]).resolve()
WORK = Path(sys.argv[2]).resolve()
INTERVAL = 2016


class Node:
    def __init__(self, name, port, rpcport, extra):
        self.datadir = WORK / name
        self.datadir.mkdir(parents=True)
        self.rpcport = rpcport
        subprocess.run([BIN / "bitcoind", f"-datadir={self.datadir}", f"-port={port}", f"-rpcport={rpcport}",
                        "-bind=127.0.0.1", "-listen=1", "-daemon", *extra], check=True, stdout=subprocess.DEVNULL)

    def cli(self, *args):
        out = subprocess.run([BIN / "bitcoin-cli", f"-datadir={self.datadir}", f"-rpcport={self.rpcport}", "-rpcwait", *map(str, args)],
                             check=True, capture_output=True, text=True).stdout.strip()
        try:
            return json.loads(out)
        except json.JSONDecodeError:
            return out

    def stop(self):
        subprocess.run([BIN / "bitcoin-cli", f"-datadir={self.datadir}", f"-rpcport={self.rpcport}", "stop"], capture_output=True)


def grind(block):
    """Find a valid nonce with bitcoin-util grind."""
    head = CBlockHeader.serialize(block).hex()
    solved = subprocess.run([BIN / "bitcoin-util", "grind", head], check=True, capture_output=True, text=True).stdout.strip()
    block.nNonce = from_hex(CBlockHeader(), solved).nNonce
    return block


def template_block(node, ntime):
    tmpl = node.cli("getblocktemplate", json.dumps({"rules": ["segwit"]}))
    coinbase = create_coinbase(tmpl["height"], halving_period=840000)
    block = create_block(tmpl=tmpl, coinbase=coinbase, ntime=ntime)
    return tmpl, block


def submit(node, block):
    return node.cli("submitblock", block.serialize().hex()) or "accepted"


POW_LIMIT = int("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff", 16)
TARGET_TIMESPAN = 84 * 60 * 60


def compact_to_target(bits):
    return (bits & 0x007fffff) << (8 * ((bits >> 24) - 3))


def target_to_compact(target):
    size = (target.bit_length() + 7) // 8
    compact = target << (8 * (3 - size)) if size <= 3 else target >> (8 * (size - 3))
    if compact & 0x00800000:
        compact >>= 8
        size += 1
    return compact | (size << 24)


def expected_bits(first_bits, actual_timespan):
    """Independent re-implementation of CalculateNextWorkRequired for mainnet."""
    actual_timespan = max(TARGET_TIMESPAN // 4, min(TARGET_TIMESPAN * 4, actual_timespan))
    target = compact_to_target(first_bits) * actual_timespan // TARGET_TIMESPAN
    return f"{target_to_compact(min(target, POW_LIMIT)):08x}"


def check(cond, msg):
    print(("OK   " if cond else "FAIL ") + msg, flush=True)
    if not cond:
        raise SystemExit(1)


def main():
    a = Node("a", 7333, 7332, [])
    b = Node("b", 7343, 7342, ["-connect=127.0.0.1:7333"])
    try:
        a.cli("createwallet", "miner")
        addr = a.cli("getnewaddress")
        genesis_bits = a.cli("getblockheader", a.cli("getblockhash", 0))["bits"]
        start = time.time()

        # Mine up to height 2014 with the internal miner.
        while (height := a.cli("getblockcount")) < INTERVAL - 2:
            n = min(100, INTERVAL - 2 - height)
            a.cli("generatetoaddress", n, addr, 10**9)
            print(f"height {height + n}  ({(time.time() - start) / 60:.0f} min)", flush=True)

        # Block 2015, the last block of the first period, 50 minutes in the future.
        tip = a.cli("getblockheader", a.cli("getbestblockhash"))
        t2015 = int(time.time()) + 3000
        tmpl, block = template_block(a, t2015)
        check(tmpl["bits"] == genesis_bits, f"block 2015 still uses the genesis difficulty ({genesis_bits})")
        check(submit(a, grind(block)) == "accepted", "block 2015 accepted")

        # Block 2016: first difficulty adjustment.
        tmpl, block = template_block(a, t2015 - 600)
        period_seconds = t2015 - a.cli("getblockheader", a.cli("getblockhash", 0))["time"]
        new_bits = expected_bits(int(genesis_bits, 16), period_seconds)
        print(f"first period took {period_seconds} s (target {TARGET_TIMESPAN} s)")
        check(tmpl["bits"] == new_bits, f"retarget at 2016: bits {tmpl['bits']} (independent computation {new_bits})")
        check(tmpl["bits"] != genesis_bits, "difficulty changed at height 2016")

        old = create_block(tmpl=tmpl, coinbase=create_coinbase(tmpl["height"], halving_period=840000), ntime=t2015 - 600)
        old.nBits = int(genesis_bits, 16)
        check(submit(a, grind(old)) == "bad-diffbits", "block 2016 with the old difficulty rejected (bad-diffbits)")

        warp = create_block(tmpl=tmpl, coinbase=create_coinbase(tmpl["height"], halving_period=840000), ntime=t2015 - 601)
        check(submit(a, grind(warp)) == "time-timewarp-attack", "block 2016 601 s before its parent rejected (time-timewarp-attack)")

        check(submit(a, grind(block)) == "accepted", "block 2016 exactly 600 s before its parent accepted")
        check(a.cli("getblockcount") == INTERVAL, "chain height is 2016")

        # Next blocks keep the new difficulty.
        a.cli("generatetoaddress", 2, addr, 10**9)
        check(a.cli("getblockheader", a.cli("getbestblockhash"))["bits"] == new_bits, f"blocks after 2016 keep bits {new_bits}")

        for _ in range(60):
            if b.cli("getblockcount") == INTERVAL + 2:
                break
            time.sleep(1)
        check(b.cli("getbestblockhash") == a.cli("getbestblockhash"), "second node followed the chain")
        print(f"difficulty: {a.cli('getdifficulty')}  total time: {(time.time() - start) / 60:.0f} min")
    finally:
        a.stop()
        b.stop()


if __name__ == "__main__":
    main()
