#pragma once
#include "../core/result.h"
#include "../core/value.h"
#include "../parser/parser.h"
#include "spv_types.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace spvdb
{

// A resolved source file/line/column, produced from OpLine or NonSemantic
// debug info.  An empty file or line==0 means "no location known."
struct SourceLoc
{
    std::string file;
    uint32_t line = 0;
    uint32_t column = 0;
    bool valid() const
    {
        return !file.empty() && line != 0;
    }
};

// A SPIR-V decoration and its literal operands.
struct Decoration
{
    SpvDecoration kind;
    std::vector<uint32_t> operands;
};

// Decoration set attached to a single result-id or struct member.
struct DecorationSet
{
    std::vector<Decoration> decorations;

    bool has(SpvDecoration d) const
    {
        for (auto &x : decorations)
            if (x.kind == d)
                return true;
        return false;
    }
    uint32_t get(SpvDecoration d, uint32_t default_val = 0) const
    {
        for (auto &x : decorations)
            if (x.kind == d && !x.operands.empty())
                return x.operands[0];
        return default_val;
    }
};

// A SPIR-V variable (OpVariable).
struct Variable
{
    uint32_t result_id = 0;
    uint32_t type_id = 0; // pointer type id
    uint32_t storage_class = 0;
    std::optional<uint32_t> initializer_id; // optional constant initializer
    std::string name;
    DecorationSet decorations;
};

// A single SPIR-V basic block.
struct BasicBlock
{
    uint32_t label_id = 0;
    std::vector<Instruction> instructions; // includes the terminator
    // Parallel to instructions: resolved source location for each instruction.
    // Populated by build_module() from OpLine / NonSemantic DebugLine.
    std::vector<SourceLoc> source_locs;
};

// A SPIR-V function.
struct Function
{
    uint32_t result_id = 0;
    uint32_t return_type_id = 0;
    uint32_t function_type_id = 0;

    // Parameters (result_id, type_id) in declaration order.
    std::vector<std::pair<uint32_t, uint32_t>> parameters;

    // Blocks in source order; block[0] is the entry block.
    std::vector<BasicBlock> blocks;

    // Fast lookup from label_id to block index.
    std::unordered_map<uint32_t, size_t> block_index;

    std::string name;

    const BasicBlock *find_block(uint32_t label_id) const
    {
        auto it = block_index.find(label_id);
        if (it == block_index.end())
            return nullptr;
        return &blocks[it->second];
    }
};

// An entry point declared by OpEntryPoint.
struct EntryPoint
{
    std::string name;
    SpvExecutionModel execution_model;
    uint32_t function_id;
    // Interface variable ids listed in the OpEntryPoint.
    std::vector<uint32_t> interface_ids;
};

// A spec-constant (OpSpecConstant / OpSpecConstantTrue / OpSpecConstantFalse /
// OpSpecConstantComposite / OpSpecConstantOp).
enum class SpecConstantKind
{
    Scalar,
    Composite,
    Op
};
struct SpecConstant
{
    SpecConstantKind kind;
    uint32_t result_id;
    uint32_t type_id;
    // For Scalar: the literal value (up to 2 words).
    std::vector<uint32_t> scalar_words;
    // For Composite: constituent result-ids.
    std::vector<uint32_t> constituent_ids;
    // For Op: the embedded opcode and its operand ids.
    SpvOp op_opcode = SpvOpNop;
    std::vector<uint32_t> op_operands;
};

// The decoded, immutable representation of a SPIR-V module.
struct SpvModule
{
    // Header info.
    uint32_t spv_version = 0;
    uint32_t spv_generator = 0;
    uint32_t id_bound = 0;

    // Entry points in declaration order.
    std::vector<EntryPoint> entry_points;

    // Type table: result-id → SpvType.
    std::unordered_map<uint32_t, std::shared_ptr<SpvType>> types;

    // Constant table: result-id → Value (evaluated, including typed booleans).
    std::unordered_map<uint32_t, Value> constants;

    // Spec-constant table (unevaluated): result-id → SpecConstant.
    std::unordered_map<uint32_t, SpecConstant> spec_constants;

    // Global variable table: result-id → Variable.
    std::unordered_map<uint32_t, Variable> variables;

    // Function table: result-id → Function.
    std::unordered_map<uint32_t, Function> functions;
    // Functions in source order (for determinism).
    std::vector<uint32_t> function_order;

    // Decoration table: result-id → DecorationSet.
    std::unordered_map<uint32_t, DecorationSet> decorations;

    // Member decoration table: (struct type result-id, member index) → DecorationSet.
    // Key = (type_id << 32) | member_index — works for id_bound < 2^32.
    std::unordered_map<uint64_t, DecorationSet> member_decorations;

    // Name table: result-id → name string.
    std::unordered_map<uint32_t, std::string> names;

    // Member name table: (type_id, member_index) → name.
    std::unordered_map<uint64_t, std::string> member_names;

    // Extension import table: result-id → extension name string.
    // e.g. id 1 → "GLSL.std.450"
    std::unordered_map<uint32_t, std::string> ext_imports;

    // OpString table: result-id → string value.
    std::unordered_map<uint32_t, std::string> strings;

    // NonSemantic.Shader.DebugInfo.100 DebugSource result-id → filename.
    // Populated from global DebugSource instructions during build_module().
    std::unordered_map<uint32_t, std::string> debug_sources;

    // NonSemantic.Shader.DebugInfo.100 DebugLocalVariable result-id → name.
    // Populated from global DebugLocalVariable instructions during build_module().
    std::unordered_map<uint32_t, std::string> debug_local_vars;

    // NonSemantic.Shader.DebugInfo.100 DebugFunction result-id → {name, source_id}.
    struct DebugFunctionInfo
    {
        std::string name;
        uint32_t source_id = 0; // DebugSource result-id for the source file
    };
    std::unordered_map<uint32_t, DebugFunctionInfo> debug_functions;

    // NonSemantic.Shader.DebugInfo.100 DebugInlinedAt result-id → call-site info.
    struct DebugInlinedAtInfo
    {
        uint32_t line = 0; // line number at the call site
        uint32_t scope_id = 0; // DebugFunction result-id of the containing scope
        uint32_t inlined_at_id = 0; // parent DebugInlinedAt for nested inlining (0 if none)
    };
    std::unordered_map<uint32_t, DebugInlinedAtInfo> debug_inlined_at;

    // Capabilities declared by the module.
    std::vector<SpvCapability> capabilities;

    // Helper: look up a type by result-id.
    const SpvType *type_of(uint32_t id) const
    {
        auto it = types.find(id);
        return it == types.end() ? nullptr : it->second.get();
    }

    const DecorationSet &decorations_of(uint32_t id) const
    {
        static const DecorationSet empty;
        auto it = decorations.find(id);
        return it == decorations.end() ? empty : it->second;
    }

    const DecorationSet &member_decorations_of(uint32_t type_id, uint32_t member) const
    {
        static const DecorationSet empty;
        uint64_t key = (static_cast<uint64_t>(type_id) << 32) | member;
        auto it = member_decorations.find(key);
        return it == member_decorations.end() ? empty : it->second;
    }

    const std::string &name_of(uint32_t id) const
    {
        static const std::string empty;
        auto it = names.find(id);
        return it == names.end() ? empty : it->second;
    }

    const std::string &member_name_of(uint32_t type_id, uint32_t member) const
    {
        static const std::string empty;
        uint64_t key = (static_cast<uint64_t>(type_id) << 32) | member;
        auto it = member_names.find(key);
        return it == member_names.end() ? empty : it->second;
    }
};

// Build a Module from a ParsedModule.
Result<std::shared_ptr<SpvModule>> build_module(const ParsedModule &parsed);

} // namespace spvdb
