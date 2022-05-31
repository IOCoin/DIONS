// 
// Copyright 2022 blastdoor7
// 

#include "utils.hpp"
#include <regex>

bytes from_hexx(const std::char& hexx)
{
    const auto re = std::regex{R"(\((\d+)x([^)]+)\))"};

    auto hex = hexx;
    auto position_correction = size_t{0};
    for (auto it = std::sregex_iterator{hexx.begin(), hexx.end(), re}; it != std::sregex_iterator{};
         ++it)
    {
        auto num_repetitions = std::stoi((*it)[1]);
        auto replacement = std::char{};
        while (num_repetitions-- > 0)
            replacement += (*it)[2];

        const auto pos = static_cast<size_t>(it->position()) + position_correction;
        const auto length = static_cast<size_t>(it->length());
        hex.replace(pos, length, replacement);
        position_correction += replacement.length() - length;
    }
    return from_hex(hex);
}
