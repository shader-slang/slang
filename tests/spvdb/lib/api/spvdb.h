#pragma once
// spvdb public C++ API.
// Link by source: add_subdirectory(path/to/spvdb) and link against the `spvdb` CMake target.

// Include the interpreter header to get shared types (InitPattern, SessionOptions,
// StopReason) rather than redefining them here.
#include "../interpreter/interpreter.h"

#include "../core/result.h"
#include "../core/value.h"
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace spvdb
{

// ---- Module loading --------------------------------------------------------

struct Module; // Opaque public handle; wraps SpvModule.

Result<std::shared_ptr<Module>> load_module(std::span<const uint32_t> spirv_words);
Result<std::shared_ptr<Module>> load_module_from_file(std::string_view path);

// ---- Entry point enumeration -----------------------------------------------

enum class ExecutionModel
{
    GLCompute,
    Vertex,
    Fragment,
    Other
};

struct EntryPointInfo
{
    std::string name;
    ExecutionModel execution_model;
};

std::vector<EntryPointInfo> list_entry_points(const Module &);

// ---- Session ---------------------------------------------------------------

// InitPattern, SessionOptions, StopReason are defined in interpreter.h.

struct Session; // Move-only, single-threaded.

Result<std::unique_ptr<Session>> create_session(std::shared_ptr<const Module>,
                                                std::string_view entry_point,
                                                SessionOptions opts = {});

// ---- Specialization constants ----------------------------------------------

Result<void> set_spec_constant(Session &, uint32_t spec_id, bool value);
Result<void> set_spec_constant(Session &, uint32_t spec_id, int32_t value);
Result<void> set_spec_constant(Session &, uint32_t spec_id, uint32_t value);
Result<void> set_spec_constant(Session &, uint32_t spec_id, float value);

// ---- Shader inputs ---------------------------------------------------------

Result<void> set_descriptor(Session &,
                            uint32_t set,
                            uint32_t binding,
                            std::span<const std::byte> data);
Result<void> set_descriptor_json(Session &, uint32_t set, uint32_t binding, std::string_view json);
// Bind a sampled-image or storage-image descriptor from an on-disk file.
// Supports PNG, BMP, JPG, TGA, HDR via stb_image; loaded as RGBA float.
Result<void> set_image(Session &, uint32_t set, uint32_t binding, std::string_view image_path);
// Bind a vertex/fragment Input variable by its Location decoration.
// Equivalent to reading a per-vertex attribute or per-fragment interpolant.
Result<void> set_input_location(Session &, uint32_t location, Value value);
Result<void> set_input_location_json(Session &, uint32_t location, std::string_view json);
Result<void> set_builtin(Session &, uint32_t builtin_id, Value value);

// ---- Breakpoints -----------------------------------------------------------

struct BreakpointId
{
    uint32_t id;
};

Result<BreakpointId> set_breakpoint(Session &, std::string_view file, uint32_t line);
Result<BreakpointId> set_breakpoint_at_id(Session &, uint32_t result_id);
void remove_breakpoint(Session &, BreakpointId);

// ---- Execution control (StopReason from interpreter.h) --------------------

// Restart resets execution to the entry point, re-applying all staged
// descriptor/builtin/spec-constant bindings and preserving breakpoints.
// Prefer this over creating a new Session when you want to re-run with the
// same inputs.
Result<void> restart(Session &);

bool is_finished(const Session &);
bool is_panicked(const Session &);

StopReason run(Session &);
StopReason step(Session &);
StopReason step_instruction(Session &);
StopReason step_over(Session &);
StopReason step_out(Session &);

std::string panic_message(const Session &);

// ---- Inspection ------------------------------------------------------------

struct SourceLocation
{
    std::string file;
    uint32_t line = 0, col = 0;
};
SourceLocation current_location(const Session &);

struct LocalVar
{
    std::string name;
    Value value;
};
std::vector<LocalVar> local_variables(const Session &);
Result<LocalVar> evaluate_variable(const Session &, std::string_view name);

std::vector<LocalVar> output_variables(const Session &);

Result<std::vector<std::byte>> read_descriptor(const Session &, uint32_t set, uint32_t binding);

struct Frame
{
    std::string function_name;
    SourceLocation loc;
};
std::vector<Frame> backtrace(const Session &);

} // namespace spvdb
