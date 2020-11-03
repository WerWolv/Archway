#pragma once

#include <armv8.hpp>

#include <bit>
#include <cstring>
#include <stdexcept>
#include <vector>

#include <elf.h>

#include "devices/device.hpp"

namespace arm {

    class AddressSpace {
    public:
        ~AddressSpace() {
            for (auto& [base, device] : this->m_devices)
                delete device;

            this->m_devices.clear();
        }

        void addDevice(addr_t address, dev::Device *device) {
            for (const auto& [base, device] : this->m_devices) {
                if (address >= base && address < base + device->getSize())
                    throw formatException<std::invalid_argument>("Address 0x%016lx lies within address range of another device!", address);
            }

            this->m_devices.push_back({ address, device });
        }

        template<std::integral T>
        T read(addr_t address) {
            T data;
            this->read(address, &data, sizeof(T));

            return data;
        }

        void read(addr_t address, void *data, size_t size) {
            for (const auto& [base, device] : this->m_devices) {
                if (address >= base && address < (base + device->getSize())) {
                    device->read(address - base, size, reinterpret_cast<u8*>(data));
                    return;
                }
            }

            throw formatException<std::invalid_argument>("No device at address %016lx:%lx!", address, size);
        }

        template<std::integral T>
        void write(addr_t address, T data) {
            this->write(address, &data, sizeof(T));
        }

        void write(addr_t address, void *data, size_t size) {
            for (const auto& [base, device] : this->m_devices) {
                if (address >= base && address < base + device->getSize()) {
                    device->write(address - base, size, reinterpret_cast<u8*>(data));
                    return;
                }
            }

            throw formatException<std::invalid_argument>("No device at address 0x%016lx", address);
        }

        void clear(addr_t address, size_t size) {
            for (const auto& [base, device] : this->m_devices) {
                if (address >= base && address < base + device->getSize()) {
                    device->clear(address - base, size);
                    return;
                }
            }
        }

        void load(addr_t address, std::initializer_list<Instruction> instructions) {
            for (const auto& instr : instructions) {
                this->write(address, instr);
                address += sizeof(Instruction);
            }
        }

        addr_t loadELF(std::string_view elfPath) {
            FILE *elfFile = fopen(elfPath.data(), "rb");
            size_t fileSize = 0;

            addr_t entryPoint = 0x00;

            if (elfFile == nullptr)
                throw std::invalid_argument("ELF file does not exist!");

            fseek(elfFile, 0, SEEK_END);
            fileSize = ftell(elfFile);
            rewind(elfFile);

            std::vector<u8> data(fileSize, 0x00);
            fread(data.data(), fileSize, 1, elfFile);

            if (std::memcmp(&data[EI_MAG0], ELFMAG, SELFMAG) != 0)
                throw std::runtime_error("File is not in a valid ELF format!");

            auto& machine = *reinterpret_cast<u16*>(&data[0x12]);

            if (machine != EM_ARM && machine != EM_AARCH64)
                throw std::invalid_argument("ELF file does not contain ARM or ARM64 byte code!");

            if (data[EI_CLASS] == ELFCLASS32) {
                auto& header = *reinterpret_cast<Elf32_Ehdr*>(&data[EI_MAG0]);

                printf("Loading 32 bit ELF\n");

                entryPoint  = header.e_entry;

                for (u32 i = 0; i < header.e_phnum; i++) {
                    auto& segmentHeader = *reinterpret_cast<Elf32_Phdr*>(&data[header.e_phoff + i * sizeof(Elf32_Phdr)]);

                    if (segmentHeader.p_type != PT_LOAD)
                        continue;

                    printf("Loading segment at 0x%08x:0x%x to 0x%08x:0x%x \n", segmentHeader.p_offset, segmentHeader.p_filesz, segmentHeader.p_paddr, segmentHeader.p_memsz);

                    if (segmentHeader.p_memsz != segmentHeader.p_filesz)
                        this->clear(segmentHeader.p_paddr, segmentHeader.p_memsz);

                    this->write(segmentHeader.p_paddr, &data[segmentHeader.p_offset], segmentHeader.p_filesz);
                }
            } else if (data[EI_CLASS] == ELFCLASS64) {
                auto& header = *reinterpret_cast<Elf64_Ehdr*>(&data[EI_MAG0]);

                printf("Loading 64 bit ELF\n");

                entryPoint  = header.e_entry;

                for (u32 i = 0; i < header.e_phnum; i++) {
                    auto& segmentHeader = *reinterpret_cast<Elf64_Phdr *>(&data[header.e_phoff + i * sizeof(Elf64_Phdr)]);

                    if (segmentHeader.p_type != PT_LOAD)
                        continue;

                    printf("Loading segment at 0x%08lx:0x%lx to 0x%08lx:0x%lx \n", segmentHeader.p_offset, segmentHeader.p_filesz, segmentHeader.p_paddr, segmentHeader.p_memsz);

                    if (segmentHeader.p_memsz != segmentHeader.p_filesz)
                        this->clear(segmentHeader.p_paddr, segmentHeader.p_memsz);

                    this->write(segmentHeader.p_paddr, &data[segmentHeader.p_offset], segmentHeader.p_filesz);
                }

            } else
                throw std::runtime_error("Invalid ELF class!");


            printf("Entry point at 0x%016lx\n", entryPoint);

            return entryPoint;
        }

    private:
        std::vector<std::pair<addr_t, dev::Device*>> m_devices;
    };

}