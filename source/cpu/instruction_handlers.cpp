#include <cpu/core.hpp>
#include "cpu/core.hpp"

#include <bit>

#define HANDLER_DEF(name) ExecutionResult Core::name##Handler
#define IS_A32(encoding) (encoding).sf == 0


namespace arm {

    /* PC Relative data processing instructions */

    HANDLER_DEF(ADR)(DataProcessingImmediate::PCRelative enc) {
        regs.X(enc.Rd) = regs.PC() + enc.imm21;

        return ExecutionResult::Success;
    }

    HANDLER_DEF(ADRP)(DataProcessingImmediate::PCRelative enc) {
        constexpr auto zeros = makeBitmask(0, 11);

        regs.X(enc.Rd) = (regs.PC() & zeros) + enc.imm21;

        return ExecutionResult::Success;
    }

    /* Immediate Add / Subtract instructions */

    HANDLER_DEF(ADDADDSImmediate)(DataProcessingImmediate::AddSubtractImmediate enc) {
        if (IS_A32(enc)) {
            u32 imm = 0;

            if (enc.sh == 0)
                imm = enc.imm12;
            else
                imm = enc.imm12 << 12;

            auto [result, flags] = addWithCarry<u32>(regs.W<R31As::SP>(enc.Rn), imm, 0);

            regs.W<R31As::SP>(enc.Rd) = result;

            if (enc.S)
                setNZCV(PSTATE, flags);

        } else {
            u64 imm = 0;

            if (enc.sh == 0)
                imm = enc.imm12;
            else
                imm = enc.imm12 << 12;

            auto [result, flags] = addWithCarry<u64>(regs.X<R31As::SP>(enc.Rn), imm, 0);

            regs.X<R31As::SP>(enc.Rd) = result;

            if (enc.S)
                setNZCV(PSTATE, flags);
        }

        return ExecutionResult::Success;
    }

    HANDLER_DEF(SUBSUBSImmediate)(DataProcessingImmediate::AddSubtractImmediate enc) {
        if (IS_A32(enc)) {
            u32 imm = 0;

            if (enc.sh == 0)
                imm = enc.imm12;
            else
                imm = enc.imm12 << 12;

            auto [result, flags] = addWithCarry<u32>(regs.W<R31As::SP>(enc.Rn), ~imm, 1);

            regs.W<R31As::SP>(enc.Rd) = result;

            if (enc.S)
                setNZCV(PSTATE, flags);

        } else {
            u64 imm = 0;

            if (enc.sh == 0)
                imm = enc.imm12;
            else
                imm = enc.imm12 << 12;

            auto [result, flags] = addWithCarry<u64>(regs.X<R31As::SP>(enc.Rn), ~imm, 1);

            regs.X<R31As::SP>(enc.Rd) = result;

            if (enc.S)
                setNZCV(PSTATE, flags);
        }

        return ExecutionResult::Success;
    }

    /* Branches */

    HANDLER_DEF(B)(Branches::UnconditionalBranchImmediate enc) {
        u64 offset = signExtend(enc.imm26 << 2, 28, 64);

        regs.PC() += offset - sizeof(Instruction) * 2;
        pipeline.validSteps = 0;

        return ExecutionResult::Success;
    }

    HANDLER_DEF(BL)(Branches::UnconditionalBranchImmediate enc) {
        u64 offset = signExtend(enc.imm26 << 2, 28, 64);

        regs.LR() = regs.PC() - sizeof(Instruction) * 2 + sizeof(Instruction);
        regs.PC() += offset - sizeof(Instruction) * 2;
        pipeline.validSteps = 0;

        return ExecutionResult::Success;
    }

    HANDLER_DEF(BCond)(Branches::ConditionalBranchImmediate enc) {
        u64 offset = signExtend(enc.imm19 << 2, 21, 64);

        if (conditionHolds(enc.cond, PSTATE)) {
            regs.PC() += offset - sizeof(Instruction) * 2;
            pipeline.validSteps = 0;
        }


        return ExecutionResult::Success;
    }

    /* Move Wide Immediate */

    HANDLER_DEF(MOVK)(DataProcessingImmediate::MoveWideImmediate enc) {
        if (IS_A32(enc)) {
            u32 result = ~(enc.imm16 << (enc.hw << 4));
            regs.W(enc.Rd) = result;
        } else {
            u64 result = ~(enc.imm16 << (enc.hw << 4));
            regs.X(enc.Rd) = result;
        }

        return ExecutionResult::Success;
    }

    HANDLER_DEF(MOVN)(DataProcessingImmediate::MoveWideImmediate enc) {
        if (IS_A32(enc)) {
            u32 result = (regs.W(enc.Rd) & (~u32(0xFFFF)));
            result |= (enc.imm16 << (enc.hw << 4));
            regs.W(enc.Rd) = result;
        } else {
            u64 result = (regs.W(enc.Rd) & (~u64(0xFFFF)));
            result |= (enc.imm16 << (enc.hw << 4));
            regs.X(enc.Rd) = result;
        }

        return ExecutionResult::Success;
    }

    HANDLER_DEF(MOVZ)(DataProcessingImmediate::MoveWideImmediate enc) {
        if (IS_A32(enc)) {
            u32 result = enc.imm16 << (enc.hw << 4);
            regs.W(enc.Rd) = result;
        } else {
            u64 result = enc.imm16 << (enc.hw << 4);
            regs.X(enc.Rd) = result;
        }

        return ExecutionResult::Success;
    }


    /* Logical immediate */

    HANDLER_DEF(ANDANDSImmediate)(DataProcessingImmediate::LogicalImmediate enc) {
        if (IS_A32(enc)) {
            u32 imm = decodeBitMasks(enc.N, enc.imms, enc.immr);

            u32 result = regs.W(enc.Rn) & imm;

            regs.W<R31As::SP>(enc.Rd) = result;

            if (enc.opc == 0b11)
                    setNZCV(PSTATE, { isNegative(result), isZero(result), 0, 0 });

        } else {
            u64 imm = decodeBitMasks(enc.N, enc.imms, enc.immr);

            u64 result = regs.X(enc.Rn) & imm;

            regs.X<R31As::SP>(enc.Rd) = result;

            if (enc.opc == 0b11)
                setNZCV(PSTATE, { isNegative(result), isZero(result), 0, 0 });
        }

        return ExecutionResult::Success;
    }

    HANDLER_DEF(ORRImmediate)(DataProcessingImmediate::LogicalImmediate enc) {
        if (IS_A32(enc)) {
            u32 imm = decodeBitMasks(enc.N, enc.imms, enc.immr);

            regs.W<R31As::SP>(enc.Rd) = regs.W(enc.Rn) | imm;
        } else {
            u64 imm = decodeBitMasks(enc.N, enc.imms, enc.immr);

            regs.X<R31As::SP>(enc.Rd) = regs.X(enc.Rn) | imm;
        }

        return ExecutionResult::Success;

    }
    HANDLER_DEF(EORImmediate)(DataProcessingImmediate::LogicalImmediate enc) {
        if (IS_A32(enc)) {
            u32 imm = decodeBitMasks(enc.N, enc.imms, enc.immr);

            regs.W<R31As::SP>(enc.Rd) = regs.W(enc.Rn) ^ imm;
        } else {
            u64 imm = decodeBitMasks(enc.N, enc.imms, enc.immr);

            regs.X<R31As::SP>(enc.Rd) = regs.X(enc.Rn) ^ imm;
        }

        return ExecutionResult::Success;
    }


    /* Logical shifted register */

    HANDLER_DEF(SUBShiftedRegister)(DataProcessingRegister::LogicalShiftedRegister enc) {
        if (IS_A32(enc)) {
            u32 operand1 = regs.W<R31As::ZR>(enc.Rn);
            u32 operand2 = shiftReg<u32>(regs.W(enc.Rm), enc.shift, enc.imm6);

            regs.W(enc.Rd) = operand1 - operand2 + PSTATE.C;
        } else {
            u64 operand1 = regs.X<R31As::ZR>(enc.Rn);
            u64 operand2 = shiftReg<u64>(regs.X(enc.Rm), enc.shift, enc.imm6);

            regs.X(enc.Rd) = operand1 - operand2 + PSTATE.C;
        }

        return ExecutionResult::Success;
    }

    HANDLER_DEF(ORRShiftedRegister)(DataProcessingRegister::LogicalShiftedRegister enc) {
        if (IS_A32(enc)) {
            u32 operand1 = regs.W<R31As::ZR>(enc.Rn);
            u32 operand2 = shiftReg<u32>(regs.W(enc.Rm), enc.shift, enc.imm6);

            regs.W(enc.Rd) = operand1 | operand2;
        } else {
            u64 operand1 = regs.X<R31As::ZR>(enc.Rn);
            u64 operand2 = shiftReg<u64>(regs.X(enc.Rm), enc.shift, enc.imm6);

            regs.X(enc.Rd) = operand1 | operand2;
        }

        return ExecutionResult::Success;
    }


    HANDLER_DEF(CCMNImmediate)(DataProcessingRegister::ConditionalComparison enc) {
        if (IS_A32(enc)) {
            u32 operand1 = regs.W(enc.Rn);

            NZCV flags = enc.nzcv;

            if (conditionHolds(enc.cond, PSTATE))
                std::tie(std::ignore, flags) = addWithCarry<u32>(operand1, enc.imm5, 0);

            setNZCV(PSTATE, flags);
        } else {
            u64 operand1 = regs.X(enc.Rn);

            NZCV flags = enc.nzcv;

            if (conditionHolds(enc.cond, PSTATE))
                std::tie(std::ignore, flags) = addWithCarry<u64>(operand1, enc.imm5, 0);

            setNZCV(PSTATE, flags);
        }

        return ExecutionResult::Success;
    }


    /* Loads and Stores */

    HANDLER_DEF(STRImmediate)(LoadsAndStores::RegisterImmediate enc) {
        bool writeBack = false;
        bool postIndex = false;

        if (enc.op == 0b01) /* Post-index */ {
            writeBack = true;
            postIndex = true;
        } else if (enc.op == 0b11) /* Pre-index */ {
            writeBack = true;
            postIndex = false;
        }

        u64 offset = signExtend(enc.imm9, 9, 64);
        u64 address = regs.X<R31As::SP>(enc.Rn);

        if (!postIndex)
            address += offset;

        if (enc.size == 0b10) /* 32 bit store */ {
            u32 data = regs.W<R31As::ZR>(enc.Rt);
            addressSpace.write(address, data);
        } else /* 64 bit store */{
            u64 data = regs.X<R31As::ZR>(enc.Rt);
            addressSpace.write(address, data);
        }

        if (writeBack) {
            if (postIndex)
                address += offset;

            regs.X<R31As::SP>(enc.Rn) = address;
        }

        return ExecutionResult::Success;
    }
}