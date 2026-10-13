// Copyright (c) 2026 The Bitcurrency developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// Checks the Bitcurrency-specific chain parameters: genesis blocks, network
// identity, emission schedule and difficulty retargeting (including BIP94).

#include <arith_uint256.h>
#include <chain.h>
#include <chainparams.h>
#include <consensus/amount.h>
#include <kernel/messagestartchars.h>
#include <pow.h>
#include <test/util/setup_common.h>
#include <uint256.h>
#include <util/chaintype.h>
#include <validation.h>

#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

BOOST_FIXTURE_TEST_SUITE(bitcurrency_tests, BasicTestingSetup)

struct ExpectedNetwork {
    ChainType chain;
    std::string genesis;
    uint16_t port;
    std::string hrp;
    MessageStartChars magic;
};

BOOST_AUTO_TEST_CASE(network_identity)
{
    const std::vector<ExpectedNetwork> networks{
        {ChainType::MAIN, "000008e43e5a666655a25fcca876dd75be95b7983a8ad01ea940c5ae9f341971", 7333, "bcy", {0xe3, 0xc1, 0xa7, 0xd5}},
        {ChainType::TESTNET, "00000b490a130b92169a5df76c875528a5ff766c304a8df6c9f9cbbbd8c81f0b", 17333, "tbcy", {0x0c, 0x12, 0x0a, 0x08}},
        {ChainType::TESTNET4, "00000cc319b22028c3693b1752c96c1dfdc361aa24e8ce4cd15aaea324293122", 47333, "tbcy", {0x1d, 0x17, 0x3e, 0x29}},
        {ChainType::SIGNET, "0000035337c183df6381bb32a867d5f3260dda1282320ca2f3330290040b2efa", 37333, "tbcy", {0x72, 0x81, 0xdf, 0x37}},
        {ChainType::REGTEST, "6e4181f65f8b53bfa47582d31e732406d83917581d5236875290ce205848f8ed", 17444, "bcyrt", {0xe4, 0xc2, 0xa8, 0xd6}},
    };
    for (const auto& net : networks) {
        const auto params = CreateChainParams(*m_node.args, net.chain);
        BOOST_CHECK_EQUAL(params->GenesisBlock().GetHash().GetHex(), net.genesis);
        BOOST_CHECK_EQUAL(params->GetConsensus().hashGenesisBlock.GetHex(), net.genesis);
        BOOST_CHECK_EQUAL(params->GetDefaultPort(), net.port);
        BOOST_CHECK_EQUAL(params->Bech32HRP(), net.hrp);
        BOOST_CHECK(params->MessageStart() == net.magic);
        // Genesis must satisfy its own proof of work
        BOOST_CHECK(CheckProofOfWork(params->GenesisBlock().GetHash(), params->GenesisBlock().nBits, params->GetConsensus()));
    }
}

BOOST_AUTO_TEST_CASE(mainnet_consensus_parameters)
{
    const auto params = CreateChainParams(*m_node.args, ChainType::MAIN);
    const Consensus::Params& consensus = params->GetConsensus();
    BOOST_CHECK_EQUAL(consensus.nPowTargetSpacing, 150);
    BOOST_CHECK_EQUAL(consensus.nPowTargetTimespan, 84 * 60 * 60);
    BOOST_CHECK_EQUAL(consensus.DifficultyAdjustmentInterval(), 2016);
    BOOST_CHECK_EQUAL(consensus.nSubsidyHalvingInterval, 840000);
    BOOST_CHECK(consensus.enforce_BIP94);
    BOOST_CHECK(!consensus.fPowAllowMinDifficultyBlocks);
    BOOST_CHECK(consensus.script_flag_exceptions.empty());
    BOOST_CHECK_EQUAL(consensus.SegwitHeight, 1);
    BOOST_CHECK_EQUAL(consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime, Consensus::BIP9Deployment::ALWAYS_ACTIVE);
    BOOST_CHECK_EQUAL(params->TxData().tx_count, 0U);
    BOOST_CHECK(params->GetAvailableSnapshotHeights().empty());
}

BOOST_AUTO_TEST_CASE(emission_schedule)
{
    const auto params = CreateChainParams(*m_node.args, ChainType::MAIN);
    const Consensus::Params& consensus = params->GetConsensus();
    BOOST_CHECK_EQUAL(GetBlockSubsidy(0, consensus), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(839'999, consensus), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(840'000, consensus), 25 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(1'680'000, consensus), 25 * COIN / 2);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(64 * 840'000, consensus), 0);

    // Sum the whole emission, one halving era at a time.
    CAmount total{0};
    for (int era = 0; era < 64; ++era) {
        total += GetBlockSubsidy(era * 840'000, consensus) * 840'000;
        BOOST_CHECK(MoneyRange(total));
    }
    BOOST_CHECK_EQUAL(total, CAmount{8'399'999'990'760'000}); // 83,999,999.9076 BCY
    BOOST_CHECK(total <= MAX_MONEY);
    BOOST_CHECK_EQUAL(MAX_MONEY, 84'000'000 * COIN);
}

/** Build a full retarget period (heights 0..2015) with evenly spaced timestamps. */
static std::vector<CBlockIndex> MakePeriod(const Consensus::Params& consensus, int64_t timespan, uint32_t first_bits, uint32_t last_bits)
{
    const int interval = consensus.DifficultyAdjustmentInterval();
    std::vector<CBlockIndex> blocks(interval);
    const int64_t start = 1'791'849'600;
    for (int i = 0; i < interval; ++i) {
        blocks[i].pprev = i ? &blocks[i - 1] : nullptr;
        blocks[i].nHeight = i;
        blocks[i].nTime = start + timespan * i / (interval - 1);
        blocks[i].nBits = i == 0 ? first_bits : last_bits;
        blocks[i].BuildSkip();
    }
    return blocks;
}

BOOST_AUTO_TEST_CASE(retarget)
{
    const auto params = CreateChainParams(*m_node.args, ChainType::MAIN);
    const Consensus::Params& consensus = params->GetConsensus();
    const uint32_t genesis_bits = params->GenesisBlock().nBits;
    BOOST_CHECK_EQUAL(genesis_bits, 0x1e0ffff0U);
    const int64_t target = consensus.nPowTargetTimespan;

    // Height 2016 is the first adjustment; anything before keeps the bits.
    {
        auto blocks = MakePeriod(consensus, target, genesis_bits, genesis_bits);
        CBlockHeader next;
        next.nTime = blocks[2014].nTime + 150;
        BOOST_CHECK_EQUAL(GetNextWorkRequired(&blocks[2014], &next, consensus), genesis_bits);
    }

    // Exactly on schedule: difficulty unchanged.
    {
        auto blocks = MakePeriod(consensus, target, genesis_bits, genesis_bits);
        CBlockHeader next;
        BOOST_CHECK_EQUAL(GetNextWorkRequired(&blocks.back(), &next, consensus), genesis_bits);
    }

    // Blocks 4x too fast: target divided by 4 (difficulty x4).
    // Blocks 10x too fast: clamped to the same x4 step.
    for (const int64_t timespan : {target / 4, target / 10}) {
        auto blocks = MakePeriod(consensus, timespan, genesis_bits, genesis_bits);
        CBlockHeader next;
        const uint32_t new_bits = GetNextWorkRequired(&blocks.back(), &next, consensus);
        BOOST_CHECK_EQUAL(new_bits, 0x1e03fffcU);
        BOOST_CHECK(PermittedDifficultyTransition(consensus, 2016, genesis_bits, new_bits));
    }

    // Blocks too slow at minimum difficulty: never easier than powLimit.
    // target * 4 * timespan exceeds 256 bits here; this used to wrap around.
    for (const int64_t timespan : {target * 2, target * 4, target * 10}) {
        auto blocks = MakePeriod(consensus, timespan, genesis_bits, genesis_bits);
        CBlockHeader next;
        const uint32_t new_bits = GetNextWorkRequired(&blocks.back(), &next, consensus);
        BOOST_CHECK_EQUAL(new_bits, UintToArith256(consensus.powLimit).GetCompact());
        BOOST_CHECK(PermittedDifficultyTransition(consensus, 2016, genesis_bits, new_bits));
    }

    // Blocks 4x too slow from 4x the minimum difficulty: back to the genesis target.
    {
        auto blocks = MakePeriod(consensus, target * 4, 0x1e03fffc, 0x1e03fffc);
        CBlockHeader next;
        const uint32_t new_bits = GetNextWorkRequired(&blocks.back(), &next, consensus);
        BOOST_CHECK_EQUAL(new_bits, genesis_bits);
        BOOST_CHECK(PermittedDifficultyTransition(consensus, 2016, 0x1e03fffc, new_bits));
    }

    // BIP94: the new target is derived from the FIRST block of the period,
    // not from the last one.
    {
        auto blocks = MakePeriod(consensus, target, /*first_bits=*/0x1e03fffc, /*last_bits=*/genesis_bits);
        CBlockHeader next;
        BOOST_CHECK_EQUAL(GetNextWorkRequired(&blocks.back(), &next, consensus), 0x1e03fffcU);
    }

    // A transition bigger than 4x is not permitted.
    BOOST_CHECK(!PermittedDifficultyTransition(consensus, 2016, genesis_bits, 0x1e01fffeU));
}

BOOST_AUTO_TEST_SUITE_END()
