#pragma once

#include "arm.hpp"

#include <type_traits>

namespace arm::core {

    struct Register {
        virtual operator u64() = 0;
        virtual Register& operator=(u64 value) = 0;
    };

    struct RegisterDouble : public Register {
        RegisterDouble() { this->W = 0; }
        RegisterDouble(u64 value) { this->W = value; }

        union {
            u64 W;
            u32 X;
        };

        virtual operator u64() override { return this->W; }
        virtual Register& operator=(u64 value) override { this->W = value; return *this; }
    };

    struct RegisterSingle : public Register {
        RegisterSingle() { this->W = 0; }
        RegisterSingle(u64 value) { this->W = value; }

        u64 W;

        virtual operator u64() {
            return this->W;
        }

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

        operator T() { return 0; }

        static_assert(std::is_same_v<T, u32> || std::is_same_v<T, u64>, "Invalid zero register size.");
    };

    struct ZRegister : public Register {
        ZRegister(){}
        virtual operator u64() override { return 0; }
        virtual ZRegister& operator=(u64 value) override { return *this; }

        ZRegisterImpl<u64> W;
        ZRegisterImpl<u32> X;
    };

    class ELRegister;

    template<u8 el>
    struct EL : public Register {
    public:
        EL() : value(0) {}
        EL(u64 value) : value(value) { }

    private:
        friend ELRegister;

        virtual operator u64() override { return this->value; }
        virtual EL& operator=(u64 value) override { this->value = value; return *this; }


        u64 value;

        static_assert(el <= 3, "Exception Level out of range.");
    };

    class ELRegister {
    public:
        template<u8 el>
        ELRegister& operator=(EL<el> value) {
            reg[el] = value;

            return *this;
        }

    private:
        u64 reg[4];
    };

    using EL0 = EL<0>;
    using EL1 = EL<1>;
    using EL2 = EL<2>;
    using EL3 = EL<3>;

}

