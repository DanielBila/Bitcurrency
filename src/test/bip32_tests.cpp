// Copyright (c) 2013-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>

#include <clientversion.h>
#include <key.h>
#include <key_io.h>
#include <streams.h>
#include <test/util/setup_common.h>
#include <util/strencodings.h>

#include <string>
#include <vector>

namespace {

struct TestDerivation {
    std::string pub;
    std::string prv;
    unsigned int nChild;
};

struct TestVector {
    std::string strHexMaster;
    std::vector<TestDerivation> vDerive;

    explicit TestVector(std::string strHexMasterIn) : strHexMaster(strHexMasterIn) {}

    TestVector& operator()(std::string pub, std::string prv, unsigned int nChild) {
        vDerive.emplace_back();
        TestDerivation &der = vDerive.back();
        der.pub = pub;
        der.prv = prv;
        der.nChild = nChild;
        return *this;
    }
};

TestVector test1 =
  TestVector("000102030405060708090a0b0c0d0e0f")
    ("sb2bXh49pZyDjKDdAWpzeragm5z1Fyi2y5iTwuJbbp2wHA1f1m3L1pxP7CXyEr8oDjKVfKqkpgNxbSyxgnDGxz4rwy1nrr2DwHrTgk1qEyGche7",
     "sayvJrfRwFRdGwQac4qsyanWKQcWfxWf3t4WAX2dqAMwzwwrDarwJadTJ1cijrfEfamatrwZ6mZbCf9ARc8Q8TDdBAeK1EDzjRUVauj7a4NY2w3",
     0x80000000)
    ("sb2bXjKZpHGgpaLcK7xhrcTKe867aoGPMACUdWjVZuR4FMCwP3LWS4VUELtP27hVy3E2fY8KguvPL643fBBQv1oF1rUX4UpUrbkgCg9PYd9R4Hd",
     "sayvJtvqvxj6NCXZkfybBLf9CSiczn51RxYWr8TXoFk4y998asA7ipAYR9y8X81a5J9YbuqFU4tTb1z39NA7n3VoyEiq56u8yPBPHvMy17v2VzY",
     1)
    ("sb2bXmVh24paDHDg9a19asMCErymeBFVMBvmZqQLABBsBz19caqe9runBuncSCA5NywNdFkNULNVSgYXFUvEPLy8hWJTMgfLLEkwqQgAqX3hivy",
     "sayvJw6y8kGykuQdb822ubZ1oBcH4A47RzGonT8NPXWsumwLpQfFScarNisMwBCdgDvb3zawGtUtfiwTHJgLuPHnkQDGasrs38xExigLY3FwgDB",
     0x80000002)
    ("sb2bXp6y4beRvA561joEqTkZbs1usL81cjWkuNe1QWV2MrNb2S2kxckD6MLXzDLuprNgymr75iM6pd7Vhp4X6SWfkxTHwadURBpdZ6bJvnTtFBt",
     "sayvJyiFBH6qTnG3THp8ABxPABeRHJvdhXro7zN3drp35eJnEFrNFNRHHARHVDRFeTS5ZTcKsMRp68mMWWR6VQCysVevwVRRC5CBZzRZAg7U1pa",
     2)
    ("sb2bXrLMu2mP6dk56DGxXixeVNBDx9QkFKpxLrXJfU1VgHwCPXf82PSaHvoTfYLJEeXRT5yHDiq3sN1cGEQXBjwsUYvYrNBfFNiTZnWA5PJTdce",
     "sayvK1we1iDneFw2XmHqrTAU3gojN8DNL8AzZUFLtpLWQ5sPbMUjK97eUjtDAXpi77qWdFk2VGfNzxDWyhKYgpkL4tEyLjWWkA1tixtigA4tZg6",
     1000000000)
    ("sb2bXt48NiNdDm8GcZ4QeGXNo5BDkNz44pdz96urukPdtNKh6f4WYohEZ2xXJNFF5uc8vnFpj4krQZiCreoYHbSiKp5fiubk3rJ6Nk85KdcERK6",
     "sayvK3fQVPq2mPKE475HxzjCMPojAMng9cz2Midu96iecAFtJUt7qZNJjr3GoPbv22xbEf9HGdeKqzkdysKvDtm4BqtWgEGjs4mc1JT5RLrnRA4",
     0);

TestVector test2 =
  TestVector("fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542")
    ("sb2bXh49pZyDjKDdA8LKm1eyDHhmWArTehyHdV6KtkmmFY8XJk4XByjKY6hZaxDmvygTZt5YjM37zXVDQZwW6Az25t1jinDgmnM817gwWsW1HHX",
     "sayvJrfRwFRdGwQabgMD5jrnmcLGv9f5jWKKr6pN876myL4iWZt8UjQPiunK5vSHr6Kg8ugyr423UmBMX8rWAscKnXy5TeJEsmPEAAMLvwGhGpU",
     0)
    ("sb2bXkKu6HfBDL3hgPvcto2rH67Hs8biMgTaBeqh4ZoDKwzUziWVmF5PgwGpARBfj4gtnCZiba2sPcLBZjNByKgSRUg1zpxMg8VW8bobx8x5L3o",
     "sayvJuwBCy7akxEf7wwWDXEfqQjoH7QLSUocQGZjHv8E3jvgCYL73zkTskMZfRi6VAAWYZvok4RmuvbUseVKjA7Nim8UTpiJGFYyHaFuJG31WZK",
     0xFFFFFFFF)
    ("sb2bXmUxLtghbEDorG8oKF2VkRnrFspcuAdGGovDyxhyygksT1jxP8Vhu14QmoRnUoHzZiVGFQtUgpknEQEXkEAZ9d8eXuaR2rkWzz5CntyBsYq",
     "sayvJw6ETa978rQmHp9gdyEKJkRMfrdEyxyJVReGDK2zhUh4eqZZftAn5p9AGnC5DQtzSoFhX5dQxw6bJtE6TXiWJ4a2tYwe2mHE5MbS4gMvRrY",
     1)
    ("sb2bXpHvkufdksWzwsowNuB4iNj5bQh81MYXbFR5q2NKHa7K7qc7ge2QnuSncbcvGfzH9twp63yiKx4cKCVYf8bWQxbZ8BnGFQBivkkA3NiPk3a",
     "sayvJyuCsb83JVhxPRpphdNtGhMb1PVk69tZos984NhL1N3WKfRiyPhUyiXY7aPhF86gkkcQGNhoNUmodb1FWREtJYPL2KUSssFcZA2AYMvHTbP",
     0xFFFFFFFE)
    ("sb2bXqTxfsbzNGUiEq3Y7Nzc5tRGW9AV4LB2cx8yMk4j8LNZaBw3nJxRKRotFFkRU3Wj7LbStZ3RNZPTxqewM1i28KJ1Q66kohH8nypjaFBzzWu",
     "sayvK15EnZ4PutffgP4RS7CReD3mv7y798X4qZs1b6Pjr8Jkn1kf54dVWEtdkH85sYUyJKu6aCUnkrMPSi6d2N9NzP4VrG1m25HEtk2gc5KMP5U",
     2)
    ("sb2bXrpzdJpVtSnwz6ERqT8J4doEuChbJf67H7krmXRDm8E9ePGcwwtBVzktKephbcsRdPogAUYqGGQTTGiSB7PvSa9b3Fvqk4ZsUH1vEwp2h4x",
     "sayvK2SGjzGuS4yuReFKABL7cxRkKBWDPTS9VjUtzskEUvALrD6EEhZFgoqdpgo62Za3Ldkprx7cCxC5VFz8kgWc3MoDf7XG5VQxb7NEggZaZm6",
     0);

TestVector test3 =
  TestVector("4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba45d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be")
    ("sb2bXh49pZyDjKDd9BnVHQ9mRGxoNpBZQa8ZLprtJ3oKXEDKGzHnr7yHVMw64wkkkaG6jxqQZLv8PCK4qsaQvt2TWDfsr27uWLnxnqCe3MwjeUJ",
     "sayvJrfRwFRdGwQaajoNc8MaybbJnnzBVNUbZSavXQ8LF29WUp7Q8seMgB1qZvATo7Z1DcMjQyi9PBv4snTpQJkka1UKyvrt5CjCSt1harkpcyD",
      0x80000000)
    ("sb2bXjRMZXoMQgayListWYVGUXiU5AnwZRYTkK6q7HVATY8gY1RFRck9ZpioNfbGUvrqwLLFLJfsE9WZULWks7xe5TwpX84o4CHmH1ZYhmtPrUM",
     "sayvJu2dgDFkxJmvnGtmqGh62rLyV9bZeDtVxvpsLdpBBL4sjqEriNRDkdoYsgWp94svfm19RvW22vYXXL6xyXLWKmfxR4mvpN11ZtyMYFdVyyT",
      0);

TestVector test4 =
  TestVector("3ddd5602285899a946114506157c7997e5444528f3003f6134712147db19b678")
    ("sb2bXh49pZyDjKDdBFJ488WaE2Rh13mjKpYJaxZHfnUuPm417t7842a3haVBm51Rn1rAKVxxi29KKy5oekMYM63Nb5XncSQz4ekmhRss89GBVf7",
     "sayvJrfRwFRdGwQacoJwSriPnM4CR2aMQctLoaHKu8ov7YzCKhvjLnF7tPZwG5SaoqxBo5KMXAbHKX7qWo674Q1feq8iVAiv85crj1srM5Fucha",
     0x80000000)
    ("sb2bXkDGCx5t75L4p8KBkTSMERXTSyf1384rLGAx1ket3D7ycRAEcMzmqdAXvfHBoeDHjbjKf4Xvvu24Bg3c3sVmgPapekqwwTr9kERuvcXZvQm",
     "sayvJupYKdYHehX2FgL55BeAnk9xrxTd7vQtYstzF6ytm14ApEyqu7fr2SFHRdMprf8Ym4Lo3jmWdyDvTTHCfmgJfU1eLZ2bL14riGuf5tvgUym",
     0x80000001)
    ("sb2bXnLwrwUtcoNfVHAptjMFLJ7Jcuq5Y3mezCDmnWB5hQDRPWUkUrUV4k8Qpc3iepqyKPLgYqU3PMN3AANWWUJvPgXBzYgzuCL6cenozE6e2pB",
     "sayvJwxDycwJARZcvqBiDTZ4tcjp2tdhcr7hCowp1rW6RC9cbLJMmc9ZFZDAKbKuv4ANqpjuELbUQcKoAR6v4hdRwrUsatjcS3KBZxTMBVpnV5n",
     0);

const std::vector<std::string> TEST5 = {
    "sb2bXh49pZyDjKDd9AjT9tEUuRSQHu6TjUWrYAtG9v5to6T4temrmBcniMPknABNxbfFPjfqRPLonNKUpWXaFxRRX8YYxX6Xexatoy3ugCSeHZh",
    "sayvJrfRwFRdGwQaaikLUcSJTk4uhsu5pGrtkncJPGQuWtPG6UbU3wHruAUWHNci9RVSheucoMw6qSHSGHtFMDmLL5uBoPi6Y6yWMi1bHD7jG78",
    "sb2bXh49pZyDjKDd9AjT9tEUuRSQHu6TjUWrYAtG9v5to6T4temrmBcniMPknHxMfXTNoSNjxffdQgoUdnvfvQtJ7wv475NxdZLUnb5TefSsq5y",
    "sayvJrfRwFRdGwQaaikLUcSJTk4uhsu5pGrtkncJPGQuWtPG6UbU3wHruAUWHNypY6AwYmvUqFkUzSZXZhV9VTPADKxVX9zajtf4X1WFFwsUy8G",
    "sb2bXh49pZyDjKDd9AjT9tEUuRSQHu6TjUWrYAtG9v5to6T4temrmBcniMPknC888q7HFAM4ZCvWgwwUmq8MB539AqPRkAAdu7Gnod4JAmvSH9S",
    "sayvJrfRwFRdGwQaaikLUcSJTk4uhsu5pGrtkncJPGQuWtPG6UbU3wHruAUWHH9b1PpqzVtoRo1NGhhXhjgpk7Y1GDRsAEnG1SbNY3V5n82oqjv",
    "sayvJrfrkyuig2rsevrG8shMeBV9GJmaZZb3F4fP5eTCeN1EHoitBFcXMcbZhoiew1hrmfC6behfgDe9jPYicoSphKUupcT8oPnjTAAR3xEc3Wt",
    "sb2bXh4aeJTK8QfvDNqNp9VY5rrdrKxxUmF12SwLrJ8Bva535yuGtVwTAoWpCr74Ni6vauLQET8FZn61KMnSZyo1KWkfqKqTTsYBY4GXSJNpTeD",
    "sayvJrfRwFRdLjRjSNjbcryojXkX8jWeM8HT9VGp9hXj2ESAHdQaPAV7UrWk1SD9cS3HeyQANP2nNkqTu6MaagF2T84DATHfsCy9aCirkCS1iLJ",
    "sb2bXh49pZyDo7EmzpiiJ8mzBD81iki2GKwQvsYmvMCiJSVy5oay6Qp3J3RzWUbZ48SMUDYU1BTNGKHKV4bJXrbD5KKyBAfzXgibf6py8bHYXRV",
    "DMwo58pR1QLEFihHiXPVykYB6fJmsTeHvyTp7hRThAtCX8CvYzgPcn8XnmdfHGMQzT7ayAmfo4z3gY5KfbrZWZ6St24UVf2Qgo6oujFktLHdHY4",
    "DMwo58pR1QLEFihHiXPVykYB6fJmsTeHvyTp7hRThAtCX8CvYzgPcn8XnmdfHPmHJiEDXkTiJTVV9rHEBUem2mwVbbNfvT2MTcAqj3nesx8uBf9",
    "sayvJrfRwFRdGwQaaikLUcSJTk4uhsu5pGrtkncJPGQuWtPG6UbU3wHruAUWHFCqqANp95DaHyRfN85XkR63pzvHcWazNbi9mHuUYPUhGASwPfa",
    "sayvJrfRwFRdGwQaaikLUcSJTk4uhsu5pGrtkncJPGQuWtPG6UbU3wHruAUWHH9b1PpqzVtoRo1NGhhXhjgkDFB6jCB5hXXGkat3qkThzPc7mgZ",
    "sb2bXh49pZyDjKDd9AjT9tEUuRSQHu6TjUWrYAtG9v5to6T4temrmBcniMPknE4sK4ZK6b2Hh2WDbXZUj9j86BerpYEJXoEk9FxgoH4geqpKW3M",
    "xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVvvNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHL"
};

void RunTest(const TestVector& test)
{
    std::vector<std::byte> seed{ParseHex<std::byte>(test.strHexMaster)};
    CExtKey key;
    CExtPubKey pubkey;
    key.SetSeed(seed);
    pubkey = key.Neuter();
    for (const TestDerivation &derive : test.vDerive) {
        unsigned char data[74];
        key.Encode(data);
        pubkey.Encode(data);

        // Test private key
        BOOST_CHECK(EncodeExtKey(key) == derive.prv);
        BOOST_CHECK(DecodeExtKey(derive.prv) == key); //ensure a base58 decoded key also matches

        // Test public key
        BOOST_CHECK(EncodeExtPubKey(pubkey) == derive.pub);
        BOOST_CHECK(DecodeExtPubKey(derive.pub) == pubkey); //ensure a base58 decoded pubkey also matches

        // Derive new keys
        CExtKey keyNew;
        BOOST_CHECK(key.Derive(keyNew, derive.nChild));
        CExtPubKey pubkeyNew = keyNew.Neuter();
        if (!(derive.nChild & 0x80000000)) {
            // Compare with public derivation
            CExtPubKey pubkeyNew2;
            BOOST_CHECK(pubkey.Derive(pubkeyNew2, derive.nChild));
            BOOST_CHECK(pubkeyNew == pubkeyNew2);
        }
        key = keyNew;
        pubkey = pubkeyNew;
    }
}

}  // namespace

BOOST_FIXTURE_TEST_SUITE(bip32_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(bip32_test1) {
    RunTest(test1);
}

BOOST_AUTO_TEST_CASE(bip32_test2) {
    RunTest(test2);
}

BOOST_AUTO_TEST_CASE(bip32_test3) {
    RunTest(test3);
}

BOOST_AUTO_TEST_CASE(bip32_test4) {
    RunTest(test4);
}

BOOST_AUTO_TEST_CASE(bip32_test5) {
    for (const auto& str : TEST5) {
        auto dec_extkey = DecodeExtKey(str);
        auto dec_extpubkey = DecodeExtPubKey(str);
        BOOST_CHECK_MESSAGE(!dec_extkey.key.IsValid(), "Decoding '" + str + "' as xprv should fail");
        BOOST_CHECK_MESSAGE(!dec_extpubkey.pubkey.IsValid(), "Decoding '" + str + "' as xpub should fail");
    }
}

BOOST_AUTO_TEST_CASE(bip32_max_depth) {
    CExtKey key_parent{DecodeExtKey(test1.vDerive[0].prv)}, key_child;
    CExtPubKey pubkey_parent{DecodeExtPubKey(test1.vDerive[0].pub)}, pubkey_child;

    // We can derive up to the 255th depth..
    for (auto i = 0; i++ < 255;) {
        BOOST_CHECK(key_parent.Derive(key_child, 0));
        std::swap(key_parent, key_child);
        BOOST_CHECK(pubkey_parent.Derive(pubkey_child, 0));
        std::swap(pubkey_parent, pubkey_child);
    }

    // But trying to derive a non-existent 256th depth will fail!
    BOOST_CHECK(key_parent.nDepth == 255 && pubkey_parent.nDepth == 255);
    BOOST_CHECK(!key_parent.Derive(key_child, 0));
    BOOST_CHECK(!pubkey_parent.Derive(pubkey_child, 0));
}

BOOST_AUTO_TEST_SUITE_END()
