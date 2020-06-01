#include "address_space.hpp"

namespace arm {

    void AddressSpace::addDevice(Device *newDevice, addr_t baseAddress) {
        for (auto &[regionBase, device] : this->m_memoryRegions)
            if (baseAddress >= regionBase && baseAddress + newDevice->getSize() < regionBase + device->getSize())
                Logger::fatal("Tried to add new device to address space at address %016llx - %016llx which overlaps device at %016llx - %016llx!",
                        baseAddress, baseAddress + newDevice->getSize(), regionBase, regionBase + device->getSize());

        this->m_memoryRegions.insert({ baseAddress, newDevice });
    }

    u64 AddressSpace::read(addr_t address, size_t size) {
        for (auto &[baseAddress, device] : this->m_memoryRegions)
            if (address + size >= baseAddress && address <= baseAddress + device->getSize()) {
                return device->read(address - baseAddress, size);
            }

        Logger::fatal("Tried to access an invalid address at %016llx!", address);
    }

    void AddressSpace::write(addr_t address, size_t size, u64 value) {
        for (auto &[baseAddress, device] : this->m_memoryRegions)
            if (address + size >= baseAddress && address <= baseAddress + device->getSize()) {
                device->write(address - baseAddress, size, value);
                return;
            }

        Logger::fatal("Tried to write to an invalid address at %016llx!", address);
    }

}