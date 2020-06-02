#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>

#include "logger.hpp"

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8  = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using addr_t = std::uint64_t;
using offset_t = std::int32_t;
using inst_t = std::uint32_t;

constexpr size_t InstructionWidth = sizeof(inst_t);

constexpr u64 operator""_kiB(u64 value) {
    return value * 1024;
}

constexpr u64 operator""_MiB(u64 value) {
    return operator""_kiB(value) * 1024;
}

constexpr u64 operator""_GiB(u64 value) {
    return operator""_MiB(value) * 1024;
}

#define BIT(bit) bit, bit
#define BITS(range) std::max(true ? range, false ? range), std::min(true ? range, false ? range)

template<u8 from, u8 to>
[[nodiscard]] constexpr u64 extract(const u64 &&value) {
    u64 mask = (std::numeric_limits<u64>::max() >> (63 - (from - to))) << to;
    return (value & mask) >> to;
}

[[nodiscard]] constexpr u64 extendSign(u64 value, u8 currWidth, u8 targetWidth) {
    u64 mask = 1LLU << (currWidth - 1);
    return (((value ^ mask) - mask) << (64 - targetWidth)) >> (64 - targetWidth);
}