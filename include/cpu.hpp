#pragma once

#include "core.hpp"
#include "address_space.hpp"

#include <vector>

namespace arm {

    class Cpu {
    public:
        Cpu(u8 numCores);
        ~Cpu();

        void tick();
        void reset();

        void addDeviceToAddressSpace(Device *device, addr_t baseAddress);

        u8 getCoreCount();
        Core& getCore(u8 id);

    private:
        u8 m_numCores;
        std::vector<Core> m_cores;

        AddressSpace m_addressSpace;
    };

}

