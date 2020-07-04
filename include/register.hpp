#pragma once

#include "arm.hpp"

#include <type_traits>

namespace arm::core {

#define EL0(x) arm::core::EL(0, x)
#define EL1(x) arm::core::EL(1, x)
#define EL2(x) arm::core::EL(2, x)
#define EL3(x) arm::core::EL(3, x)

    struct Register {
        virtual operator u64() const = 0;
        virtual Register& operator=(u64 value) = 0;
    };

    struct RegisterDouble : public Register {
        RegisterDouble() { this->X = 0; }
        RegisterDouble(u64 value) { this->X = value; }

        union {
            u64 X;
            u32 W;
        };

        virtual operator u64() const override { return this->W; }
        virtual Register& operator=(u64 value) override { this->W = value; return *this; }
    };

    struct RegisterSingle : public Register {
        RegisterSingle() { this->X = 0; }
        RegisterSingle(u64 value) { this->X = value; }

        u64 X;

        virtual operator u64() const override { return this->X; }

        virtual Register& operator=(u64 value) override { this->X = value; return *this; }

        auto operator+(const u64 other) {
            RegisterSingle reg { this->X + other };

            return reg;
        }

        auto operator-(const u64 other) {
            RegisterSingle reg { this->X - other };

            return reg;
        }

        auto operator+=(const u64 other) {
            this->X += other;

            return *this;
        }

        auto operator-=(const u64 other) {
            this->X -= other;

            return *this;
        }
    };

    struct ZRegister;

    template<typename T>
    class ZRegisterImpl {
    public:
        ZRegister& operator=(RegisterDouble &other) { return *this; }
        ZRegister& operator=(RegisterSingle &other) { return *this; }
        ZRegister& operator=(T &other) { return *this; }

        operator T() const { return 0; }

        static_assert(std::is_same_v<T, u32> || std::is_same_v<T, u64>, "Invalid zero register size.");
    };

    struct ZRegister : public RegisterDouble {
        ZRegister(){}
        virtual operator u64() const override { return 0; }
        virtual ZRegister& operator=(u64 value) override { return *this; }

        ZRegisterImpl<u32> W;
        ZRegisterImpl<u64> X;
    };

    class ELRegister {
    public:
        ELRegister() {
            for (u8 i = 0; i < 4; i++)
                this->m_reg[i] = 0;
        }
        constexpr core::RegisterDouble& operator[](u8 el) {
            return this->m_reg[el];
        }
    private:
        core::RegisterDouble m_reg[4];
    };


    class GPRegister {
    public:
        constexpr core::RegisterDouble& operator[](u8 R) {
            if (R < 31)
                return GPR[R];
            else if (R >= 32 && R <= 35)
                return SP[R - 32];
            else
                return ZR;
        }
    private:
        core::RegisterDouble GPR[31];
        core::ZRegister ZR;
        core::ELRegister SP;
    };

}

