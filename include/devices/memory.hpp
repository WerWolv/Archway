#pragma once

#include <armv8.hpp>
#include <cstring>
#include <stdexcept>

#include "devices/device.hpp"

namespace arm::dev {

    class Memory : public Device {
    public:
        Memory(size_t size) : Device(size) {
            this->m_data = new u8[size];
            std::memset(this->m_data, size, 0x00);
        }

        virtual ~Memory() {
            delete[] this->m_data;
        }

        virtual constexpr void read(addr_t address, size_t size, u8 *data) {
            if (address < 0 || (address + size) >= this->getSize())
                throw formatException<std::invalid_argument>("Reading address 0x%016lx out of range!", address);

            std::memcpy(data, this->m_data + address, size);
        }

        virtual constexpr void write(addr_t address, size_t size, u8 *data) {
            if (address < 0 || (address + size) >= this->getSize())
                throw formatException<std::invalid_argument>("Writing address 0x%016lx out of range!", address);

            std::memcpy(this->m_data + address, data, size);
        }

        virtual constexpr void clear(addr_t address, size_t size) {
            if (address < 0 || (address + size) >= this->getSize())
                throw formatException<std::invalid_argument>("Clearing address 0x%016lx out of range!", address);

            std::memset(this->m_data + address, 0x00, size);
        }

    private:
        u8 *m_data;
    };

}