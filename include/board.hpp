#pragma once

#include <arm.hpp>

#include "cpu.hpp"
#include "core.hpp"
#include "address_space.hpp"

namespace arm {

    class Board {
    public:
        Board();
        ~Board();

        void powerUp();
        void reset();

    private:
        Cpu CPU = Cpu(1);
        Device *BROM;
        Device *IRAM;
        Device *DRAM;
        Device *FLASH;

        bool m_powered = false;

        void run();
    };

}