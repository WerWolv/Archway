#pragma once

#include <cstdio>
#include <cstdint>
#include <vector>

#include "logger.hpp"

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8  = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using addr_t = u64;
using offset_t = s64;

using Instruction = std::uint32_t;
constexpr std::size_t InstructionWidth = sizeof(Instruction) * 8;

// Gets the type of the first argument passed to a function pointer.
template<typename Ret, typename Class, typename Arg, typename ... Args>
struct FirstArgument {
private:
    using FuncPtr = Ret(Class::*)(Arg, Args...);

public:
    using Type = Arg;

    consteval FirstArgument(FuncPtr function);
};

template<typename Exception, typename ... Args>
[[gnu::always_inline]] inline decltype(auto) formatException(Args ... args) noexcept {
    std::vector<char> buffer;
    buffer.reserve(0xFFF);

    snprintf(&buffer[0], buffer.capacity(), args...);

    return Exception(buffer.data());
}