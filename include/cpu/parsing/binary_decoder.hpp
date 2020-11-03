#pragma once

#include <armv8.hpp>

#include <array>
#include <stdexcept>

#include "cpu/arm_functions.hpp"

namespace arm {

    class BinaryDecoder {
    public:
        consteval BinaryDecoder(const char *encoding) {
            char patternCharacter = 'A';
            bool foundPatternCharacter = false;

            do {
                uint8_t patternOccurance = 0;
                int16_t position = 31;
                bool newPattern = false;

                foundPatternCharacter = false;

                for (const char *currChar = encoding; currChar != encoding + __builtin_strlen(encoding); currChar++) {
                    if (*currChar == ' ' || *currChar == '\'') continue;

                    if (*currChar == patternCharacter) {

                        if (!newPattern) {
                            parts[patternCharacter - 'A'][patternOccurance].first = position + 1;
                            foundPatternCharacter = true;
                            newPattern = true;
                        }

                    } else {
                        if (newPattern) {
                            parts[patternCharacter - 'A'][patternOccurance].second = position + 1;
                            patternOccurance++;
                            newPattern = false;
                        }
                    }

                    position--;
                }

                if (position != -1)
                    throw std::invalid_argument("Pattern must be exactly 32 bit long");

                if (*(encoding + __builtin_strlen(encoding) - 1) == patternCharacter) {
                    parts[patternCharacter - 'A'][patternOccurance].second = position + 1;
                    patternOccurance++;
                }

                partsCount[patternCharacter - 'A'] = patternOccurance;
                patternCharacter++;
            } while (foundPatternCharacter);


        }

        [[nodiscard]] constexpr uint64_t get(char character, uint32_t input, bool lowFirst = true) const noexcept {
            const auto &currParts = this->parts[character - 'A'];
            size_t count = this->partsCount[character - 'A'];

            if (count == 0)
                return 0x00;

            uint64_t result = 0x00;

            uint8_t currPosition = 0;
            if (lowFirst) {
                for (int8_t i = 0; i < count; i++) {
                    size_t width = currParts[i].first - currParts[i].second;

                    result |= ((input & makeBitmask(currParts[i].second, currParts[i].first)) >> currParts[i].second) << currPosition;
                    currPosition += width;
                }
            } else {
                for (int8_t i = count - 1; i >= 0; i--) {
                    size_t width = currParts[i].first - currParts[i].second;

                    result |= ((input & makeBitmask(currParts[i].second, currParts[i].first)) >> currParts[i].second) << currPosition;
                    currPosition += width;
                }
            }


            return result;
        }

    private:
        std::array<std::array<std::pair<uint32_t, uint32_t>, 32>, 10> parts = { { } };
        std::array<size_t, 10> partsCount = { 0 };
    };

}