#pragma once

#include "devices/device.hpp"

namespace arm::dev {

    class Memory : public Device {
    public:
        explicit Memory(size_t size);
        virtual ~Memory();

        virtual u64 read(offset_t offset, size_t size);
        virtual void write(offset_t offset, size_t size, u64 value);

        void load(const std::string &path);
        void load(const std::initializer_list<inst_t> &instructions);
        void load(const u8 *data, const size_t size);
    private:
        u8 *m_memory = nullptr;
    };

}