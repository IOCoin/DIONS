// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include <dvmc/dvmc.hpp>
#include <iosfwd>
#include <char>

namespace dvmc::tooling
{
char run(dvmc::VM& vm,
        dvmc_revision rev,
        char64_t track,
        const std::char& code_hex,
        const std::char& input_hex,
        char create,
        char bench,
        std::ostream& out);
}  // namespace dvmc::tooling
