#pragma once

#include <armv8.hpp>

#include "cpu/parsing/binary_decoder.hpp"

#include <cstring>
#include <cstdio>

namespace arm {

    enum class ExecutionResult : u8 {
        Success
    };

    struct DataProcessingImmediate {
        DataProcessingImmediate() = delete;

         struct PCRelative {
             PCRelative(Instruction input) {
                 constexpr BinaryDecoder decoder("ABBC'CCCC'BBBB'BBBB'BBBB'BBBB'BBBD'DDDD");

                 this->op    = decoder.get('A', input);
                 this->instr = decoder.get('C', input);
                 this->imm21 = decoder.get('B', input);
                 this->Rd    = decoder.get('D', input);
             }

             u8 Rd      = 0;
             u32 imm21  = 0;
             u8 instr   = 0;
             u8 op      = 0;
         };

         struct AddSubtractImmediate {
             AddSubtractImmediate(Instruction input) {
                 constexpr BinaryDecoder decoder("ABCD'DDDD'DEFF'FFFF'FFFF'FFGG'GGGH'HHHH");

                 this->sf       = decoder.get('A', input);
                 this->op       = decoder.get('B', input);
                 this->S        = decoder.get('C', input);
                 this->instr    = decoder.get('D', input);
                 this->sh       = decoder.get('E', input);
                 this->imm12    = decoder.get('F', input);
                 this->Rn       = decoder.get('G', input);
                 this->Rd       = decoder.get('H', input);
             }

             u8 sf      = 0;
             u8 op      = 0;
             u8 S       = 0;
             u8 instr   = 0;
             u8 sh      = 0;
             u16 imm12  = 0;
             u8 Rn      = 0;
             u8 Rd      = 0;
         };

         struct LogicalImmediate {
             LogicalImmediate(Instruction input) {
                 constexpr BinaryDecoder decoder("ABBC'CCCC'CDEE'EEEE'FFFF'FFGG'GGGH'HHHH");

                 this->sf       = decoder.get('A', input);
                 this->opc      = decoder.get('B', input);
                 this->instr    = decoder.get('C', input);
                 this->N        = decoder.get('D', input);
                 this->immr     = decoder.get('E', input);
                 this->imms     = decoder.get('F', input);
                 this->Rn       = decoder.get('G', input);
                 this->Rd       = decoder.get('H', input);
             }

             u8 sf      = 0;
             u8 opc     = 0;
             u8 instr   = 0;
             u8 N       = 0;
             u8 immr    = 0;
             u8 imms    = 0;
             u8 Rn      = 0;
             u8 Rd      = 0;
         };

         struct MoveWideImmediate {
             MoveWideImmediate(Instruction input) {
                 constexpr BinaryDecoder decoder("ABB-'----'-CCD'DDDD'DDDD'DDDD'DDDE'EEEE");

                 this->sf       = decoder.get('A', input);
                 this->opc      = decoder.get('B', input);
                 this->hw       = decoder.get('C', input);
                 this->imm16    = decoder.get('D', input);
                 this->Rd       = decoder.get('E', input);
             }

             u8 sf      = 0;
             u8 opc     = 0;
             u8 hw      = 0;
             u16 imm16  = 0;
             u8 Rd      = 0;
         };
    };

    struct Branches {
        struct UnconditionalBranchImmediate {
            UnconditionalBranchImmediate(Instruction input) {
                constexpr BinaryDecoder decoder("A---'--BB'BBBB'BBBB'BBBB'BBBB'BBBB'BBBB");

                this->op    = decoder.get('A', input);
                this->imm26 = decoder.get('B', input);
            }

            u8 op       = 0;
            u32 imm26   = 0;
        };

        struct ConditionalBranchImmediate {
            ConditionalBranchImmediate(Instruction input) {
                constexpr BinaryDecoder decoder("----'---A'BBBB'BBBB'BBBB'BBBB'BBBA'CCCC");

                this->o     = decoder.get('A', input);
                this->imm19 = decoder.get('B', input);
                this->cond  = decoder.get('C', input);
            }

            u8 o        = 0;
            u32 imm19   = 0;
            u8 cond     = 0;
        };

        struct ExceptionGeneration {
            ExceptionGeneration(Instruction input) {
                constexpr BinaryDecoder decoder("----'----'AAAB'BBBB'BBBB'BBBB'BBBC'CCDD");

                this->opc   = decoder.get('A', input);
                this->imm16 = decoder.get('B', input);
                this->op2   = decoder.get('C', input);
                this->LL    = decoder.get('D', input);
            }

            u8 opc      = 0;
            u16 imm16   = 0;
            u8 op2      = 0;
            u8 LL       = 0;
        };

        struct Hint {
            Hint(Instruction input) {
                constexpr BinaryDecoder decoder("----'----'----'----'----'AAAA'BBB-'----");

                this->CRm = decoder.get('A', input);
                this->op2 = decoder.get('B', input);
            }

            u8 CRm  = 0;
            u8 op2  = 0;
        };
    };

    struct DataProcessingRegister {

        struct TwoSource {
            TwoSource(Instruction input) {
                constexpr BinaryDecoder decoder("A-B-'----'---C'CCCC'DDDD'DDEE'EEEF'FFFF");

                this->sf        = decoder.get('A', input);
                this->S         = decoder.get('B', input);
                this->Rm        = decoder.get('C', input);
                this->opcode    = decoder.get('D', input);
                this->Rn        = decoder.get('E', input);
                this->Rd        = decoder.get('F', input);
            }

            u8 sf       = 0;
            u8 S        = 0;
            u8 Rm       = 0;
            u8 opcode   = 0;
            u8 Rn       = 0;
            u8 Rd       = 0;
        };

        struct OneSource {
            OneSource(Instruction input) {
                constexpr BinaryDecoder decoder("A-B-'----'---C'CCCC'DDDD'DDEE'EEEF'FFFF");

                this->sf        = decoder.get('A', input);
                this->S         = decoder.get('B', input);
                this->opcode2   = decoder.get('C', input);
                this->opcode    = decoder.get('D', input);
                this->Rn        = decoder.get('E', input);
                this->Rd        = decoder.get('F', input);
            }

            u8 sf       = 0;
            u8 S        = 0;
            u8 opcode2  = 0;
            u8 opcode   = 0;
            u8 Rn       = 0;
            u8 Rd       = 0;
        };

        struct LogicalShiftedRegister {
            LogicalShiftedRegister(Instruction input) {
                constexpr BinaryDecoder decoder("ABB-'----'CCDE'EEEE'FFFF'FFGG'GGGH'HHHH");

                this->sf        = decoder.get('A', input);
                this->opc       = decoder.get('B', input);
                this->shift     = decoder.get('C', input);
                this->N         = decoder.get('D', input);
                this->Rm        = decoder.get('E', input);
                this->imm6      = decoder.get('F', input);
                this->Rn        = decoder.get('G', input);
                this->Rd        = decoder.get('H', input);
            }

            u8 sf       = 0;
            u8 opc      = 0;
            u8 shift    = 0;
            u8 N        = 0;
            u8 Rm       = 0;
            u8 imm6     = 0;
            u8 Rn       = 0;
            u8 Rd       = 0;
        };

        struct ConditionalComparison {
            ConditionalComparison(Instruction input) {
                constexpr BinaryDecoder decoder("AB--'----'---C'CCCC'DDDD'--EE'EEE-'FGHI");

                this->sf        = decoder.get('A', input);
                this->op        = decoder.get('B', input);
                this->imm5      = decoder.get('C', input);
                this->cond      = decoder.get('D', input);
                this->Rn        = decoder.get('E', input);
                this->nzcv.N    = decoder.get('F', input);
                this->nzcv.Z    = decoder.get('G', input);
                this->nzcv.C    = decoder.get('H', input);
                this->nzcv.V    = decoder.get('I', input);
            }

            u8 sf   = 0;
            u8 op   = 0;
            u8 imm5 = 0;
            u8 cond = 0;
            u8 Rn   = 0;
            NZCV nzcv = { 0 };
        };

    };


    struct LoadsAndStores {

        struct RegisterImmediate {
            RegisterImmediate(Instruction input) {
                constexpr BinaryDecoder decoder("AA--'-B--'CC-D'DDDD'DDDD'EEFF'FFFG'GGGG");

                this->size  = decoder.get('A', input);
                this->V     = decoder.get('B', input);
                this->opc   = decoder.get('C', input);
                this->imm9  = decoder.get('D', input);
                this->op    = decoder.get('E', input);
                this->Rn    = decoder.get('F', input);
                this->Rt    = decoder.get('G', input);
            }

            u8 size     = 0;
            u8 V        = 0;
            u8 opc      = 0;
            u16 imm9    = 0;
            u8 op       = 0;
            u8 Rn       = 0;
            u8 Rt       = 0;
        };

    };

}