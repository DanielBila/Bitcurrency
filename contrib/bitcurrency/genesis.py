#!/usr/bin/env python3
# Copyright (c) 2026 The Bitcurrency developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Mine a genesis block exactly as CreateGenesisBlock() in src/kernel/chainparams.cpp builds it.

Example:
    contrib/bitcurrency/genesis.py --time 1791892800 --bits 0x1e0ffff0 \\
        --timestamp "Bitcurrency 13/Oct/2026 ..." --pubkey 04abcd...

Prints nNonce, the block hash and the merkle root to paste into chainparams.cpp.
"""

import argparse
import hashlib
import struct
import sys


def sha256d(data: bytes) -> bytes:
    return hashlib.sha256(hashlib.sha256(data).digest()).digest()


def ser_compact_size(n: int) -> bytes:
    if n < 253:
        return bytes([n])
    if n <= 0xFFFF:
        return b"\xfd" + struct.pack("<H", n)
    if n <= 0xFFFFFFFF:
        return b"\xfe" + struct.pack("<I", n)
    return b"\xff" + struct.pack("<Q", n)


def push_data(data: bytes) -> bytes:
    if len(data) < 0x4C:
        return bytes([len(data)]) + data
    if len(data) <= 0xFF:
        return b"\x4c" + bytes([len(data)]) + data
    return b"\x4d" + struct.pack("<H", len(data)) + data


def script_num(n: int) -> bytes:
    """Serialize like CScriptNum::serialize (minimal little-endian, sign bit)."""
    if n == 0:
        return b""
    neg = n < 0
    absv = abs(n)
    out = bytearray()
    while absv:
        out.append(absv & 0xFF)
        absv >>= 8
    if out[-1] & 0x80:
        out.append(0x80 if neg else 0x00)
    elif neg:
        out[-1] |= 0x80
    return bytes(out)


def coinbase_tx(timestamp: bytes, pubkey: bytes, reward: int) -> bytes:
    # CScript() << 486604799 << CScriptNum(4) << timestamp
    script_sig = push_data(script_num(486604799)) + push_data(script_num(4)) + push_data(timestamp)
    # CScript() << pubkey << OP_CHECKSIG
    script_pubkey = push_data(pubkey) + b"\xac"
    tx = struct.pack("<I", 1)                                   # version
    tx += ser_compact_size(1)                                   # vin
    tx += b"\x00" * 32 + struct.pack("<I", 0xFFFFFFFF)          # null prevout
    tx += ser_compact_size(len(script_sig)) + script_sig
    tx += struct.pack("<I", 0xFFFFFFFF)                         # nSequence
    tx += ser_compact_size(1)                                   # vout
    tx += struct.pack("<q", reward)
    tx += ser_compact_size(len(script_pubkey)) + script_pubkey
    tx += struct.pack("<I", 0)                                  # nLockTime
    return tx


def bits_to_target(bits: int) -> int:
    exponent = bits >> 24
    mantissa = bits & 0x007FFFFF
    return mantissa << (8 * (exponent - 3))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--timestamp", required=True, help="pszTimestamp text")
    parser.add_argument("--pubkey", required=True, help="hex pubkey of the genesis output script")
    parser.add_argument("--time", type=int, required=True, help="nTime (unix)")
    parser.add_argument("--bits", type=lambda s: int(s, 0), required=True, help="nBits, e.g. 0x1e0ffff0")
    parser.add_argument("--version", type=int, default=1)
    parser.add_argument("--reward", type=int, default=50, help="genesis reward in whole coins")
    parser.add_argument("--nonce", type=int, default=0, help="first nonce to try")
    args = parser.parse_args()

    tx = coinbase_tx(args.timestamp.encode(), bytes.fromhex(args.pubkey), args.reward * 100_000_000)
    merkle = sha256d(tx)  # single transaction: merkle root == txid
    target = bits_to_target(args.bits)
    prefix = struct.pack("<i", args.version) + b"\x00" * 32 + merkle + struct.pack("<II", args.time, args.bits)

    nonce = args.nonce
    while nonce <= 0xFFFFFFFF:
        h = sha256d(prefix + struct.pack("<I", nonce))
        if int.from_bytes(h, "little") <= target:
            print(f"nTime       = {args.time}")
            print(f"nNonce      = {nonce}")
            print(f"nBits       = {args.bits:#010x}")
            print(f"genesis     = {h[::-1].hex()}")
            print(f"merkle root = {merkle[::-1].hex()}")
            return 0
        nonce += 1
    print("Nonce space exhausted; try another --time", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
