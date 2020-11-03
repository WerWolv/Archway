#pragma once

#include <armv8.hpp>

#include <concepts>

namespace arm::dev {

    class Device {
    public:
        Device(size_t size) : m_size(size) { }
        virtual ~Device() { }

        constexpr size_t getSize() const noexcept {
            return this->m_size;
        }

        virtual constexpr void read(addr_t address, size_t size, u8 *data) = 0;
        virtual constexpr void write(addr_t address, size_t size, u8 *data) = 0;
        virtual constexpr void clear(addr_t address, size_t size) {}

    private:
        const size_t m_size;
    };

}