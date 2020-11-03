#include "cpu/core.hpp"

#include <array>
#include <cpu/core.hpp>

#include "devices/memory.hpp"

namespace arm {

    Core::Core() {
        this->addressSpace.addDevice(0x0000'0000, new dev::Memory(0x100'0000));

        this->regs.PC() = this->addressSpace.loadELF("test.elf");
    }

    /* Pipeline stages */

    Instruction Core::prefetch() {
        Instruction fetchedInstruction = this->addressSpace.read<Instruction>(this->regs.PC());

        this->regs.PC() += sizeof(Instruction);

        return fetchedInstruction;
    }

    auto Core::decode(Instruction instruction) {
        for(const auto& opcode : getOpcodes()) {
            if (opcode.encoding.matches(instruction))
                return [this, instruction, opcode] { log::debug("%08lx : %s\n", regs.PC() - 8, opcode.name); return (this->*opcode.callback)(instruction); };
        }


        throw formatException<std::runtime_error>("Unknown opcode 0x%08lx at %016lx!", instruction, regs.PC() - sizeof(Instruction));
    }

    ExecutionResult Core::execute(std::function<ExecutionResult()> &handler) {
        return handler();
    }

    void Core::tick() {
        if (this->pipeline.validSteps >= 2) {
            this->pipeline.validSteps = 3;
            this->pipeline.executeStep = this->execute(this->pipeline.decodeStep);
        }

        if (this->pipeline.validSteps >= 1) {
            this->pipeline.validSteps = 2;
            this->pipeline.decodeStep = this->decode(this->pipeline.prefetchStep);
        }

        this->pipeline.prefetchStep = this->prefetch();
        if (this->pipeline.validSteps == 0)
            this->pipeline.validSteps = 1;
    }

}