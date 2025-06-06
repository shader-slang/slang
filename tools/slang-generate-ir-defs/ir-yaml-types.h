#pragma once

#include "core/slang-list.h"
#include "core/slang-string.h"

#include <cstdint>

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


struct Entry
{
    enum Flavor
    {
        Instruction,
        Range
    };

    Flavor flavor;
    Slang::String name; // mnemonic for instructions, name for ranges
    Slang::String comment;
    IROpFlags flags = IROpFlags::None;

    // Instruction-specific fields (only used when type == INSTRUCTION)
    Slang::String type_name;
    int operands = 0;

    // Range-specific fields (only used when type == RANGE)
    Slang::List<Entry> children; // Nested entries for ranges

    // Helper methods
    bool isInstruction() const { return flavor == Instruction; }
    bool isRange() const { return flavor == Range; }

    // Helper to find first and last instruction in range
    const Entry* findFirstInstruction() const;
    const Entry* findLastInstruction() const;
};

struct InstructionSet
{
    Slang::List<Entry> entries;
};
