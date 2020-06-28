#include "devices/memory.hpp"

namespace arm::dev {

    Memory::Memory(size_t size) : Device(size) {
        this->m_memory = new u8[size];
    }

    Memory::~Memory() {
        delete[] this->m_memory;
    }

    u64 Memory::read(offset_t offset, size_t size) {
        if (offset + size >= this->getSize())
            Logger::fatal("Tried to access an invalid address at BASE + %016llx!", offset);
        if (size > sizeof(u64))
            Logger::fatal("Tried to read more than 8 bytes: %u", size);

        u64 out = 0x00;
        memcpy(&out, &this->m_memory[offset], size);

        return out;
    }

    void Memory::write(offset_t offset, size_t size, u64 value) {
        if (offset + size >= this->getSize())
            Logger::fatal("Tried to access an invalid address at BASE + %016llx!", offset);
        if (size > sizeof(u64))
            Logger::fatal("Tried to write more than 8 bytes: %u!", size);

        memcpy(&this->m_memory[offset], &value, size);
    }

    void Memory::load(const std::string &path) {
        FILE *file = fopen(path.c_str(), "r");
        if (file == nullptr)
            Logger::fatal("File " + path + " cannot be read!");

        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        rewind(file);

        if (fileSize > this->getSize())
            Logger::fatal("File content of size 0x%lX does not fit into memory region of size 0x%lX!", fileSize, this->getSize());

        fread(this->m_memory, 1, fileSize, file);
        fclose(file);
    }

    void Memory::load(const std::initializer_list<inst_t> &instructions) {
        addr_t address = 0;

        if (instructions.size() * InstructionWidth > this->getSize())
            Logger::fatal("Instructions with total size of 0x%lX does not fit into memory region of size 0x%lX!");

        for (const auto& instruction : instructions) {
            this->write(address, InstructionWidth, instruction);
            address += InstructionWidth;
        }
    }

    void Memory::load(const u8 *data, const size_t size) {
        if (size > this->getSize())
            return;
        memcpy(this->m_memory, data, size);
    }

}