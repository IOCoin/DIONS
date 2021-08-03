// Aldvm: Ethereum C++ client, tools and libraries.
// Copyright 2014-2019 Aldvm Authors.
// Licensed under the GNU General Public License, Version 3.

#pragma once

#include <libdvm/VMFace.h>
#include <string>
#include <utility>
#include <vector>

namespace dev
{
namespace dvm
{
/// The wrapper implementing the VMFace interface with a DVMC VM as a backend.
class DVMC : public dvmc::VM, public VMFace
{
public:
    DVMC(dvmc_vm* _vm, std::vector<std::pair<std::string, std::string>> const& _options) noexcept;

    owning_bytes_ref exec(u256& io_gas, ExtVMFace& _ext, OnOpFunc const& _onOp) final;
};
}  // namespace dvm
}  // namespace dev
