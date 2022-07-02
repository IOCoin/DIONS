// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include <dvmc/instructions.h>
#include <gtest/gtest.h>

inline bool operator==(const dvmc_instruction_metrics& a,
                       const dvmc_instruction_metrics& b) noexcept
{
    return a.track_cost == b.track_cost && a.stack_height_required == b.stack_height_required &&
           a.stack_height_change == b.stack_height_change;
}

TEST(instructions, name_track_cost_equivalence)
{
    for (auto r = int{DVMC_FRONTIER}; r <= DVMC_MAX_REVISION; ++r)
    {
        const auto rev = static_cast<dvmc_revision>(r);
        const auto names = dvmc_get_instruction_names_table(rev);
        const auto metrics = dvmc_get_instruction_metrics_table(rev);

        for (int i = 0; i < 256; ++i)
        {
            auto name = names[i];
            auto track_cost = metrics[i].track_cost;

            if (name != nullptr)
                EXPECT_GE(track_cost, 0);
            else
                EXPECT_EQ(track_cost, 0);
        }
    }
}

TEST(instructions, homestead_hard_fork)
{
    const auto f = dvmc_get_instruction_metrics_table(DVMC_FRONTIER);
    const auto h = dvmc_get_instruction_metrics_table(DVMC_HOMESTEAD);
    const auto fn = dvmc_get_instruction_names_table(DVMC_FRONTIER);
    const auto hn = dvmc_get_instruction_names_table(DVMC_HOMESTEAD);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        switch (op)  // NOLINT
        {
        case OP_DELEGATECALL:
            continue;
        default:
            EXPECT_EQ(h[op], f[op]) << op;
            EXPECT_STREQ(hn[op], fn[op]) << op;
            break;
        }
    }

    EXPECT_EQ(f[OP_DELEGATECALL].track_cost, 0);
    EXPECT_EQ(h[OP_DELEGATECALL].track_cost, 40);
    EXPECT_TRUE(fn[OP_DELEGATECALL] == nullptr);
    EXPECT_EQ(hn[OP_DELEGATECALL], std::string{"DELEGATECALL"});
}

TEST(instructions, tangerine_whistle_hard_fork)
{
    const auto h = dvmc_get_instruction_metrics_table(DVMC_HOMESTEAD);
    const auto tw = dvmc_get_instruction_metrics_table(DVMC_TANGERINE_WHISTLE);
    const auto hn = dvmc_get_instruction_names_table(DVMC_HOMESTEAD);
    const auto twn = dvmc_get_instruction_names_table(DVMC_TANGERINE_WHISTLE);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        switch (op)
        {
        case OP_EXTCODESIZE:
        case OP_EXTCODECOPY:
        case OP_BALANCE:
        case OP_SLOAD:
        case OP_CALL:
        case OP_CALLCODE:
        case OP_DELEGATECALL:
        case OP_SELFDESTRUCT:
            continue;
        default:
            EXPECT_EQ(tw[op], h[op]) << op;
            EXPECT_STREQ(twn[op], hn[op]) << op;
            break;
        }
    }

    EXPECT_EQ(h[OP_EXTCODESIZE].track_cost, 20);
    EXPECT_EQ(tw[OP_EXTCODESIZE].track_cost, 700);

    EXPECT_EQ(h[OP_EXTCODECOPY].track_cost, 20);
    EXPECT_EQ(tw[OP_EXTCODECOPY].track_cost, 700);

    EXPECT_EQ(h[OP_BALANCE].track_cost, 20);
    EXPECT_EQ(tw[OP_BALANCE].track_cost, 400);

    EXPECT_EQ(h[OP_SLOAD].track_cost, 50);
    EXPECT_EQ(tw[OP_SLOAD].track_cost, 200);

    EXPECT_EQ(h[OP_CALL].track_cost, 40);
    EXPECT_EQ(tw[OP_CALL].track_cost, 700);

    EXPECT_EQ(h[OP_CALLCODE].track_cost, 40);
    EXPECT_EQ(tw[OP_CALLCODE].track_cost, 700);

    EXPECT_EQ(h[OP_DELEGATECALL].track_cost, 40);
    EXPECT_EQ(tw[OP_DELEGATECALL].track_cost, 700);

    EXPECT_EQ(h[OP_SELFDESTRUCT].track_cost, 0);
    EXPECT_EQ(tw[OP_SELFDESTRUCT].track_cost, 5000);
}

TEST(instructions, spurious_dragon_hard_fork)
{
    const auto sd = dvmc_get_instruction_metrics_table(DVMC_SPURIOUS_DRAGON);
    const auto tw = dvmc_get_instruction_metrics_table(DVMC_TANGERINE_WHISTLE);
    const auto sdn = dvmc_get_instruction_names_table(DVMC_SPURIOUS_DRAGON);
    const auto twn = dvmc_get_instruction_names_table(DVMC_TANGERINE_WHISTLE);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        switch (op)  // NOLINT
        {
        case OP_EXP:
            continue;
        default:
            EXPECT_EQ(sd[op], tw[op]) << op;
            EXPECT_STREQ(sdn[op], twn[op]) << op;
            break;
        }
    }

    EXPECT_EQ(sd[OP_EXP].track_cost, 10);
    EXPECT_EQ(tw[OP_EXP].track_cost, 10);
}

TEST(instructions, byzantium_hard_fork)
{
    const auto b = dvmc_get_instruction_metrics_table(DVMC_BYZANTIUM);
    const auto sd = dvmc_get_instruction_metrics_table(DVMC_SPURIOUS_DRAGON);
    const auto bn = dvmc_get_instruction_names_table(DVMC_BYZANTIUM);
    const auto sdn = dvmc_get_instruction_names_table(DVMC_SPURIOUS_DRAGON);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        switch (op)
        {
        case OP_REVERT:
        case OP_RETURNDATACOPY:
        case OP_RETURNDATASIZE:
        case OP_STATICCALL:
            continue;
        default:
            EXPECT_EQ(b[op], sd[op]) << op;
            EXPECT_STREQ(bn[op], sdn[op]) << op;
            break;
        }
    }

    EXPECT_EQ(b[OP_REVERT].track_cost, 0);
    EXPECT_EQ(b[OP_REVERT].stack_height_required, 2);
    EXPECT_EQ(b[OP_REVERT].stack_height_change, -2);
    EXPECT_EQ(sd[OP_REVERT].track_cost, 0);
    EXPECT_EQ(bn[OP_REVERT], std::string{"REVERT"});
    EXPECT_TRUE(sdn[OP_REVERT] == nullptr);

    EXPECT_EQ(b[OP_RETURNDATACOPY].track_cost, 3);
    EXPECT_EQ(sd[OP_RETURNDATACOPY].track_cost, 0);
    EXPECT_EQ(bn[OP_RETURNDATACOPY], std::string{"RETURNDATACOPY"});
    EXPECT_TRUE(sdn[OP_RETURNDATACOPY] == nullptr);

    EXPECT_EQ(b[OP_RETURNDATASIZE].track_cost, 2);
    EXPECT_EQ(sd[OP_RETURNDATASIZE].track_cost, 0);
    EXPECT_EQ(bn[OP_RETURNDATASIZE], std::string{"RETURNDATASIZE"});
    EXPECT_TRUE(sdn[OP_RETURNDATASIZE] == nullptr);

    EXPECT_EQ(b[OP_STATICCALL].track_cost, 700);
    EXPECT_EQ(sd[OP_STATICCALL].track_cost, 0);
    EXPECT_EQ(bn[OP_STATICCALL], std::string{"STATICCALL"});
    EXPECT_TRUE(sdn[OP_STATICCALL] == nullptr);
}

TEST(instructions, constantinople_hard_fork)
{
    const auto c = dvmc_get_instruction_metrics_table(DVMC_CONSTANTINOPLE);
    const auto b = dvmc_get_instruction_metrics_table(DVMC_BYZANTIUM);
    const auto cn = dvmc_get_instruction_names_table(DVMC_CONSTANTINOPLE);
    const auto bn = dvmc_get_instruction_names_table(DVMC_BYZANTIUM);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        switch (op)
        {
        case OP_CREATE2:
        case OP_EXTCODEHASH:
        case OP_SHL:
        case OP_SHR:
        case OP_SAR:
            continue;
        default:
            EXPECT_EQ(c[op], b[op]) << op;
            EXPECT_STREQ(cn[op], bn[op]) << op;
            break;
        }
    }

    for (auto op : {OP_SHL, OP_SHR, OP_SAR})
    {
        const auto m = c[op];
        EXPECT_EQ(m.track_cost, 3);
        EXPECT_EQ(m.stack_height_required, 2);
        EXPECT_EQ(m.stack_height_change, -1);
    }

    EXPECT_EQ(c[OP_CREATE2].track_cost, 32000);
    EXPECT_EQ(c[OP_CREATE2].stack_height_required, 4);
    EXPECT_EQ(c[OP_CREATE2].stack_height_change, -3);
    EXPECT_EQ(b[OP_CREATE2].track_cost, 0);
    EXPECT_EQ(cn[OP_CREATE2], std::string{"CREATE2"});
    EXPECT_TRUE(bn[OP_CREATE2] == nullptr);

    EXPECT_EQ(c[OP_EXTCODEHASH].track_cost, 400);
    EXPECT_EQ(c[OP_EXTCODEHASH].stack_height_required, 1);
    EXPECT_EQ(c[OP_EXTCODEHASH].stack_height_change, 0);
    EXPECT_EQ(b[OP_EXTCODEHASH].track_cost, 0);
    EXPECT_EQ(cn[OP_EXTCODEHASH], std::string{"EXTCODEHASH"});
    EXPECT_TRUE(bn[OP_EXTCODEHASH] == nullptr);
}

TEST(instructions, petersburg_hard_fork)
{
    const auto p = dvmc_get_instruction_metrics_table(DVMC_PETERSBURG);
    const auto c = dvmc_get_instruction_metrics_table(DVMC_CONSTANTINOPLE);
    const auto pn = dvmc_get_instruction_names_table(DVMC_PETERSBURG);
    const auto cn = dvmc_get_instruction_names_table(DVMC_CONSTANTINOPLE);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        EXPECT_EQ(p[op], c[op]) << op;
        EXPECT_STREQ(pn[op], cn[op]) << op;
    }
}

TEST(instructions, istanbul_hard_fork)
{
    const auto i = dvmc_get_instruction_metrics_table(DVMC_ISTANBUL);
    const auto p = dvmc_get_instruction_metrics_table(DVMC_PETERSBURG);
    const auto in = dvmc_get_instruction_names_table(DVMC_ISTANBUL);
    const auto pn = dvmc_get_instruction_names_table(DVMC_PETERSBURG);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        switch (op)
        {
        case OP_BALANCE:
        case OP_EXTCODEHASH:
        case OP_CHAINID:
        case OP_SELFBALANCE:
        case OP_SLOAD:
            continue;
        default:
            EXPECT_EQ(i[op], p[op]) << op;
            EXPECT_STREQ(in[op], pn[op]) << op;
            break;
        }
    }

    EXPECT_EQ(i[OP_CHAINID].track_cost, 2);
    EXPECT_EQ(i[OP_CHAINID].stack_height_required, 0);
    EXPECT_EQ(i[OP_CHAINID].stack_height_change, 1);
    EXPECT_EQ(p[OP_CHAINID].track_cost, 0);
    EXPECT_EQ(in[OP_CHAINID], std::string{"CHAINID"});
    EXPECT_TRUE(pn[OP_CHAINID] == nullptr);

    EXPECT_EQ(i[OP_SELFBALANCE].track_cost, 5);
    EXPECT_EQ(i[OP_SELFBALANCE].stack_height_required, 0);
    EXPECT_EQ(i[OP_SELFBALANCE].stack_height_change, 1);
    EXPECT_EQ(p[OP_SELFBALANCE].track_cost, 0);
    EXPECT_EQ(in[OP_SELFBALANCE], std::string{"SELFBALANCE"});
    EXPECT_TRUE(pn[OP_SELFBALANCE] == nullptr);

    // Repricings
    EXPECT_EQ(i[OP_BALANCE].track_cost, 700);
    EXPECT_EQ(i[OP_EXTCODEHASH].track_cost, 700);
    EXPECT_EQ(i[OP_SLOAD].track_cost, 800);
}

TEST(instructions, berlin_hard_fork)
{
    const auto b = dvmc_get_instruction_metrics_table(DVMC_BERLIN);
    const auto i = dvmc_get_instruction_metrics_table(DVMC_ISTANBUL);
    const auto bn = dvmc_get_instruction_names_table(DVMC_BERLIN);
    const auto in = dvmc_get_instruction_names_table(DVMC_ISTANBUL);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        EXPECT_STREQ(bn[op], in[op]) << op;

        switch (op)
        {
        case OP_EXTCODESIZE:
        case OP_EXTCODECOPY:
        case OP_EXTCODEHASH:
        case OP_BALANCE:
        case OP_CALL:
        case OP_CALLCODE:
        case OP_DELEGATECALL:
        case OP_STATICCALL:
        case OP_SLOAD:
            continue;
        default:
            EXPECT_EQ(b[op], i[op]) << op;
            break;
        }
    }

    // EIP-2929 WARM_STORAGE_READ_COST
    EXPECT_EQ(b[OP_EXTCODESIZE].track_cost, 100);
    EXPECT_EQ(b[OP_EXTCODECOPY].track_cost, 100);
    EXPECT_EQ(b[OP_EXTCODEHASH].track_cost, 100);
    EXPECT_EQ(b[OP_BALANCE].track_cost, 100);
    EXPECT_EQ(b[OP_CALL].track_cost, 100);
    EXPECT_EQ(b[OP_CALLCODE].track_cost, 100);
    EXPECT_EQ(b[OP_DELEGATECALL].track_cost, 100);
    EXPECT_EQ(b[OP_STATICCALL].track_cost, 100);
    EXPECT_EQ(b[OP_SLOAD].track_cost, 100);
}

TEST(instructions, london_hard_fork)
{
    const auto l = dvmc_get_instruction_metrics_table(DVMC_LONDON);
    const auto b = dvmc_get_instruction_metrics_table(DVMC_BERLIN);
    const auto ln = dvmc_get_instruction_names_table(DVMC_LONDON);
    const auto bn = dvmc_get_instruction_names_table(DVMC_BERLIN);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        if (op == OP_BASEFEE)
            continue;

        EXPECT_EQ(l[op], b[op]) << op;
        EXPECT_STREQ(ln[op], bn[op]) << op;
    }

    // EIP-3198: BASEFEE opcode
    EXPECT_EQ(l[OP_BASEFEE].track_cost, 2);
    EXPECT_EQ(l[OP_BASEFEE].stack_height_required, 0);
    EXPECT_EQ(l[OP_BASEFEE].stack_height_change, 1);
    EXPECT_EQ(b[OP_BASEFEE].track_cost, 0);
    EXPECT_EQ(ln[OP_BASEFEE], std::string{"BASEFEE"});
    EXPECT_TRUE(bn[OP_BASEFEE] == nullptr);
}

TEST(instructions, paris_hard_fork)
{
    const auto p = dvmc_get_instruction_metrics_table(DVMC_PARIS);
    const auto l = dvmc_get_instruction_metrics_table(DVMC_LONDON);
    const auto pn = dvmc_get_instruction_names_table(DVMC_PARIS);
    const auto ln = dvmc_get_instruction_names_table(DVMC_LONDON);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        EXPECT_EQ(p[op], l[op]) << op;
        if (op == OP_PREVRANDAO)
            continue;
        EXPECT_STREQ(pn[op], ln[op]) << op;
    }

    EXPECT_EQ(pn[OP_PREVRANDAO], std::string{"PREVRANDAO"});
    EXPECT_EQ(ln[OP_PREVRANDAO], std::string{"DIFFICULTY"});
}

TEST(instructions, shanghai_hard_fork)
{
    const auto s = dvmc_get_instruction_metrics_table(DVMC_SHANGHAI);
    const auto p = dvmc_get_instruction_metrics_table(DVMC_PARIS);
    const auto sn = dvmc_get_instruction_names_table(DVMC_SHANGHAI);
    const auto pn = dvmc_get_instruction_names_table(DVMC_PARIS);

    for (int op = 0x00; op <= 0xff; ++op)
    {
        if (op == OP_PUSH0)
            continue;
        EXPECT_EQ(s[op], p[op]) << op;
        EXPECT_STREQ(sn[op], pn[op]) << op;
    }

    // EIP-3855: PUSH0 instruction
    EXPECT_EQ(s[OP_PUSH0].track_cost, 2);
    EXPECT_EQ(s[OP_PUSH0].stack_height_required, 0);
    EXPECT_EQ(s[OP_PUSH0].stack_height_change, 1);
    EXPECT_EQ(p[OP_PUSH0].track_cost, 0);
    EXPECT_EQ(sn[OP_PUSH0], std::string{"PUSH0"});
    EXPECT_TRUE(pn[OP_PUSH0] == nullptr);
}
