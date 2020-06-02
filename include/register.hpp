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
        RegisterDouble() { this->W = 0; }
        RegisterDouble(u64 value) { this->W = value; }

        union {
            u64 W;
            u32 X;
        };

        virtual operator u64() const override { return this->W; }
        virtual Register& operator=(u64 value) override { this->W = value; return *this; }
    };

    struct RegisterSingle : public Register {
        RegisterSingle() { this->W = 0; }
        RegisterSingle(u64 value) { this->W = value; }

        u64 W;

        virtual operator u64() const override { return this->W; }

        virtual Register& operator=(u64 value) override { this->W = value; return *this; }

        auto operator+(const u64 other) {
            RegisterSingle reg { this->W + other };

            return reg;
        }

        auto operator-(const u64 other) {
            RegisterSingle reg { this->W - other };

            return reg;
        }

        auto operator+=(const u64 other) {
            this->W += other;

            return *this;
        }

        auto operator-=(const u64 other) {
            this->W -= other;

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

        ZRegisterImpl<u64> W;
        ZRegisterImpl<u32> X;
    };

    class ELRegister;

    struct EL : public Register {
    public:
        EL() : m_el(0), m_value(0) {}
        EL(u8 el, u64 value) : m_el(el), m_value(value) { }

        const u8 getEL() const { return this->m_el; }

    private:
        friend ELRegister;

        virtual operator u64() const override { return this->m_value; }
        virtual EL& operator=(u64 value) override { this->m_value = value; return *this; }

        u8 m_el;
        u64 m_value;
    };

    class ELRegister {
    public:
        u64& operator[](u8 el) {
            return this->m_reg[el];
        }
    private:
        u64 m_reg[4];
    };


    class GPRegister {
    public:
        core::RegisterDouble& operator[](u8 R) {
            if (R < 31)
                return GPR[R];
            else
                return ZR;
        }
    private:
        core::RegisterDouble GPR[31];
        core::ZRegister ZR;
    };

}

