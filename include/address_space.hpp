#pragma once

#include <arm.hpp>
#include "devices/device.hpp"

#include <array>
#include <unordered_map>

namespace arm {

    class AddressSpace {
    public:
        void addDevice(Device *newDevice, addr_t baseAddress);

        u64 read(addr_t address, size_t size);
        void write(addr_t address, size_t size, u64 value);
    private:
        std::unordered_map<addr_t, Device*> m_memoryRegions;
    };

}