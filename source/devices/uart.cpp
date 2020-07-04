#include "devices/uart.hpp"

namespace arm::dev {

    UART::UART() : Device(sizeof(u64)) {
    }

    UART::~UART() {
    }

    u64 UART::read(offset_t offset, size_t size) {
        if (offset + size > this->getSize())
            Logger::fatal("Tried to access an invalid address at BASE + %016llx!", offset);
        if (size > sizeof(u64))
            Logger::fatal("Tried to read more than 8 bytes: %u", size);

        return 0x00;
    }

    void UART::write(offset_t offset, size_t size, u64 value) {
        if (offset + size > this->getSize())
            Logger::fatal("Tried to access an invalid address at BASE + %016llx!", offset);
        if (size > sizeof(u64))
            Logger::fatal("Tried to write more than 8 bytes: %u!", size);

        Logger::info("UART Output: %c", static_cast<char>(value));
    }

}