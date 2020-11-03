#pragma once

#include <armv8.hpp>

#include <cassert>
#include <numeric>
#include <type_traits>
#include <concepts>

#include "registers.hpp"

namespace arm {

    struct NZCV {
        u8 N : 1;
        u8 Z : 1;
        u8 C : 1;
        u8 V : 1;
    };

    template<typename T>
    concept register_type = std::same_as<T, u32> || std::same_as<T, u64>;

    using field_t = u32;
    #define field_bits(range) (false ? range) - (true ? range) + 1

    constexpr u64 operator""_kiB(unsigned long long value) {
        return value * 1024;
    }

    constexpr u64 operator""_MiB(unsigned long long value) {
        return operator""_kiB(value) * 1024;
    }

    constexpr u64 operator""_GiB(unsigned long long value) {
        return operator""_MiB(value) * 1024;
    }

    #define BIT(bit) bit, bit
    #define BITS(range) std::max(true ? range, false ? range), std::min(true ? range, false ? range)

    template<typename T>
    consteval u8 bitsof() {
        return sizeof(T) * 8;
    }

    template<u8 fromHigh, u8 toHigh, u8 fromLow, u8 toLow>
    [[nodiscard]] constexpr inline u64 combine(u64 valueHigh, u64 valueLow) {
        constexpr u8 sizeHigh = fromHigh - toHigh;
        constexpr u8 sizeLow  = fromLow - toLow;

        static_assert((sizeHigh + sizeLow) <= sizeof(u64) * 8, "Combined size greater than 64 bit");

        constexpr u64 mask = 0xFFFF'FFFF'FFFF'FFFF;
        return (valueLow & ~(mask << sizeLow)) | (valueHigh << sizeHigh);
    }

    template<u8 from, u8 to>
    [[nodiscard]] constexpr inline u64 extract(const u64 &value) {
        u64 mask = (std::numeric_limits<u64>::max() >> (63 - (from - to))) << to;
        return (value & mask) >> to;
    }

    [[nodiscard]] constexpr inline u64 signExtend(u64 value, u8 currWidth, u8 targetWidth) {
        u64 mask = 1LLU << (currWidth - 1);
        return (((value ^ mask) - mask) << (64 - targetWidth)) >> (64 - targetWidth);
    }

    [[nodiscard]] constexpr inline uint32_t makeBitmask(uint8_t from, uint8_t to) {
        uint32_t mask = 0xFFFF'FFFF;

        return (((mask >> from) << from) << (32 - to)) >> (32 - to);
    }

    [[nodiscard]] constexpr inline bool conditionHolds(u8 cond, ProcState &PSTATE) {
        bool result = false;
        switch (cond >> 1) {
            case 0b000: result = (PSTATE.Z == 1); break;                            // EQ or NE
            case 0b001: result = (PSTATE.C == 1); break;                            // CS or CC
            case 0b010: result = (PSTATE.N == 1); break;                            // MI or PL
            case 0b011: result = (PSTATE.V == 1); break;                            // VS or VC
            case 0b100: result = (PSTATE.C == 1 && PSTATE.Z == 0); break;           // HI or LS
            case 0b101: result = (PSTATE.N == PSTATE.V); break;                     // GE or LT
            case 0b110: result = (PSTATE.N == PSTATE.V && PSTATE.Z == 0); break;    // GT or LE
            case 0b111: result = true; break;                                       // AL
        }

        if (extract<BIT(0)>(cond) == 1 && cond != 0b1111)
            result = !result;

        return result;
    }

    template<register_type T>
    [[nodiscard]] constexpr inline T shiftReg(T value, u8 shiftType, u8 shiftAmount) {
        switch (shiftType) {
            case 0b00: // LSL
                value = value << shiftAmount;
                break;
            case 0b01: // LSR
                value = value >> shiftAmount;
                break;
            case 0b10: // ASR
                value = (typename std::make_signed<T>::type)(value) >> shiftAmount;
                break;
            case 0b11: // ROR
                value = std::rotr(value, shiftAmount);
        }

        return value;
    }

    template<register_type T>
    constexpr inline bool isNegative(T x) {
        return extract<BIT(sizeof(T) - 1)>(x) != 0;
    }

    template<register_type T>
    constexpr inline bool isZero(T x) {
        return x == 0;
    }

    template<register_type T>
    [[nodiscard]] constexpr inline std::pair<T, NZCV> addWithCarry(T x, T y, u8 carryBit) {
        using ST = typename std::make_signed<T>::type;
        u64 unsignedSum = T(x) + T(y) + T(carryBit);
        s64 signedSum = ST(x) + ST(y) + T(carryBit);

        T result = extract<BITS(0:(bitsof<T>() - 1))>(unsignedSum);
        NZCV flags;

        flags.N = isNegative(unsignedSum);
        flags.Z = isZero(result);
        flags.C = T(result)  != unsignedSum;
        flags.V = ST(result) != signedSum;

        return { result, flags };
    }

    constexpr inline void setNZCV(ProcState &PSTATE, const NZCV &nzcv) {
        PSTATE.N = nzcv.N;
        PSTATE.Z = nzcv.Z;
        PSTATE.C = nzcv.C;
        PSTATE.V = nzcv.V;
    }

    constexpr inline u64 replicate(u64 mask, size_t size) {
        while (size < 64) {
            mask |= (mask << size);
            size <<= 1;
        }

        return mask;
    }

    constexpr inline u64 decodeBitMasks(u8 N, u8 imms, u8 immr) {
        assert(N <= 1 && imms < 64 && immr < 64);

        s32 length = 31 - __builtin_clz((N << 6) | ((~imms) & 0x3F));
        assert(length >= 1);

        u32 size = 1 << length;
        u32 levels = size - 1;
        u32 s = imms & levels;
        u32 r = immr & levels;

        assert(s != levels);

        u64 mask = makeBitmask(0, s + 1);
        if (r != 0) {
            mask = (mask >> r) | (mask << (size - r));
            mask &= makeBitmask(0, size);
        }
        mask = replicate(mask, size);

        return mask;
    }

}