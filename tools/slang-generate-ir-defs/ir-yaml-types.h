#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// IR Operation Flags
enum : uint32_t
{
    kIROpFlags_None = 0,
    kIROpFlag_Parent = 1 << 0,   ///< This op is a parent op
    kIROpFlag_UseOther = 1 << 1, ///< If set this op can use 'other bits' to store information
    kIROpFlag_Hoistable =
        1 << 2, ///< If set this op is a hoistable inst that needs to be deduplicated.
    kIROpFlag_Global =
        1 << 3, ///< If set this op should always be hoisted but should never be deduplicated.
};

using IROpFlags = uint32_t;

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
    IROpFlags flags = kIROpFlags_None;

    Type getType() const override { return INSTRUCTION; }
};

// Range definition
struct RangeEntry : Entry
{
    std::string name;
    std::string comment;
    std::vector<std::string> flag_strings;
    IROpFlags flags = kIROpFlags_None;
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
