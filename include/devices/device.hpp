#pragma once

#include <arm.hpp>

#include <type_traits>

namespace arm {

    class Device {
    public:
        explicit Device(size_t size) : m_size(size) {}
        virtual ~Device() {}

        virtual u64 read(offset_t offset, size_t size) = 0;
        virtual void write(offset_t offset, size_t size, u64 value) = 0;

        size_t getSize() const { return this->m_size; }

        template<typename T>
        static T* as(Device *device) requires std::is_base_of_v<Device, T> {
            return static_cast<T*>(device);
        }

    private:
        const size_t m_size;
    };

}