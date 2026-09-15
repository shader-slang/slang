#include "../module/module.h"
#include "../parser/parser.h"
#include "spvdb_session.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace spvdb
{

// Public Module handle (wraps the internal SpvModule).
struct Module
{
    std::shared_ptr<SpvModule> impl;
};

// ---- Module loading --------------------------------------------------------

Result<std::shared_ptr<Module>> load_module(std::span<const uint32_t> spirv_words)
{
    auto pr = parse_spirv(spirv_words);
    if (!pr)
        return Result<std::shared_ptr<Module>>::err(pr.error().message);
    auto mr = build_module(*pr);
    if (!mr)
        return Result<std::shared_ptr<Module>>::err(mr.error().message);
    auto pub = std::make_shared<Module>();
    pub->impl = *mr;
    return pub;
}

Result<std::shared_ptr<Module>> load_module_from_file(std::string_view path)
{
    std::ifstream f(std::string(path), std::ios::binary);
    if (!f)
        return Result<std::shared_ptr<Module>>::err("Cannot open file: " + std::string(path));
    f.seekg(0, std::ios::end);
    size_t sz = f.tellg();
    f.seekg(0, std::ios::beg);
    if (sz % 4 != 0)
        return Result<std::shared_ptr<Module>>::err("File size not a multiple of 4");
    std::vector<uint32_t> words(sz / 4);
    f.read(reinterpret_cast<char *>(words.data()), sz);
    return load_module(std::span<const uint32_t>(words));
}

// ---- Entry point enumeration -----------------------------------------------

static ExecutionModel to_exec_model(SpvExecutionModel m)
{
    switch (m) {
        case SpvExecutionModelGLCompute: return ExecutionModel::GLCompute;
        case SpvExecutionModelVertex: return ExecutionModel::Vertex;
        case SpvExecutionModelFragment: return ExecutionModel::Fragment;
        default: return ExecutionModel::Other;
    }
}

std::vector<EntryPointInfo> list_entry_points(const Module &mod)
{
    std::vector<EntryPointInfo> result;
    for (auto &ep : mod.impl->entry_points)
        result.push_back({ep.name, to_exec_model(ep.execution_model)});
    return result;
}

// ---- Session creation ------------------------------------------------------

Result<std::unique_ptr<Session>> create_session(std::shared_ptr<const Module> mod,
                                                std::string_view entry_point,
                                                SessionOptions opts)
{
    // Verify entry point exists.
    bool found = false;
    for (auto &ep : mod->impl->entry_points)
        if (ep.name == entry_point) {
            found = true;
            break;
        }
    if (!found)
        return Result<std::unique_ptr<Session>>::err("Entry point not found: "
                                                     + std::string(entry_point));
    // Share ownership of the inner module.
    auto inner = std::const_pointer_cast<::spvdb::SpvModule>(mod->impl);
    auto sess = std::make_unique<Session>(inner, std::string(entry_point), opts);
    return sess;
}

// ---- Specialization constants ----------------------------------------------

Result<void> set_spec_constant(Session &s, uint32_t spec_id, bool value)
{
    s.interp.set_spec_constant(spec_id, Value::make_bool(value));
    return Result<void> {};
}
Result<void> set_spec_constant(Session &s, uint32_t spec_id, int32_t value)
{
    s.interp.set_spec_constant(spec_id, Value::make_i32(value));
    return Result<void> {};
}
Result<void> set_spec_constant(Session &s, uint32_t spec_id, uint32_t value)
{
    s.interp.set_spec_constant(spec_id, Value::make_u32(value));
    return Result<void> {};
}
Result<void> set_spec_constant(Session &s, uint32_t spec_id, float value)
{
    s.interp.set_spec_constant(spec_id, Value::make_f32(value));
    return Result<void> {};
}

// ---- Shader inputs ---------------------------------------------------------

// Convert raw binary data to a Value given the type of a variable.
static Value bytes_to_value(const std::byte *data,
                            size_t size,
                            const SpvType *type,
                            const ::spvdb::SpvModule &m)
{
    if (!type || size == 0)
        return Value::make_u32(0);
    switch (type->kind) {
        case SpvType::Kind::Int:
            {
                auto *it = static_cast<const IntType *>(type);
                if (it->width <= 32) {
                    uint32_t v = 0;
                    std::memcpy(&v, data, std::min(size, sizeof(v)));
                    return it->is_signed ? Value::make_i32(static_cast<int32_t>(v))
                                         : Value::make_u32(v);
                }
                uint64_t v = 0;
                std::memcpy(&v, data, std::min(size, sizeof(v)));
                return it->is_signed ? Value::make_i64(static_cast<int64_t>(v))
                                     : Value::make_u64(v);
            }
        case SpvType::Kind::Float:
            {
                auto *ft = static_cast<const FloatType *>(type);
                if (ft->width <= 32) {
                    float v = 0;
                    std::memcpy(&v, data, std::min(size, sizeof(v)));
                    return Value::make_f32(v);
                }
                double v = 0;
                std::memcpy(&v, data, std::min(size, sizeof(v)));
                return Value::make_f64(v);
            }
        case SpvType::Kind::Vector:
            {
                auto *vt = static_cast<const VectorType *>(type);
                uint32_t elem_size = 4; // assume 32-bit
                if (vt->element_type && vt->element_type->kind == SpvType::Kind::Float) {
                    auto *ft = static_cast<const FloatType *>(vt->element_type.get());
                    elem_size = ft->width / 8;
                } else if (vt->element_type && vt->element_type->kind == SpvType::Kind::Int) {
                    auto *it = static_cast<const IntType *>(vt->element_type.get());
                    elem_size = it->width / 8;
                }
                std::vector<Value> elems;
                for (uint32_t i = 0; i < vt->count && (i + 1) * elem_size <= size; ++i)
                    elems.push_back(
                        bytes_to_value(data + i * elem_size, elem_size, vt->element_type.get(), m));
                return Value::make_composite(std::move(elems));
            }
        case SpvType::Kind::Struct:
            {
                auto *st = static_cast<const StructType *>(type);
                std::vector<Value> mems;
                std::vector<std::string> names;
                for (size_t i = 0; i < st->members.size(); ++i) {
                    uint32_t offset = st->members[i].offset;
                    size_t remaining = offset < size ? size - offset : 0;
                    mems.push_back(
                        bytes_to_value(data + offset, remaining, st->members[i].type.get(), m));
                    names.push_back(st->members[i].name);
                }
                Value v = Value::make_composite(std::move(mems));
                v.member_names = std::move(names);
                return v;
            }
        case SpvType::Kind::Array:
            {
                auto *at = static_cast<const ArrayType *>(type);
                uint32_t stride = at->array_stride;
                std::vector<Value> elems;
                for (uint32_t i = 0; i < at->length && (stride == 0 || (i + 1) * stride <= size);
                     ++i)
                {
                    size_t off = stride ? i * stride : 0;
                    elems.push_back(bytes_to_value(data + off,
                                                   stride ? stride : size,
                                                   at->element_type.get(),
                                                   m));
                }
                return Value::make_composite(std::move(elems));
            }
        default: return Value::make_u32(0);
    }
}

Result<void> set_descriptor(Session &s,
                            uint32_t set,
                            uint32_t binding,
                            std::span<const std::byte> data)
{
    // Find the variable's type to interpret the raw bytes.
    for (auto &[id, var] : s.module->variables) {
        auto &dset = s.module->decorations_of(id);
        if (dset.get(SpvDecorationDescriptorSet) == set
            && dset.get(SpvDecorationBinding) == binding)
        {
            auto *ptr_type = s.module->type_of(var.type_id);
            if (ptr_type && ptr_type->kind == SpvType::Kind::Pointer) {
                auto *pt = static_cast<const PointerType *>(ptr_type);
                Value val = bytes_to_value(data.data(), data.size(), pt->pointee.get(), *s.module);
                return s.interp.set_descriptor(set, binding, val);
            }
        }
    }
    return Result<void>::err("No descriptor variable at set=" + std::to_string(set)
                             + " binding=" + std::to_string(binding));
}

// JSON → Value conversion using the module's type info.
static Value json_to_value(const nlohmann::json &j, const SpvType *type);

static Value json_to_value(const nlohmann::json &j, const SpvType *type)
{
    if (!type)
        return Value::make_u32(0);
    switch (type->kind) {
        case SpvType::Kind::Bool: return Value::make_bool(j.get<bool>());
        case SpvType::Kind::Int:
            {
                auto *it = static_cast<const IntType *>(type);
                if (it->width <= 32)
                    return it->is_signed ? Value::make_i32(j.get<int32_t>())
                                         : Value::make_u32(j.get<uint32_t>());
                return it->is_signed ? Value::make_i64(j.get<int64_t>())
                                     : Value::make_u64(j.get<uint64_t>());
            }
        case SpvType::Kind::Float:
            {
                auto *ft = static_cast<const FloatType *>(type);
                return ft->width <= 32 ? Value::make_f32(j.get<float>())
                                       : Value::make_f64(j.get<double>());
            }
        case SpvType::Kind::Vector:
            {
                auto *vt = static_cast<const VectorType *>(type);
                std::vector<Value> elems;
                for (uint32_t i = 0; i < vt->count && i < j.size(); ++i)
                    elems.push_back(json_to_value(j[i], vt->element_type.get()));
                return Value::make_composite(std::move(elems));
            }
        case SpvType::Kind::Matrix:
            {
                auto *mt = static_cast<const MatrixType *>(type);
                std::vector<Value> cols;
                for (uint32_t i = 0; i < mt->columns && i < j.size(); ++i)
                    cols.push_back(json_to_value(j[i], mt->column_type.get()));
                return Value::make_composite(std::move(cols));
            }
        case SpvType::Kind::Struct:
            {
                auto *st = static_cast<const StructType *>(type);
                std::vector<Value> mems;
                for (size_t i = 0; i < st->members.size(); ++i) {
                    const nlohmann::json *field = nullptr;
                    // Try named access first.
                    if (j.is_object() && !st->members[i].name.empty()) {
                        auto it = j.find(st->members[i].name);
                        if (it != j.end())
                            field = &(*it);
                    }
                    // Fallback: index access.
                    if (!field && j.is_array() && i < j.size())
                        field = &j[i];
                    if (field)
                        mems.push_back(json_to_value(*field, st->members[i].type.get()));
                    else
                        mems.push_back(Value::make_u32(0));
                }
                return Value::make_composite(std::move(mems));
            }
        case SpvType::Kind::Array:
            {
                auto *at = static_cast<const ArrayType *>(type);
                std::vector<Value> elems;
                for (uint32_t i = 0; i < at->length && i < j.size(); ++i)
                    elems.push_back(json_to_value(j[i], at->element_type.get()));
                return Value::make_composite(std::move(elems));
            }
        case SpvType::Kind::RuntimeArray:
            {
                auto *rat = static_cast<const RuntimeArrayType *>(type);
                std::vector<Value> elems;
                if (j.is_array()) {
                    for (size_t i = 0; i < j.size(); ++i)
                        elems.push_back(json_to_value(j[i], rat->element_type.get()));
                } else {
                    elems.push_back(json_to_value(j, rat->element_type.get()));
                }
                return Value::make_composite(std::move(elems));
            }
        default: return Value::make_u32(0);
    }
}

Result<void> set_descriptor_json(Session &s,
                                 uint32_t set,
                                 uint32_t binding,
                                 std::string_view json_str)
{
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(json_str);
    } catch (std::exception &e) {
        return Result<void>::err("JSON parse error: " + std::string(e.what()));
    }
    for (auto &[id, var] : s.module->variables) {
        // Only StorageBuffer/Uniform/UniformConstant variables carry descriptor
        // bindings.  Function-scope variables (e.g. debug temporaries emitted
        // by -g3) have no DescriptorSet/Binding decorations, so their
        // decorations_of().get() returns 0 for both — which would cause them
        // to incorrectly match (set=0, binding=0).
        if (var.storage_class != SpvStorageClassStorageBuffer
            && var.storage_class != SpvStorageClassUniform
            && var.storage_class != SpvStorageClassUniformConstant)
            continue;
        auto &dset = s.module->decorations_of(id);
        if (dset.get(SpvDecorationDescriptorSet) == set
            && dset.get(SpvDecorationBinding) == binding)
        {
            auto *ptr_type = s.module->type_of(var.type_id);
            if (ptr_type && ptr_type->kind == SpvType::Kind::Pointer) {
                auto *pt = static_cast<const PointerType *>(ptr_type);
                const SpvType *effective_type = pt->pointee.get();
                bool wrap_in_struct = false;

                // Transparently unwrap single-member Block structs (Vulkan SSBO pattern).
                // Lets callers pass [1, 2, 3] instead of [[1, 2, 3]] for a
                // buffer whose type is Block { RuntimeArray<T> }.
                if (effective_type && effective_type->kind == SpvType::Kind::Struct && j.is_array())
                {
                    auto *st = static_cast<const StructType *>(effective_type);
                    if (st->members.size() == 1 && st->members[0].type
                        && (st->members[0].type->kind == SpvType::Kind::RuntimeArray
                            || st->members[0].type->kind == SpvType::Kind::Array))
                    {
                        effective_type = st->members[0].type.get();
                        wrap_in_struct = true;
                    }
                }

                Value val;
                try {
                    val = json_to_value(j, effective_type);
                } catch (std::exception &e) {
                    return Result<void>::err(std::string("JSON → SPIR-V value conversion failed: ")
                                             + e.what());
                }
                if (wrap_in_struct)
                    val = Value::make_composite({std::move(val)});
                return s.interp.set_descriptor(set, binding, val);
            }
        }
    }
    return Result<void>::err("No descriptor variable at set=" + std::to_string(set)
                             + " binding=" + std::to_string(binding));
}

Result<void> set_image(Session &s, uint32_t set, uint32_t binding, std::string_view image_path)
{
    return s.interp.set_image(set, binding, image_path);
}

Result<void> set_input_location(Session &s, uint32_t location, Value value)
{
    return s.interp.set_input_location(location, std::move(value));
}

Result<void> set_input_location_json(Session &s, uint32_t location, std::string_view json_str)
{
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(json_str);
    } catch (std::exception &e) {
        return Result<void>::err("JSON parse error: " + std::string(e.what()));
    }
    // Find the Input variable at this location to get its type.
    for (auto &[id, var] : s.module->variables) {
        if (var.storage_class != SpvStorageClassInput)
            continue;
        auto &dset = s.module->decorations_of(id);
        if (dset.has(SpvDecorationBuiltIn))
            continue;
        if (dset.get(SpvDecorationLocation) == location) {
            auto *ptr_type = s.module->type_of(var.type_id);
            if (ptr_type && ptr_type->kind == SpvType::Kind::Pointer) {
                auto *pt = static_cast<const PointerType *>(ptr_type);
                Value val = json_to_value(j, pt->pointee.get());
                return s.interp.set_input_location(location, std::move(val));
            }
        }
    }
    return Result<void>::err("No Input variable at location=" + std::to_string(location));
}

Result<void> set_builtin(Session &s, uint32_t builtin_id, Value value)
{
    return s.interp.set_builtin(static_cast<SpvBuiltIn>(builtin_id), std::move(value));
}

// ---- Breakpoints -----------------------------------------------------------

Result<BreakpointId> set_breakpoint(Session &s, std::string_view file, uint32_t line)
{
    uint32_t id = s.interp.add_breakpoint(std::string(file), line);
    return BreakpointId {id};
}

Result<BreakpointId> set_breakpoint_at_id(Session &s, uint32_t result_id)
{
    uint32_t id = s.interp.add_breakpoint_at_id(result_id);
    return BreakpointId {id};
}

void remove_breakpoint(Session &s, BreakpointId bp)
{
    s.interp.remove_breakpoint(bp.id);
}

// ---- Execution control -----------------------------------------------------

static StopReason to_public(::spvdb::StopReason r)
{
    switch (r) {
        case ::spvdb::StopReason::Breakpoint: return StopReason::Breakpoint;
        case ::spvdb::StopReason::Step: return StopReason::Step;
        case ::spvdb::StopReason::EntryReached: return StopReason::EntryReached;
        case ::spvdb::StopReason::EntryFinished: return StopReason::EntryFinished;
        case ::spvdb::StopReason::Panic: return StopReason::Panic;
    }
    return StopReason::Panic;
}

template<typename Fn> static StopReason ensure_and_run(Session &s, Fn fn)
{
    auto r = s.ensure_started();
    if (!r)
        return StopReason::Panic;
    return to_public(fn());
}

Result<void> restart(Session &s)
{
    s.started = false;
    return s.ensure_started();
}
bool is_finished(const Session &s)
{
    return s.interp.is_finished();
}
bool is_panicked(const Session &s)
{
    return s.interp.is_panicked();
}

StopReason run(Session &s)
{
    return ensure_and_run(s, [&] { return s.interp.run_to_breakpoint(); });
}
StopReason step_instruction(Session &s)
{
    return ensure_and_run(s, [&] { return s.interp.step_instruction(); });
}
StopReason step_over(Session &s)
{
    return ensure_and_run(s, [&] { return s.interp.step_over_source_line(); });
}
StopReason step(Session &s)
{
    return ensure_and_run(s, [&] { return s.interp.step_source_line(); });
}
StopReason step_out(Session &s)
{
    return ensure_and_run(s, [&] { return s.interp.step_out(); });
}

std::string panic_message(const Session &s)
{
    return s.interp.panic_message();
}

// ---- Inspection ------------------------------------------------------------

SourceLocation current_location(const Session &s)
{
    auto loc = s.interp.current_source_location();
    return {loc.file, loc.line, loc.column};
}

std::vector<LocalVar> local_variables(const Session &s)
{
    std::vector<LocalVar> result;
    for (auto &[name, val] : s.interp.local_variables()) result.push_back({name, val});
    return result;
}

Result<LocalVar> evaluate_variable(const Session &s, std::string_view name)
{
    for (auto &[n, v] : s.interp.local_variables())
        if (n == name)
            return LocalVar {n, v};
    return Result<LocalVar>::err("Variable not found: " + std::string(name));
}

std::vector<LocalVar> output_variables(const Session &s)
{
    std::vector<LocalVar> result;
    for (auto &[name, val] : s.interp.output_variables()) result.push_back({name, val});
    return result;
}

Result<std::vector<std::byte>> read_descriptor(const Session &s, uint32_t set, uint32_t binding)
{
    auto r = s.interp.read_descriptor(set, binding);
    if (!r)
        return Result<std::vector<std::byte>>::err(r.error().message);
    // Serialize Value back to bytes. For now return the raw u32/float bytes.
    // A full implementation would walk the type tree with layout info.
    std::vector<std::byte> bytes;
    std::function<void(const Value &)> ser = [&](const Value &v)
    {
        switch (v.kind) {
            case Value::Kind::UInt32:
            case Value::Kind::Int32:
                {
                    const auto *p = reinterpret_cast<const std::byte *>(&v.scalar.u32);
                    bytes.insert(bytes.end(), p, p + 4);
                    break;
                }
            case Value::Kind::Float32:
                {
                    const auto *p = reinterpret_cast<const std::byte *>(&v.scalar.f32);
                    bytes.insert(bytes.end(), p, p + 4);
                    break;
                }
            case Value::Kind::Int64:
            case Value::Kind::UInt64:
                {
                    const auto *p = reinterpret_cast<const std::byte *>(&v.scalar.u64);
                    bytes.insert(bytes.end(), p, p + 8);
                    break;
                }
            case Value::Kind::Float64:
                {
                    const auto *p = reinterpret_cast<const std::byte *>(&v.scalar.f64);
                    bytes.insert(bytes.end(), p, p + 8);
                    break;
                }
            case Value::Kind::Bool:
                {
                    uint32_t b = v.scalar.b ? 1 : 0;
                    const auto *p = reinterpret_cast<const std::byte *>(&b);
                    bytes.insert(bytes.end(), p, p + 4);
                    break;
                }
            case Value::Kind::Composite:
                for (auto &e : v.elements) ser(e);
                break;
            default: break;
        }
    };
    ser(*r);
    return bytes;
}

static SourceLocation loc_from_pc(const ::spvdb::SpvModule &m, const PC &pc)
{
    auto fit = m.functions.find(pc.function_id);
    if (fit == m.functions.end())
        return {};
    auto bit = fit->second.block_index.find(pc.block_label);
    if (bit == fit->second.block_index.end())
        return {};
    const BasicBlock &bb = fit->second.blocks[bit->second];
    if (pc.instr_index < bb.source_locs.size()) {
        auto &sl = bb.source_locs[pc.instr_index];
        return {sl.file, sl.line, sl.column};
    }
    return {};
}

std::vector<Frame> backtrace(const Session &s)
{
    std::vector<Frame> result;

    uint32_t scope_id = s.interp.current_debug_scope_id();
    uint32_t inlined_at_id = s.interp.current_debug_inlined_at_id();

    // Innermost frame: current execution position.
    {
        Frame f;
        // Prefer the NonSemantic debug function name over the SPIR-V function name
        // (e.g. when we're inside an inlined function the SPIR-V function is the
        // caller, but the debug scope names the callee).
        if (scope_id != 0) {
            auto dit = s.module->debug_functions.find(scope_id);
            if (dit != s.module->debug_functions.end() && !dit->second.name.empty())
                f.function_name = dit->second.name;
        }
        if (f.function_name.empty()) {
            const PC &pc = s.interp.current_pc();
            auto fit = s.module->functions.find(pc.function_id);
            if (fit != s.module->functions.end())
                f.function_name = fit->second.name.empty() ? "%" + std::to_string(pc.function_id)
                                                           : fit->second.name;
        }
        auto loc = s.interp.current_source_location();
        f.loc = {loc.file, loc.line, loc.column};
        result.push_back(std::move(f));
    }

    // Walk the DebugInlinedAt chain to reconstruct virtual inlined call frames.
    while (inlined_at_id != 0) {
        auto ait = s.module->debug_inlined_at.find(inlined_at_id);
        if (ait == s.module->debug_inlined_at.end())
            break;
        const auto &ia = ait->second;

        Frame caller;
        if (ia.scope_id != 0) {
            auto dit = s.module->debug_functions.find(ia.scope_id);
            if (dit != s.module->debug_functions.end() && !dit->second.name.empty())
                caller.function_name = dit->second.name;
            // Source file from the scope's DebugFunction → DebugSource.
            auto src_it = s.module->debug_sources.find(dit->second.source_id);
            if (src_it != s.module->debug_sources.end())
                caller.loc.file = src_it->second;
        }
        caller.loc.line = ia.line;
        result.push_back(std::move(caller));

        inlined_at_id = ia.inlined_at_id;
    }

    // Outer frames: actual (non-inlined) callers from the call stack.
    auto stack = s.interp.call_stack();
    for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
        Frame f;
        auto fit = s.module->functions.find(it->return_pc.function_id);
        if (fit != s.module->functions.end())
            f.function_name = fit->second.name.empty()
                                ? "%" + std::to_string(it->return_pc.function_id)
                                : fit->second.name;
        f.loc = loc_from_pc(*s.module, it->return_pc);
        result.push_back(std::move(f));
    }
    return result;
}

} // namespace spvdb
