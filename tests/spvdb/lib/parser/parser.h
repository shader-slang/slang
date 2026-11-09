#pragma once
#include "../core/result.h"
#include <cstdint>
#include <span>
#include <vector>

// Include SPIRV-Headers for opcode/enum definitions.
#include <spirv/unified1/spirv.h>

namespace spvdb
{

// A single decoded SPIR-V instruction.
// `words` contains ALL operand words (i.e. everything after the opcode word),
// so indices into `words` are the same as the SPIR-V spec's "word N" references
// minus 1 (the opcode word).
//
// `type_id` and `result_id` are pre-extracted for convenience; they are also
// present at their natural positions in `words` when the opcode carries them.
struct Instruction
{
    SpvOp opcode = SpvOpNop;
    uint32_t type_id = 0; // 0 if the opcode has no type operand
    uint32_t result_id = 0; // 0 if the opcode produces no result

    // All operand words (after the opcode word).
    // For opcodes that have <type> <result> as their first two operands,
    // words[0] == type_id and words[1] == result_id.
    std::vector<uint32_t> words;

    // Convenience: operand words AFTER type_id and result_id have been consumed.
    // Returns words[n] where n accounts for the presence of type+result.
    uint32_t operand(uint32_t idx) const;
    uint32_t operand_count() const;
};

// The result of parsing a SPIR-V binary: a flat ordered list of instructions.
struct ParsedModule
{
    // SPIR-V header fields.
    uint32_t version = 0;
    uint32_t generator = 0;
    uint32_t id_bound = 0;

    // All instructions in source order (excluding the header words).
    std::vector<Instruction> instructions;
};

// Parse a SPIR-V binary given as a span of 32-bit words.
// Handles both native-endian and byte-swapped (big-endian) modules.
// Returns an error if the magic number is absent or the word-count fields
// are inconsistent.
Result<ParsedModule> parse_spirv(std::span<const uint32_t> words);

} // namespace spvdb
