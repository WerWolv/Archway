#include "board.hpp"

#include "devices/memory.hpp"
#include "devices/uart.hpp"

namespace arm {

    Board::Board() {
        BROM  = new dev::Memory(10_MiB);
        IRAM  = new dev::Memory(1_MiB);
        DRAM  = new dev::Memory(2_MiB);
        FLASH = new dev::Memory(100_MiB);

        UART1 = new dev::UART();

        CPU.addDeviceToAddressSpace(BROM,  0x0000'0000'0000'0000);
        CPU.addDeviceToAddressSpace(IRAM,  0x1000'0000'0000'0000);
        CPU.addDeviceToAddressSpace(DRAM,  0x2000'0000'0000'0000);
        CPU.addDeviceToAddressSpace(FLASH, 0x3000'0000'0000'0000);
        CPU.addDeviceToAddressSpace(UART1, 0x8000'0000'0000'0000);

        //Device::as<dev::Memory>(BROM)->load("test.elf");
        Device::as<dev::Memory>(BROM)->load((const u8*)"\x00\x00\x80\xd2\x00\x00\xa0\xf2\x00\x00\xc0\xf2\x00\x00\xf0\xf2\x21\x08\x80\xd2\x01\x00\x00\xf9", 24);

        this->CPU.reset();
    }

    Board::~Board() {
        this->CPU.reset();

        delete FLASH;
        delete DRAM;
        delete IRAM;
        delete BROM;
    }

    void Board::powerUp() {
        this->m_powered = true;
    }

    void Board::tick() {
        if (this->m_powered) {
            this->CPU.tick();

            for (u8 coreId = 0; coreId < this->CPU.getCoreCount(); coreId++) {
                auto& core = this->CPU.getCore(coreId);
            }
        }
    }

}