#pragma once

#include <armv8.hpp>

#include <array>
#include <concepts>
#include <functional>
#include <numeric>

#include "cpu/address_space.hpp"
#include "cpu/parsing/encodings.hpp"
#include "registers.hpp"
#include "cpu/parsing/opcode.hpp"
#include "arm_functions.hpp"

namespace arm {

    class Core;
    using OpcodeHandler = ExecutionResult(Core::*)(Instruction);

    struct Opcode {
        consteval Opcode(const char *name, OpcodeEncoding encoding, OpcodeHandler callback) : name(name), encoding(encoding), callback(callback) {}
        const char *name;
        OpcodeEncoding encoding;
        OpcodeHandler callback;
    };


    class Core {
    public:
        Core();

        void tick();
        Instruction prefetch();
        auto decode(Instruction instruction);
        ExecutionResult execute(std::function<ExecutionResult()> &handler);

    private:
        struct {
            Instruction prefetchStep;
            std::function<ExecutionResult()> decodeStep;
            ExecutionResult executeStep;

            u8 validSteps = 0;
        } pipeline;

        Registers regs;
        AddressSpace addressSpace;
        ProcState PSTATE;

        template<auto Callback, typename Encoding>
        ExecutionResult Handler(Instruction instruction) {
            return (this->*Callback)(Encoding(instruction));
        }

        /* Define all instruction handlers */
        #define HANDLER_DEF(name) ExecutionResult name##Handler
        #define OPCODE(pattern, name, ...) Opcode { #name, OpcodeEncoding(pattern), &Core::Handler<&Core::name##Handler, decltype(FirstArgument(&Core::name##Handler))::Type> }

        constexpr auto getOpcodes() const {
            return std::array
            {
                OPCODE("0xx1'0000'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx", ADR),
                OPCODE("1xx1'0000'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx", ADRP),
                OPCODE("x0x1'0001'0xxx'xxxx'xxxx'xxxx'xxxx'xxxx", ADDADDSImmediate),
                OPCODE("x1x1'0001'0xxx'xxxx'xxxx'xxxx'xxxx'xxxx", SUBSUBSImmediate),

                OPCODE("xxx1'0010'0xxx'xxxx'xxxx'xxxx'xxxx'xxxx", ANDANDSImmediate),
                OPCODE("x011'0010'0xxx'xxxx'xxxx'xxxx'xxxx'xxxx", ORRImmediate),
                OPCODE("x101'0010'0xxx'xxxx'xxxx'xxxx'xxxx'xxxx", EORImmediate),

                OPCODE("0001'01xx'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx", B),
                OPCODE("1001'01xx'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx", BL),
                OPCODE("0101'0100'xxxx'xxxx'xxxx'xxxx'xxx0'xxxx", BCond),

                OPCODE("x111'0010'1xxx'xxxx'xxxx'xxxx'xxxx'xxxx", MOVK),
                OPCODE("x001'0010'1xxx'xxxx'xxxx'xxxx'xxxx'xxxx", MOVN),
                OPCODE("x101'0010'1xxx'xxxx'xxxx'xxxx'xxxx'xxxx", MOVZ),

                OPCODE("x100'1011'xx0x'xxxx'xxxx'xxxx'xxxx'xxxx", SUBShiftedRegister),
                OPCODE("x010'1010'xx0x'xxxx'xxxx'xxxx'xxxx'xxxx", ORRShiftedRegister),

                OPCODE("xx11'1010'010x'xxxx'xxxx'10xx'xxx0'xxxx", CCMNImmediate),

                OPCODE("1x11'1000'000x'xxxx'xxxx'xxxx'xxxx'xxxx", STRImmediate)
            };
        }

        HANDLER_DEF(ADR)(DataProcessingImmediate::PCRelative);
        HANDLER_DEF(ADRP)(DataProcessingImmediate::PCRelative);
        HANDLER_DEF(ADDADDSImmediate)(DataProcessingImmediate::AddSubtractImmediate);
        HANDLER_DEF(SUBSUBSImmediate)(DataProcessingImmediate::AddSubtractImmediate);

        HANDLER_DEF(ANDANDSImmediate)(DataProcessingImmediate::LogicalImmediate);
        HANDLER_DEF(ORRImmediate)(DataProcessingImmediate::LogicalImmediate);
        HANDLER_DEF(EORImmediate)(DataProcessingImmediate::LogicalImmediate);

        HANDLER_DEF(B)(Branches::UnconditionalBranchImmediate);
        HANDLER_DEF(BL)(Branches::UnconditionalBranchImmediate);
        HANDLER_DEF(BCond)(Branches::ConditionalBranchImmediate);

        HANDLER_DEF(MOVK)(DataProcessingImmediate::MoveWideImmediate);
        HANDLER_DEF(MOVN)(DataProcessingImmediate::MoveWideImmediate);
        HANDLER_DEF(MOVZ)(DataProcessingImmediate::MoveWideImmediate);

        HANDLER_DEF(SUBShiftedRegister)(DataProcessingRegister::LogicalShiftedRegister);
        HANDLER_DEF(ORRShiftedRegister)(DataProcessingRegister::LogicalShiftedRegister);

        HANDLER_DEF(CCMNImmediate)(DataProcessingRegister::ConditionalComparison);

        HANDLER_DEF(STRImmediate)(LoadsAndStores::RegisterImmediate);

#undef HANDLER_DEF

    };

}