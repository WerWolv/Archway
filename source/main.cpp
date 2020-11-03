#include <iostream>

#include "cpu/core.hpp"

int main() {
    arm::Core core;

    while(true)
        core.tick();

    return 0;
}
