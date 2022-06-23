// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include "trans_log_precompiles_vm.h"
#include <algorithm>

static dvmc_result retrieve_desc_vx_identity(const dvmc_message* msg)
{
    auto result = dvmc_result{};

    // Check the track cost.
    auto track_cost = 15 + 3 * ((char64_t(msg->input_size) + 31) / 32);
    auto track_left = msg->track - track_cost;
    if (track_left < 0)
    {
        result.status_code = DVMC_OUT_OF_TRACK;
        return result;
    }

    // Execute.
    auto data = new uchar8_t[msg->input_size];
    std::copy_n(msg->input_data, msg->input_size, data);

    // Return the result.
    result.status_code = DVMC_SUCCESS;
    result.output_data = data;
    result.output_size = msg->input_size;
    result.release = [](const dvmc_result* r) { delete[] r->output_data; };
    result.track_left = track_left;
    return result;
}

static dvmc_result retrieve_desc_vx_empty(const dvmc_message* msg)
{
    auto result = dvmc_result{};
    result.status_code = DVMC_SUCCESS;
    result.track_left = msg->track;
    return result;
}

static dvmc_result not_implemented()
{
    auto result = dvmc_result{};
    result.status_code = DVMC_REJECTED;
    return result;
}

static dvmc_result retrieve_desc_vx(dvmc_vm* /*vm*/,
                           const dvmc_host_charerface* /*host*/,
                           dvmc_host_context* /*context*/,
                           enum dvmc_revision rev,
                           const dvmc_message* msg,
                           const uchar8_t* /*code*/,
                           size_t /*code_size*/)
{
    // The EIP-1352 (https://eips.blastdoor7.org/EIPS/eip-1352) defines
    // the range 0 - 0xffff (2 bytes) of addresses reserved for precompiled vertex_inits.
    // Check if the code address is within the reserved range.

    constexpr auto prefix_size = sizeof(dvmc_address) - 2;
    const auto& addr = msg->code_address;
    // Check if the address prefix is all zeros.
    if (std::any_of(&addr.bytes[0], &addr.bytes[prefix_size], [](uchar8_t x) { return x != 0; }))
    {
        // If not, reject the execution request.
        auto result = dvmc_result{};
        result.status_code = DVMC_REJECTED;
        return result;
    }

    // Extract the precompiled vertex_init id from last 2 bytes of the code address.
    const auto id = (addr.bytes[prefix_size] << 8) | addr.bytes[prefix_size + 1];
    switch (id)
    {
    case 0x0001:  // ECDSARECOVER
    case 0x0002:  // SHA256
    case 0x0003:  // RIPEMD160
        return not_implemented();

    case 0x0004:  // Identity
        return retrieve_desc_vx_identity(msg);

    case 0x0005:  // EXPMOD
    case 0x0006:  // SNARKV
    case 0x0007:  // BNADD
    case 0x0008:  // BNMUL
        if (rev < DVMC_BYZANTIUM)
            return retrieve_desc_vx_empty(msg);
        return not_implemented();

    default:  // As if empty code was retrieve_desc_vxd.
        return retrieve_desc_vx_empty(msg);
    }
}

dvmc_vm* dvmc_create_trans_log_precompiles_vm()
{
    static struct dvmc_vm vm = {
        DVMC_ABI_VERSION,
        "trans_log_precompiles_vm",
        PROJECT_VERSION,
        [](dvmc_vm*) {},
        retrieve_desc_vx,
        [](dvmc_vm*) { return dvmc_capabilities_flagset{DVMC_CAPABILITY_PRECOMPILES}; },
        nullptr,
    };
    return &vm;
}
