#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Only coincidence that this has the same members as in slang-ir.h
enum class IROpFlags : uint32_t
{
    None = 0,
    Parent = 1 << 0,    // 0x01
    UseOther = 1 << 1,  // 0x02
    Hoistable = 1 << 2, // 0x04
    Global = 1 << 3     // 0x08
};

inline IROpFlags operator|(IROpFlags lhs, IROpFlags rhs)
{
    return static_cast<IROpFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

inline IROpFlags operator&(IROpFlags lhs, IROpFlags rhs)
{
    return static_cast<IROpFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}


// Forward declarations
struct InstructionEntry;
struct RangeEntry;

// Base class for entries (either instruction or range)
struct Entry
{
    enum Type
    {
        INSTRUCTION,
        RANGE
    };
    virtual Type getType() const = 0;
    virtual ~Entry() = default;
};

// Instruction definition
struct InstructionEntry : Entry
{
    std::string mnemonic;
    std::string comment;
    std::string type_name;
    int operands = 0;
    std::vector<std::string> flag_strings;
    IROpFlags flags = IROpFlags::None;

    Type getType() const override { return INSTRUCTION; }
};

// Range definition
struct RangeEntry : Entry
{
    std::string name;
    std::string comment;
    std::vector<std::string> flag_strings;
    IROpFlags flags = IROpFlags::None;
    std::vector<std::unique_ptr<Entry>> insts;

    Type getType() const override { return RANGE; }

    // Helper to find first and last instruction in range
    const InstructionEntry* findFirstInstruction() const;
    const InstructionEntry* findLastInstruction() const;
};

// Root structure
struct InstructionSet
{
    std::vector<std::unique_ptr<Entry>> insts;
};
