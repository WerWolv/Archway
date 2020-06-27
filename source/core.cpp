#include "core.hpp"
#include <bit>

namespace arm {

    Core::Core(AddressSpace *addressSpace) : m_addressSpace(addressSpace) {
        PC = 0x0000;
    }

    constexpr auto Core::getInstructionPatternLUT() const {
        std::array lut {
            INSTRUCTION(0b1111'1111'1111'1111'1111'1111'1111'1111, 0b1101'0101'0000'0011'0010'0000'0001'1111, NOP),
            INSTRUCTION(0b0111'1111'1000'0000'0000'0000'0000'0000, 0b0011'0010'0000'0000'0000'0000'0000'0000, ORR_IMMEDIATE), // MOV Alias
            INSTRUCTION(0b0111'1111'0010'0000'0000'0000'0000'0000, 0b0010'1010'0000'0000'0000'0000'0000'0000, ORR_SHIFTED_REGISTER), // MOV Alias
            INSTRUCTION(0b0111'1111'1000'0000'0000'0000'0000'0000, 0b0101'0010'1000'0000'0000'0000'0000'0000, MOVZ),
            INSTRUCTION(0b1111'1100'0000'0000'0000'0000'0000'0000, 0b0001'0100'0000'0000'0000'0000'0000'0000, B),
            INSTRUCTION(0b1111'1111'0000'0000'0000'0000'0001'0000, 0b0101'0100'0000'0000'0000'0000'0000'0000, B_COND),
            INSTRUCTION(0b1111'1100'0000'0000'0000'0000'0000'0000, 0b1001'0100'0000'0000'0000'0000'0000'0000, BL),
            INSTRUCTION(0b0111'1111'0000'0000'0000'0000'0000'0000, 0b0001'0001'0000'0000'0000'0000'0000'0000, ADD_IMMEDIATE),
            INSTRUCTION(0b0111'1111'0010'0000'0000'0000'0000'0000, 0b0000'1011'0000'0000'0000'0000'0000'0000, ADD_SHIFTED_REGISTER),
            INSTRUCTION(0b0111'1111'0000'0000'0000'0000'0000'0000, 0b0011'0001'0000'0000'0000'0000'0000'0000, ADDS_IMMEDIATE),
            INSTRUCTION(0b0111'1111'0000'0000'0000'0000'0000'0000, 0b0101'0001'0000'0000'0000'0000'0000'0000, SUB_IMMEDIATE),
            INSTRUCTION(0b0111'1111'0010'0000'0000'0000'0000'0000, 0b0100'1011'0000'0000'0000'0000'0000'0000, SUB_SHIFTED_REGISTER),
            INSTRUCTION(0b0111'1111'0000'0000'0000'0000'0000'0000, 0b0111'0001'0000'0000'0000'0000'0000'0000, SUBS_IMMEDIATE),
            INSTRUCTION(0b0111'1111'0010'0000'0000'0000'0000'0000, 0b0110'1011'0000'0000'0000'0000'0000'0000, SUBS_SHIFTED_REGISTER),
            INSTRUCTION(0b0111'1111'1110'0000'0000'1100'0001'0000, 0b0011'1010'0100'0000'0000'1000'0000'0000, CCMN_IMMEDIATE),
            INSTRUCTION(0b0111'1111'1110'0000'0000'1100'0001'0000, 0b0011'1010'0100'0000'0000'0000'0000'0000, CCMN_REGISTER),
            INSTRUCTION(0b1111'1111'1110'0000'0000'0000'0001'1111, 0b1101'0100'0000'0000'0000'0000'0000'0001, SVC),
            INSTRUCTION(0b1001'1111'0000'0000'0000'0000'0000'0000, 0b1001'0000'0000'0000'0000'0000'0000'0000, ADRP),
            INSTRUCTION(0b0111'1111'1000'0000'0000'0000'0000'0000, 0b0001'0010'0000'0000'0000'0000'0000'0000, AND_IMMEDIATE),
            INSTRUCTION(0b0111'1111'0010'0000'0000'0000'0000'0000, 0b0000'1010'0000'0000'0000'0000'0000'0000, AND_SHIFTED_REGISTER),
            INSTRUCTION(0b0111'1111'1000'0000'0000'0000'0000'0000, 0b0111'0010'0000'0000'0000'0000'0000'0000, ANDS_IMMEDIATE),
            INSTRUCTION(0b0111'1111'0010'0000'0000'0000'0000'0000, 0b0110'1010'0000'0000'0000'0000'0000'0000, ANDS_SHIFTED_REGISTER),
            INSTRUCTION(0b1011'1100'1110'0000'0000'0000'0000'0000, 0b1011'1000'0000'0000'0000'0000'0000'0000, STR_IMMEDIATE),
            INSTRUCTION(0b1011'1111'1110'0000'0000'1100'0000'0000, 0b1011'1000'0010'0000'0000'1000'0000'0000, STR_REGISTER),
            INSTRUCTION(0b1011'1111'1110'0000'0000'1100'0000'0000, 0b1011'1000'0100'0000'0000'0010'0000'0000, LDR_IMMEDIATE),
            INSTRUCTION(0b1011'1111'1110'0000'0000'1100'0000'0000, 0b1011'1000'0110'0000'0000'1000'0000'0000, LDR_REGISTER)
            //INSTRUCTION(0b0111'1111'1110'0000'0000'1100'0001'0000, 0b0111'1010'0100'0000'0000'1000'0000'0000, CCMP_IMMEDIATE),
        };

        return lut;
    }

    inst_t Core::prefetch(const addr_t &pc) const {
        return inst_t(this->m_addressSpace->read(pc, InstructionWidth));
    }

    InstructionHandler Core::decode(const inst_t &instruction) {
        for (const auto& entry : getInstructionPatternLUT()) {
            if ((instruction & entry.mask) == entry.pattern) {
                Logger::debug("[%08llx] %s", PC.W, entry.name);
                return entry.type;
            }
        }

        dumpRegisters();
        Logger::fatal("Invalid instruction 0x%08llx!", instruction);

        return nullptr;
    }

    void Core::execute(const InstructionHandler& handler, const inst_t &instruction) {
        const u8 Rd = extract<BITS(0:4)>(instruction);
        const u8 Rn = extract<BITS(5:9)>(instruction);
        const u8 Rm = extract<BITS(16:20)>(instruction);
        const bool sf = extract<BITS(31:31)>(instruction);
        const u8 imm3 = extract<BITS(10:12)>(instruction);
        const u8 imm6 = extract<BITS(10:15)>(instruction);
        const u16 imm12 = extract<BITS(10:21)>(instruction);
        const u8 shift = extract<BITS(22:23)>(instruction);
        const u8 size = shift;

        //Logger::debug("Rd %u, Rn %u, Rm %u, sf %u, imm3 %u, imm6 %u, imm12 %u, shift %u", Rd, Rn, Rm, sf, imm3, imm6, imm12, shift);

        (this->*handler)(instruction, Rd, Rn, Rm, sf, imm3, imm6, imm12, shift, size);
    }

    void Core::reset() {
        this->m_halted = false;
    }

    void Core::halt() {
        this->m_halted = true;
    }

    void Core::tick() {
        if (this->m_halted || (this->m_broken && !this->m_breakpoints[TemporarySteppingBreakpointId].has_value()))
            return;

        const inst_t instruction = this->prefetch(PC);
        const InstructionHandler handler = this->decode(instruction);

        PC += InstructionWidth;
        this->execute(handler, instruction);

        if (this->m_debugMode) {
            for (const auto &breakpoint : this->m_breakpoints) {
                if (breakpoint.has_value() && breakpoint == PC) {
                    this->m_broken = true;
                    this->m_breakpoints[TemporarySteppingBreakpointId] = {};

                    Core::dumpRegisters();
                }
            }
        }
    }


    void Core::dumpRegisters() {
        Logger::info("== Register Dump ==");
        Logger::info(" N: %u Z: %u C: %u V: %u", PSTATE.N, PSTATE.Z, PSTATE.C, PSTATE.V);
        Logger::info(" PC:  0x%016llx", PC.W);
        Logger::info(" SP:  0x%016llx", GPSP(31).W);
        for (u8 i = 0; i < 31; i++)
            Logger::info(" W%02u: 0x%016llx", i, GPR[i].W);
    }

    void Core::enterDebugMode() {
        this->m_debugMode = true;
    }

    void Core::exitDebugMode() {
        this->m_debugMode = false;
    }

    void Core::breakCore() {
        this->m_broken = true;
        Core::enterDebugMode();
    }

    void Core::continueCore() {
        this->m_broken = false;
    }

    u8 Core::setBreakpoint(addr_t address) {
        for (u8 i = 0; i < NumBreakpoints; i++)
            if (!this->m_breakpoints[i].has_value()) {
                this->m_breakpoints[i] = address;
                return i;
            }

        return 0xFF;
    }

    void Core::removeBreakpoint(u8 breakpointId) {
        if (breakpointId >= NumBreakpoints)
            return;

        this->m_breakpoints[breakpointId] = {};
    }

    void Core::singleStep() {
        this->m_breakpoints[TemporarySteppingBreakpointId] = PC + InstructionWidth;
    }

    void Core::setNZCVFlags(u32 oldValue, u32 newValue) {
        PSTATE.N = (newValue >> 31) == 1;
        PSTATE.Z = newValue == 0;
        PSTATE.C = (oldValue >> 31) == 0 && (newValue >> 31) == 1;
        PSTATE.V = (oldValue >> 31) == 1 && (newValue >> 31) == 0;
    }

    void Core::setNZCVFlags(u64 oldValue, u64 newValue) {
        PSTATE.N = (newValue >> 63) == 1;
        PSTATE.Z = newValue == 0;
        PSTATE.C = (oldValue >> 63) == 0 && (newValue >> 63) == 1;
        PSTATE.V = (oldValue >> 63) == 1 && (newValue >> 63) == 0;
    }

    bool Core::doesConditionHold(u8 cond) const {
        bool conditionHolds = false;
        switch (cond >> 1) {
            case 0b000:
                conditionHolds = (PSTATE.Z == 1);
                break;                            // EQ or NE
            case 0b001:
                conditionHolds = (PSTATE.C == 1);
                break;                            // CS or CC
            case 0b010:
                conditionHolds = (PSTATE.N == 1);
                break;                            // MI or PL
            case 0b011:
                conditionHolds = (PSTATE.V == 1);
                break;                            // VS or VC
            case 0b100:
                conditionHolds = (PSTATE.C == 1 && PSTATE.Z == 0);
                break;           // HI or LS
            case 0b101:
                conditionHolds = (PSTATE.N == PSTATE.V);
                break;                     // GE or LT
            case 0b110:
                conditionHolds = (PSTATE.N == PSTATE.V && PSTATE.Z == 0);
                break;    // GT or LE
            case 0b111:
                conditionHolds = true;
                break;                                       // AL
        }

        if ((cond & 0b0001) == 1 && cond != 0b1111)
            conditionHolds = !conditionHolds;

        return conditionHolds;
    }

    [[nodiscard]] u64 Core::decodeImmediateWMask(u32 N, u32 imms, u32 immr) {
        s32 length = 31 - __builtin_clz((N << 6) | (~imms & 0x3F));
        u32 e = 1ULL << length;
        u32 levels = e - 1;
        u32 s = imms & levels;
        u32 r = immr & levels;

        u64 mask = (~0ULL >> (64 - (s + 1)));
        if (r != 0) {
            mask = (mask >> r) | (mask << (e - r));
            mask &= (~0ULL >> (64 - e));
        }

        while (e < 64) {
            mask |= mask << e;
            e <<= 1ULL;
        }

        return mask;
    }


    INSTRUCTION_DEF(NOP) {

    }

    INSTRUCTION_DEF(ORR_IMMEDIATE) {
        if (sf == 0)
            GPSP(Rd).X = GPZR(Rn).X | imm6;
        else
            GPSP(Rd).W = GPZR(Rn).W | imm6;
    }

    INSTRUCTION_DEF(ORR_SHIFTED_REGISTER) {
        if (sf == 0) {
            u32 operand2;
            switch (shift) {
                case 0b00: operand2 = GPZR(Rm).X << imm6; break;
                case 0b01: operand2 = GPZR(Rm).X >> imm6; break;
                case 0b10: operand2 = s32(GPZR(Rm).X) >> imm6; break;
                case 0b11: operand2 = std::rotr(GPZR(Rm).X, imm6); break;

            }
            GPZR(Rd).X = GPZR(Rn).X | operand2;
        } else {
            u64 operand2;
            switch (shift) {
                case 0b00: operand2 = GPZR(Rm).W << imm6; break;
                case 0b01: operand2 = GPZR(Rm).W >> imm6; break;
                case 0b10: operand2 = s64(GPZR(Rm).W) >> imm6; break;
                case 0b11: operand2 = std::rotr(GPZR(Rm).W, imm6); break;

            }
            GPZR(Rd).W = GPZR(Rn).W | operand2;
        }
    }

    INSTRUCTION_DEF(MOVZ) {
        u16 imm16 = extract<BITS(5:20)>(inst);
        u8 hw = extract<BITS(21:22)>(inst);

        if (sf == 0)
            GPZR(Rd).X = u32(imm16) << (hw << 4);
        else
            GPZR(Rd).W = u64(imm16) << (hw << 4);
    }

    INSTRUCTION_DEF(B) {
        s32 offset = extract<BITS(0:25)>(inst) * InstructionWidth;

        PC += offset - InstructionWidth;
    }

    INSTRUCTION_DEF(B_COND) {
        u8 cond = extract<BITS(0:3)>(inst);
        s32 offset = extendSign(extract<BITS(5:23)>(inst), 19, 32) * InstructionWidth;

        if (doesConditionHold(cond))
            PC += offset - InstructionWidth;
    }

    INSTRUCTION_DEF(BL) {
        s32 offset = extendSign(extract<BITS(0:25)>(inst), 26, 32) * InstructionWidth;

        GPR[30] = PC + InstructionWidth;
        PC += offset - InstructionWidth;
    }

    INSTRUCTION_DEF(ADD_IMMEDIATE) {
        u32 operand2 = 0;
        switch (shift) {
            case 0b00: operand2 = imm12; break;
            case 0b01: operand2 = imm12 << 12; break;
        }

        if (sf == 0) {
            u32 result = GPSP(Rn).X + operand2;
            GPSP(Rd).X = result;
        } else {
            u64 result = GPSP(Rn).W + operand2;
            GPSP(Rd).W = result;
        }
    }

    INSTRUCTION_DEF(ADD_SHIFTED_REGISTER) {
        u32 operand2 = 0;
        switch (shift) {
            case 0b00: operand2 = GPZR(Rm); break;
            case 0b01: operand2 = GPZR(Rm) << 12; break;
        }

        if (sf == 0) {
            GPZR(Rd).X = GPZR(Rn).X + operand2;
        } else {
            GPZR(Rd).W = GPZR(Rn).W + operand2;
        }
    }

    INSTRUCTION_DEF(ADDS_IMMEDIATE) {
        u32 operand2 = 0;
        switch (shift) {
            case 0b00: operand2 = imm12; break;
            case 0b01: operand2 = imm12 << 12; break;
        }

        if (sf == 0) {
            u32 result = GPSP(Rn).X + operand2;
            Core::setNZCVFlags(GPZR(Rd).X, result);
            GPZR(Rd).X = result;
        } else {
            u64 result = GPSP(Rn).W + operand2;
            Core::setNZCVFlags(GPZR(Rd).W, result);
            GPZR(Rd).W = result;
        }
    }

    INSTRUCTION_DEF(SUB_IMMEDIATE) {
        u32 operand2 = 0;
        switch (shift) {
            case 0b00: operand2 = imm12; break;
            case 0b01: operand2 = imm12 << 12; break;
        }

        if (sf == 0) {
            GPSP(Rd).X = GPSP(Rn).X - operand2;
        } else {
            GPSP(Rd).W = GPSP(Rn).W - operand2;
        }
    }

    INSTRUCTION_DEF(SUB_SHIFTED_REGISTER) {
        u32 operand2 = 0;
        switch (shift) {
            case 0b00: operand2 = GPZR(Rm); break;
            case 0b01: operand2 = GPZR(Rm) << 12; break;
        }

        if (sf == 0) {
            GPZR(Rd).X = GPZR(Rn).X - operand2;
        } else {
            GPZR(Rd).W = GPZR(Rn).W - operand2;
        }
    }

    INSTRUCTION_DEF(SUBS_IMMEDIATE) {
        u32 operand2 = 0;
        switch (shift) {
            case 0b00: operand2 = imm12; break;
            case 0b01: operand2 = imm12 << 12; break;
        }

        if (sf == 0) {
            u32 result = GPSP(Rn).X - operand2;
            Core::setNZCVFlags(GPZR(Rd).X, result);
            GPZR(Rd).X = result;
        } else {
            u64 result = GPSP(Rn).W - operand2;
            Core::setNZCVFlags(GPZR(Rd).W, result);
            GPZR(Rd).W = result;
        }
    }

    INSTRUCTION_DEF(SUBS_SHIFTED_REGISTER) {
        u32 operand2 = 0;
        switch (shift) {
            case 0b00: operand2 = GPZR(Rm); break;
            case 0b01: operand2 = GPZR(Rm) << 12; break;
        }

        if (sf == 0) {
            u32 result = GPZR(Rn).X - operand2;
            Core::setNZCVFlags(GPZR(Rd).X, result);
            GPZR(Rd).X = result;
        } else {
            u64 result = GPZR(Rn).X - operand2;
            Core::setNZCVFlags(GPZR(Rd).W, result);
            GPZR(Rd).W = result;
        }
    }

    INSTRUCTION_DEF(SUBS_EXTENDED_REGISTER) {

    }

    INSTRUCTION_DEF(CCMN_IMMEDIATE) {
        u8 imm5 = extract<BITS(16:20)>(inst);
        u8 cond = extract<BITS(12:15)>(inst);
        u8 nzcv = extract<BITS(0:3)>(inst);

        if (Core::doesConditionHold(cond)) {
            if (sf == 0) {
                u32 result = GPZR(Rn).X + imm5;
                Core::setNZCVFlags(GPZR(Rn).X, result);
            } else {
                u64 result = GPZR(Rn).W + imm5;
                Core::setNZCVFlags(GPZR(Rn).W, result);
            }
        } else {
            PSTATE.N = extract<BIT(3)>(nzcv);
            PSTATE.Z = extract<BIT(2)>(nzcv);
            PSTATE.C = extract<BIT(1)>(nzcv);
            PSTATE.V = extract<BIT(0)>(nzcv);
        }
    }

    INSTRUCTION_DEF(CCMN_REGISTER) {
        u8 cond = extract<BITS(12:15)>(inst);
        u8 nzcv = extract<BITS(0:3)>(inst);

        if (Core::doesConditionHold(cond)) {
            if (sf == 0) {
                u32 result = GPZR(Rn).X + GPZR(Rm).X;
                Core::setNZCVFlags(GPZR(Rn).X, result);
            } else {
                u64 result = GPZR(Rn).W + GPZR(Rm).W;
                Core::setNZCVFlags(GPZR(Rn).W, result);
            }
        } else {
            PSTATE.N = extract<BIT(3)>(nzcv);
            PSTATE.Z = extract<BIT(2)>(nzcv);
            PSTATE.C = extract<BIT(1)>(nzcv);
            PSTATE.V = extract<BIT(0)>(nzcv);
        }
    }

    INSTRUCTION_DEF(SVC) {
        u16 imm16 = extract<BITS(5:20)>(inst);
        Logger::info("Supervisor Called! (SVC %u)", imm16);
    }

    INSTRUCTION_DEF(ADRP) {
        s64 imm = extendSign(((extract<BITS(5:23)>(inst) << 2) | extract<BITS(29:30)>(inst)) << 12, 33, 64);

        GPZR(Rd) = (PC & 0xFFF) + imm;
    }

    INSTRUCTION_DEF(AND_IMMEDIATE) {
        if (sf == 0) {
            u32 imm = extendSign(decodeImmediateWMask(0, extract<BITS(10:15)>(inst), extract<BITS(16:21)>(inst)), 12, 32);
            GPSP(Rd).X = GPZR(Rd).X & imm;
        } else {
            u64 imm = decodeImmediateWMask(extract<BIT(22)>(inst), extract<BITS(10:15)>(inst), extract<BITS(16:21)>(inst));
            GPSP(Rd).W = GPZR(Rd).W & imm;
        }
    }

    INSTRUCTION_DEF(AND_SHIFTED_REGISTER) {
        if (sf == 0) {
            u32 operand2;
            switch (shift) {
                case 0b00: operand2 = GPZR(Rm).X << imm6; break;
                case 0b01: operand2 = GPZR(Rm).X >> imm6; break;
                case 0b10: operand2 = s32(GPZR(Rm).X) >> imm6; break;
                case 0b11: operand2 = std::rotr(GPZR(Rm).X, imm6); break;

            }
            GPZR(Rd).X = GPZR(Rn).X & operand2;
        } else {
            u64 operand2;
            switch (shift) {
                case 0b00: operand2 = GPZR(Rm).W << imm6; break;
                case 0b01: operand2 = GPZR(Rm).W >> imm6; break;
                case 0b10: operand2 = s64(GPZR(Rm).W) >> imm6; break;
                case 0b11: operand2 = std::rotr(GPZR(Rm).W, imm6); break;

            }
            GPZR(Rd).W = GPZR(Rn).W & operand2;
        }
    }

    INSTRUCTION_DEF(ANDS_IMMEDIATE) {
        if (sf == 0) {
            u32 imm = (extract<BITS(10:15)>(inst) << 6) | extract<BITS(16:21)>(inst);
            u32 result = GPZR(Rn).X & imm;
            Core::setNZCVFlags(GPZR(Rd).X, result);
            GPZR(Rd).X = result;
        } else {
            u64 imm = (extract<BIT(22)>(inst) << 12) | (extract<BITS(10:15)>(inst) << 6) | extract<BITS(16:21)>(inst);
            u64 result = GPZR(Rn).W & imm;
            Core::setNZCVFlags(GPZR(Rd).W, result);
            GPZR(Rd).W = result;
        }
    }

    INSTRUCTION_DEF(ANDS_SHIFTED_REGISTER) {
        if (sf == 0) {
            u32 operand2;
            switch (shift) {
                case 0b00: operand2 = GPZR(Rm).X << imm6; break;
                case 0b01: operand2 = GPZR(Rm).X >> imm6; break;
                case 0b10: operand2 = s32(GPZR(Rm).X) >> imm6; break;
                case 0b11: operand2 = std::rotr(GPZR(Rm).X, imm6); break;

            }
            u32 result = GPZR(Rn).X & operand2;
            Core::setNZCVFlags(GPZR(Rd).X, result);
            GPZR(Rd).X = result;
        } else {
            u64 operand2;
            switch (shift) {
                case 0b00: operand2 = GPZR(Rm).W << imm6; break;
                case 0b01: operand2 = GPZR(Rm).W >> imm6; break;
                case 0b10: operand2 = s64(GPZR(Rm).W) >> imm6; break;
                case 0b11: operand2 = std::rotr(GPZR(Rm).W, imm6); break;

            }
            u64 result = GPZR(Rn).W & operand2;
            Core::setNZCVFlags(GPZR(Rd).W, result);
            GPZR(Rd).W = result;
        }
    }

    INSTRUCTION_DEF(STR_IMMEDIATE) {
        u8 Rt = Rd;
        u16 imm9 = extract<BITS(12:20)>(inst);
        u8 scale = extract<BITS(31:30)>(inst);

        if (extract<BITS(24:25)>(inst) == 0b00 && extract<BITS(10:11)>(inst) == 0b01) { // Post-index
            s64 offset = extendSign(imm9, 9, 64);
            this->m_addressSpace->write(GPSP(Rn).W, 1U << scale, GPZR(Rt).W);

            if (scale == 0b10)
                GPSP(Rn).X += offset;
            else
                GPSP(Rn).W += offset;
        } else if (extract<BITS(24:25)>(inst) == 0b00 && extract<BITS(10:11)>(inst) == 0b11) { // Pre-index {
            s64 offset = extendSign(imm9, 9, 64);

            if (scale == 0b10)
                GPSP(Rn).X += offset;
            else
                GPSP(Rn).W += offset;

            this->m_addressSpace->write(GPSP(Rn).W, 1U << scale, GPZR(Rt).W);
        } else if (extract<BITS(24:25)>(inst) == 0b01) { // Unsigned offset
            s64 offset = extendSign(extract<BITS(10:21)>(inst), 12, 64) << scale;

            this->m_addressSpace->write(GPSP(Rn).W + offset, 1U << scale, GPZR(Rt).W);
        }

    }

    INSTRUCTION_DEF(STR_REGISTER) {
        u8 Rt = Rd;
        u8 scale = extract<BITS(31:30)>(inst);
        u8 option = extract<BITS(13:15)>(inst);
        u8 S = extract<BIT(12)>(inst);

        switch (option) {
            case 0b010: { // UXTW
                u32 offset = GPZR(Rm).X;
                this->m_addressSpace->write(GPSP(Rn).W + offset, 1U << scale, GPZR(Rt).W);
            } break;
            case 0b011: { // LSL
                u8 shiftAmount = 0;
                if (S == 0)
                    shiftAmount = 0;
                else if (scale == 0b10)
                    shiftAmount = 2;
                else if (scale == 0b11)
                    shiftAmount = 3;

                s32 offset = GPZR(Rm).X << shiftAmount;
                this->m_addressSpace->write(GPSP(Rn).W + offset, 1U << scale, GPZR(Rt).W);

            } break;
            case 0b110: { // SXTW
                s32 offset = GPZR(Rm).X;
                this->m_addressSpace->write(GPSP(Rn).W + offset, 1U << scale, GPZR(Rt).W);
            } break;
            case 0b111: { // SXTX
                s64 offset = extendSign(GPZR(Rm).W, 32, 64);
                this->m_addressSpace->write(GPSP(Rn).W + offset, 1U << scale, GPZR(Rt).W);
            }
        }

    }
    INSTRUCTION_DEF(LDR_IMMEDIATE) {
        u8 Rt = Rd;
        u16 imm9 = extract<BITS(12:20)>(inst);
        u8 scale = extract<BITS(31:30)>(inst);

        if (extract<BITS(24:25)>(inst) == 0b00 && extract<BITS(10:11)>(inst) == 0b01) { // Post-index
            s64 offset = extendSign(imm9, 9, 64);
            GPZR(Rt) = this->m_addressSpace->read(GPSP(Rn).W, 1U << scale);

            if (scale == 0b10)
                GPSP(Rn).X += offset;
            else
                GPSP(Rn).W += offset;
        } else if (extract<BITS(24:25)>(inst) == 0b00 && extract<BITS(10:11)>(inst) == 0b11) { // Pre-index {
            s64 offset = extendSign(imm9, 9, 64);

            if (scale == 0b10)
                GPSP(Rn).X += offset;
            else
                GPSP(Rn).W += offset;

            GPZR(Rt) = this->m_addressSpace->read(GPSP(Rn).W, 1U << scale);
        } else if (extract<BITS(24:25)>(inst) == 0b01) { // Unsigned offset
            s64 offset = extendSign(extract<BITS(10:21)>(inst), 12, 64) << scale;

            GPZR(Rt) = this->m_addressSpace->read(GPSP(Rn).W + offset, 1U << scale);
        }
    }

    INSTRUCTION_DEF(LDR_REGISTER) {
        u8 Rt = Rd;
        u8 scale = extract<BITS(31:30)>(inst);
        u8 option = extract<BITS(13:15)>(inst);
        u8 S = extract<BIT(12)>(inst);

        switch (option) {
            case 0b010: { // UXTW
                u32 offset = GPZR(Rm).X;
                GPZR(Rt) = this->m_addressSpace->read(GPSP(Rn).W + offset, 1U << scale);
            } break;
            case 0b011: { // LSL
                u8 shiftAmount = 0;
                if (S == 0)
                    shiftAmount = 0;
                else if (scale == 0b10)
                    shiftAmount = 2;
                else if (scale == 0b11)
                    shiftAmount = 3;

                s32 offset = GPZR(Rm).X << shiftAmount;
                GPZR(Rt) = this->m_addressSpace->read(GPSP(Rn).W + offset, 1U << scale);

            } break;
            case 0b110: { // SXTW
                s32 offset = GPZR(Rm).X;
                GPZR(Rt) = this->m_addressSpace->read(GPSP(Rn).W + offset, 1U << scale);
            } break;
            case 0b111: { // SXTX
                s64 offset = extendSign(GPZR(Rm).W, 32, 64);
                GPZR(Rt) = this->m_addressSpace->read(GPSP(Rn).W + offset, 1U << scale);
            }
        }
    }

}