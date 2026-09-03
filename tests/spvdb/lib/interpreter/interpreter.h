#pragma once
#include "../module/module.h"
#include "image_sampler.h"
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace spvdb
{

// How to initialize memory at session creation.
enum class InitPattern
{
    Zero,
    Pattern
};

struct SessionOptions
{
    InitPattern init_pattern = InitPattern::Zero;
    uint32_t init_value = 0xDEADBEEFu;
};

enum class StopReason
{
    Breakpoint,
    Step,
    EntryReached,
    EntryFinished,
    Panic,
};

// A breakpoint (by source location or result-id).
struct Breakpoint
{
    uint32_t id;
    bool active = true;
    // Source breakpoint:
    std::string file;
    uint32_t line = 0;
    // Instruction breakpoint:
    uint32_t result_id = 0;
};

// Program counter: identifies one instruction within a function's CFG.
struct PC
{
    uint32_t function_id = 0;
    uint32_t block_label = 0;
    uint32_t instr_index = 0;

    bool valid() const
    {
        return function_id != 0;
    }
};

// One frame on the call stack.
struct CallFrame
{
    PC return_pc;
    uint32_t return_value_id = 0; // result-id of the OpFunctionCall instruction
};

// Execution state for a single SPIR-V invocation.
class Interpreter
{
public:
    explicit Interpreter(std::shared_ptr<const SpvModule> mod, SessionOptions opts = {});

    // Select entry point and initialize memory. Must be called before step/run.
    Result<void> begin(const std::string &entry_name);

    // Execute one SPIR-V instruction.  May return Breakpoint if the new PC
    // matches an active breakpoint (result-id or source location).
    StopReason step_instruction();

    // Execute until a breakpoint, EntryFinished, or Panic.
    StopReason run_to_breakpoint();

    // Execute until the source line changes (step into).
    StopReason step_source_line();

    // Execute until the source line changes at the current or shallower call
    // depth (step over).
    StopReason step_over_source_line();

    // Execute until the current function returns.
    StopReason step_out();

    // Restart execution from the entry point (re-initializes memory, keeps inputs).
    Result<void> restart();

    // --- Inputs ---
    Result<void> set_descriptor(uint32_t set, uint32_t binding, const Value &value);
    // Bind a sampled-image or storage-image descriptor from an on-disk file.
    Result<void> set_image(uint32_t set, uint32_t binding, std::string_view path);
    // Bind a vertex/fragment Input variable by its Location decoration.
    Result<void> set_input_location(uint32_t location, Value value);
    Result<void> set_builtin(SpvBuiltIn builtin, Value value);
    Result<void> set_spec_constant(uint32_t spec_id, Value value);

    // --- Inspection ---
    const PC &current_pc() const
    {
        return pc_;
    }
    bool is_finished() const
    {
        return finished_;
    }
    bool is_panicked() const
    {
        return panicked_;
    }
    const std::string &panic_message() const
    {
        return panic_msg_;
    }

    // Source location of the instruction at the current PC.
    // Returns an invalid SourceLoc (line==0) when no debug info is available.
    SourceLoc current_source_location() const;

    // NonSemantic debug scope tracking.
    // Returns 0 when no DebugScope has been seen yet or after DebugNoScope.
    uint32_t current_debug_scope_id() const
    {
        return current_scope_id_;
    }
    uint32_t current_debug_inlined_at_id() const
    {
        return current_inlined_at_id_;
    }

    // All variables currently live in the id_map.
    std::vector<std::pair<std::string, Value>> local_variables() const;

    // Output variables (storage class Output / StorageBuffer).
    std::vector<std::pair<std::string, Value>> output_variables() const;

    // Read back a storage-buffer or output binding.
    Result<Value> read_descriptor(uint32_t set, uint32_t binding) const;

    // Call stack (most recent first).
    std::vector<CallFrame> call_stack() const
    {
        return call_stack_;
    }

    // --- Breakpoints ---
    uint32_t add_breakpoint(std::string file, uint32_t line);
    uint32_t add_breakpoint_at_id(uint32_t result_id);
    void remove_breakpoint(uint32_t bp_id);

    // Diagnostics emitted during execution (non-fatal warnings).
    std::vector<std::string> diagnostics;

private:
    std::shared_ptr<const SpvModule> module_;
    SessionOptions opts_;

    // The selected entry point function.
    uint32_t entry_function_id_ = 0;

    // Current PC and previous block label (for OpPhi).
    PC pc_;
    uint32_t prev_block_label_ = 0;

    // Per-instruction result-id → Value map (SSA values + loaded temporaries).
    std::unordered_map<uint32_t, Value> id_map_;

    // Variable memory store: variable result-id → current Value.
    // Covers all storage classes (Input, Output, Uniform, StorageBuffer,
    // Private, Function, Workgroup, etc.).
    std::unordered_map<uint32_t, Value> memory_;

    // Descriptor bindings: (set << 16 | binding) → variable result-id.
    std::unordered_map<uint32_t, uint32_t> descriptor_map_;

    // Staged descriptor values: (set << 16 | binding) → Value.
    // Persists across begin() calls so that descriptors set before execution
    // starts are not lost when init_memory() reinitializes variable storage.
    std::unordered_map<uint32_t, Value> descriptor_staging_;

    // Staged location-based input values: location → Value.
    // Used by set_input_location(); applied in begin() after init_memory().
    std::unordered_map<uint32_t, Value> location_staging_;

    // Built-in overrides: SpvBuiltIn → Value.
    std::unordered_map<uint32_t, Value> builtin_overrides_;

    // Specialization constant overrides: spec-id → Value.
    std::unordered_map<uint32_t, Value> spec_overrides_;

    // Staged image bindings: (set << 16 | binding) → ImageData.
    // Persists across begin() calls; images survive init_memory().
    std::unordered_map<uint32_t, ImageData> image_staging_;
    // var_id → staging_key; rebuilt in begin() from image_staging_ + module variables.
    std::unordered_map<uint32_t, uint32_t> image_var_map_;

    // SSA debug variable values from DebugValue instructions.
    // Maps DebugLocalVariable result-id → most recently assigned Value.
    std::unordered_map<uint32_t, Value> debug_var_values_;

    // Current NonSemantic DebugScope state.
    uint32_t current_scope_id_ = 0; // DebugFunction result-id
    uint32_t current_inlined_at_id_ = 0; // DebugInlinedAt result-id (0 = not inlined)

    // Call stack.
    std::vector<CallFrame> call_stack_;

    // Breakpoints.
    std::vector<Breakpoint> breakpoints_;
    uint32_t next_bp_id_ = 1;

    bool finished_ = false;
    bool panicked_ = false;
    std::string panic_msg_;
    std::string entry_name_;

    // ---- helpers -----------------------------------------------------------
    void panic(const std::string &msg);

    // Returns true if the current PC matches any active breakpoint.
    bool check_breakpoint_at_pc() const;

    // Get or produce the Value for a result-id (checks id_map_ then constants).
    const Value &lookup(uint32_t id) const;

    // Initialize memory for all global variables to init_pattern.
    void init_memory();

    // Evaluate all spec-constants (with overrides applied).
    void eval_spec_constants();

    // Resolve a pointer to the Value it points at (read).
    Result<Value> load_ptr(const Value &ptr) const;

    // Resolve a pointer and store a value through it.
    Result<void> store_ptr(const Value &ptr, Value val);

    // Index into a composite value (for access-chain resolution).
    static Result<Value *> index_into(Value &root, const std::vector<uint32_t> &chain);
    static Result<const Value *> index_into(const Value &root, const std::vector<uint32_t> &chain);

    // Execute a single instruction; returns true if execution should stop.
    StopReason execute_one(const Instruction &inst);

    // ---- opcode handlers ---------------------------------------------------
    StopReason exec_arith_logic(const Instruction &inst);
    StopReason exec_convert(const Instruction &inst);
    StopReason exec_memory(const Instruction &inst);
    StopReason exec_composite(const Instruction &inst);
    StopReason exec_control_flow(const Instruction &inst);
    StopReason exec_ext_inst(const Instruction &inst);
    StopReason exec_derivative(const Instruction &inst);
    StopReason exec_image(const Instruction &inst);

    // Move PC forward to the next instruction in the current block.
    // Returns false if the block is exhausted (should not happen mid-block).
    bool advance_pc();
};

} // namespace spvdb
