#include "cpu.hpp"

namespace arm {

    Cpu::Cpu(u8 numCores) : m_numCores(numCores) {
        for (u8 i = 0; i < numCores; i++)
            this->m_cores.emplace_back(Core(&this->m_addressSpace));
    }

    Cpu::~Cpu() {
        for (u8 i = 0; i < this->m_numCores; i++) {
            this->m_cores[i].halt();
        }
    }

    void Cpu::reset() {
        for (u8 i = 0; i < this->m_numCores; i++)
            this->m_cores[i].reset();
    }

    void Cpu::tick() {
        for (u8 core = 0; core < this->m_numCores; core++)
            this->m_cores[core].tick();
    }

    u8 Cpu::getCoreCount() {
        return this->m_numCores;
    }

    Core& Cpu::getCore(u8 id) {
        return this->m_cores[id];
    }

    void Cpu::addDeviceToAddressSpace(Device *device, addr_t baseAddress) {
        this->m_addressSpace.addDevice(device, baseAddress);
    }

}