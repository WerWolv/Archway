#include "board.hpp"

#include "devices/memory.hpp"

namespace arm {

    Board::Board() {
        BROM  = new dev::Memory(10_MiB);
        IRAM  = new dev::Memory(1_MiB);
        DRAM  = new dev::Memory(2_MiB);
        FLASH = new dev::Memory(100_MiB);

        CPU.addDeviceToAddressSpace(BROM,  0x0000'0000'0000'0000);
        CPU.addDeviceToAddressSpace(IRAM,  0x1000'0000'0000'0000);
        CPU.addDeviceToAddressSpace(DRAM,  0x2000'0000'0000'0000);
        CPU.addDeviceToAddressSpace(FLASH, 0x3000'0000'0000'0000);

        Device::as<dev::Memory>(BROM)->load("test.elf");
        /*Device::as<dev::Memory>(BROM)->load({
            0xAA1E03E7,
            0x94000001
        });*/

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