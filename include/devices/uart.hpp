#pragma once

#include "devices/device.hpp"

namespace arm::dev {

    class UART : public Device {
    public:
        explicit UART();
        virtual ~UART();

        virtual u64 read(offset_t offset, size_t size);
        virtual void write(offset_t offset, size_t size, u64 value);
    private:
    };

}