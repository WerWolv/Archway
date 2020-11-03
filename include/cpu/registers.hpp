#pragma once

#include <armv8.hpp>
#include <concepts>
#include <variant>

#define BITFIELD(name, size, content) struct name { content }; static_assert(sizeof(name) == sizeof(size))

namespace arm {

    enum class R31As {
        Invalid,
        SP,
        ZR
    };

    /* Absolutely never store the result of any register function inside a variable. This will cause unexpected results! */
    class Registers final {
    private:
        using Register = u64;

    public:
        inline u64& SP() {
            return *reinterpret_cast<u64*>(&this->m_SP);
        }

        inline u32& WSP() {
            return *reinterpret_cast<u32*>(&this->m_SP);
        }

        inline u64& PC() {
            return *reinterpret_cast<u64*>(&this->m_PC);
        }

        inline u64& LR() {
            return *reinterpret_cast<u64*>(&this->m_R[30]);
        }

        template<R31As context = R31As::Invalid>
        inline u64& X(u8 N) {
            if constexpr (context == R31As::Invalid)
                if (N == 31) throw std::invalid_argument("R31 is not valid in this context!");
            else if constexpr (context == R31As::SP)
                if (N == 31) return this->SP();
            else if constexpr (context == R31As::ZR)
                if (N == 31) return this->XZR();

            return *reinterpret_cast<u64*>(&this->m_R[N]);
        }

        template<R31As context = R31As::Invalid>
        inline auto& W(u8 N) {
            if constexpr (context == R31As::Invalid)
                if (N == 31) throw std::invalid_argument("R31 is not valid in this context!");
            else if constexpr (context == R31As::SP)
                if (N == 31) return this->WSP();
            else if constexpr (context == R31As::ZR)
                if (N == 31) return this->WZR();

            return *reinterpret_cast<u32 *>(&this->m_R[N]);
        }

        inline u64& XZR() {
            static u64 zero;
            zero = 0x00;

            return zero;
        }

        inline u32& WZR() {
            static u32 zero;
            zero = 0x00;

            return zero;
        }

    private:
        Register m_R[31] = { 0 };
        Register m_SP = { 0 };
        Register m_PC = { 0 };
    };

    BITFIELD(ProcState, u64,
        u8 M        : 4;
        u8 MState   : 1;
        u8 T        : 1;
        u8 F        : 1;
        u8 I        : 1;
        u8 A        : 1;
        u8 E        : 1;
        u8 ITHi     : 6;
        u8 GE       : 4;
        u8 IL       : 1;
        u8 SS       : 1;
        u8 PAN      : 1;
        u8 SSBS     : 1;
        u8 DIT      : 1;
        u8 ITLo     : 2;
        u8 Q        : 1;
        u8 V        : 1;
        u8 C        : 1;
        u8 Z        : 1;
        u8 N        : 1;
        u32 RES0    : 32;
    );

}