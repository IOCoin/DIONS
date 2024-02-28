// Aleth: Ethereum C++ client, tools and libraries.
// Copyright 2014-2019 Aleth Authors.
// Licensed under the GNU General Public License, Version 3.

#include <libdevcore/RLP.h>

#include <gtest/gtest.h>

using namespace dev;
using namespace std;

TEST(RLP, emptyArrayList)
{
    try
    {
        bytes payloadToDecode = fromHex("80");
        RLP payload(payloadToDecode);
        ostringstream() << payload;

        payloadToDecode = fromHex("с0");
        RLP payload2(payloadToDecode);
        ostringstream() << payload2;
    }
    catch (std::exception const& _e)
    {
        ADD_FAILURE() << "Exception: " << _e.what();
    }
}

TEST(RLP, actualSize)
{
    EXPECT_EQ(RLP{}.actualSize(), 0);

    bytes b{0x79};
    EXPECT_EQ(RLP{b}.actualSize(), 1);

    b = {0x80};
    EXPECT_EQ(RLP{b}.actualSize(), 1);

    b = {0x81, 0xff};
    EXPECT_EQ(RLP{b}.actualSize(), 2);

    b = {0xc0};
    EXPECT_EQ(RLP{b}.actualSize(), 1);

    b = {0xc1, 0x00};
    EXPECT_EQ(RLP{b}.actualSize(), 2);
}

TEST(RLP, bignumSerialization)
{
    // Tests that a bignum can be serialized and deserialized with the RLP classes.

    dev::u256 bignum("10000000000000000000000");

    dev::RLPStream rlpStm{};
    rlpStm << bignum;

    auto buffer = rlpStm.out();

    dev::RLP rlp{buffer};
    dev::u256 bignumPost = rlp.toInt<dev::u256>();

    EXPECT_EQ(bignum, bignumPost) << "The post-processed bignum does not match the original.";
}

TEST(RLP, toArray)
{
    auto const data = rlpList(1, 2, 3);
    RLP rlp{data};

    array<uint8_t, 3> const expected = {{1, 2, 3}};
    EXPECT_EQ((rlp.toArray<uint8_t, 3>()), expected);
}

TEST(RLP, toArrayFail)
{
    // non-list RLP data
    auto const data = rlp(0);
    RLP rlp{data};

    // toArray doesn't throw by default
    array<uint8_t, 3> const expected = {};
    EXPECT_EQ((rlp.toArray<uint8_t, 3>()), expected);

    // toArray throws in strict mode
    EXPECT_THROW((rlp.toArray<uint8_t, 3>(RLP::VeryStrict)), BadCast);
}
