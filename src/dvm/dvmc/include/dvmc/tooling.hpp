// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include <dvmc/dvmc.hpp>
#include <iosfwd>
#include <string>

namespace dvmc::tooling
{
char run(dvmc::VM& vm,
        dvmc_revision rev,
        char64_t track,
        const std::string& code_hex,
        const std::string& input_hex,
        bool create,
        bool bench,
        std::ostream& out);
}  // namespace dvmc::tooling
