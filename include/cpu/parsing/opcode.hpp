#pragma once

#include <armv8.hpp>
#include <stdexcept>

namespace arm {

    class OpcodeEncoding {
    public:
        consteval OpcodeEncoding(const char *encoding) noexcept {
            s32 position = 31;
            for (const char *currChar = encoding; currChar != encoding + __builtin_strlen(encoding); currChar++) {
                switch (*currChar) {
                    case '\'': continue;
                    case ' ':  continue;
                    case '0':  break;
                    case '1':
                        pattern |= (1UL << position);
                        break;
                    case 'x':
                    case 'X':
                        mask &= ~(1UL << position);
                        break;
                    default:
                        throw std::invalid_argument("Invalid character in pattern!");
                }

                position--;
            }

            if (position != -1)
                throw std::invalid_argument("Pattern must have exactly 32 bits!");
        }

        [[nodiscard]] constexpr inline bool matches(const u32 opcode) const noexcept {
            return (opcode & this->mask) == this->pattern;
        }

    private:
        u32 mask = 0xFFFF'FFFF, pattern = 0x0000'0000;
    };

}