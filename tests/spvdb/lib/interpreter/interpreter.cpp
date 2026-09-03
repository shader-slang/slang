#include "interpreter.h"
#include "image_sampler.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <sstream>

namespace spvdb
{

// Forward declaration of GLSL.std.450 dispatch.
Value dispatch_glsl_std_450(uint32_t inst_id,
                            const std::vector<Value> &args,
                            std::vector<std::string> &diagnostics);

// ---- Value helpers ---------------------------------------------------------

static Value zero_for_type(const SpvType *t, const SpvModule &m)
{
    if (!t)
        return Value::make_u32(0);
    switch (t->kind) {
        case SpvType::Kind::Bool: return Value::make_bool(false);
        case SpvType::Kind::Int:
            {
                auto *it = static_cast<const IntType *>(t);
                return Value::zero_int(it->width, it->is_signed);
            }
        case SpvType::Kind::Float:
            {
                auto *ft = static_cast<const FloatType *>(t);
                return Value::zero_float(ft->width);
            }
        case SpvType::Kind::Vector:
            {
                auto *vt = static_cast<const VectorType *>(t);
                std::vector<Value> elems(vt->count, zero_for_type(vt->element_type.get(), m));
                return Value::make_composite(std::move(elems));
            }
        case SpvType::Kind::Matrix:
            {
                auto *mt = static_cast<const MatrixType *>(t);
                std::vector<Value> cols(mt->columns, zero_for_type(mt->column_type.get(), m));
                return Value::make_composite(std::move(cols));
            }
        case SpvType::Kind::Array:
            {
                auto *at = static_cast<const ArrayType *>(t);
                std::vector<Value> elems(at->length, zero_for_type(at->element_type.get(), m));
                return Value::make_composite(std::move(elems));
            }
        case SpvType::Kind::Struct:
            {
                auto *st = static_cast<const StructType *>(t);
                std::vector<Value> mems;
                for (auto &mem : st->members) mems.push_back(zero_for_type(mem.type.get(), m));
                return Value::make_composite(std::move(mems));
            }
        default: return Value::make_u32(0);
    }
}

static Value pattern_for_type(const SpvType *t, const SpvModule &m, uint32_t pattern)
{
    if (!t)
        return Value::make_u32(pattern);
    switch (t->kind) {
        case SpvType::Kind::Bool: return Value::make_bool(pattern != 0);
        case SpvType::Kind::Int:
            {
                auto *it = static_cast<const IntType *>(t);
                if (it->width <= 32)
                    return it->is_signed ? Value::make_i32(static_cast<int32_t>(pattern))
                                         : Value::make_u32(pattern);
                uint64_t p64 = (static_cast<uint64_t>(pattern) << 32) | pattern;
                return it->is_signed ? Value::make_i64(static_cast<int64_t>(p64))
                                     : Value::make_u64(p64);
            }
        case SpvType::Kind::Float:
            {
                auto *ft = static_cast<const FloatType *>(t);
                if (ft->width <= 32) {
                    float f;
                    std::memcpy(&f, &pattern, 4);
                    return Value::make_f32(f);
                }
                uint64_t p64 = (static_cast<uint64_t>(pattern) << 32) | pattern;
                double d;
                std::memcpy(&d, &p64, 8);
                return Value::make_f64(d);
            }
        case SpvType::Kind::Vector:
            {
                auto *vt = static_cast<const VectorType *>(t);
                std::vector<Value> elems(vt->count,
                                         pattern_for_type(vt->element_type.get(), m, pattern));
                return Value::make_composite(std::move(elems));
            }
        case SpvType::Kind::Matrix:
            {
                auto *mt = static_cast<const MatrixType *>(t);
                std::vector<Value> cols(mt->columns,
                                        pattern_for_type(mt->column_type.get(), m, pattern));
                return Value::make_composite(std::move(cols));
            }
        case SpvType::Kind::Array:
            {
                auto *at = static_cast<const ArrayType *>(t);
                std::vector<Value> elems(at->length,
                                         pattern_for_type(at->element_type.get(), m, pattern));
                return Value::make_composite(std::move(elems));
            }
        case SpvType::Kind::Struct:
            {
                auto *st = static_cast<const StructType *>(t);
                std::vector<Value> mems;
                for (auto &mem : st->members)
                    mems.push_back(pattern_for_type(mem.type.get(), m, pattern));
                return Value::make_composite(std::move(mems));
            }
        default: return Value::make_u32(pattern);
    }
}

// ---- Interpreter -----------------------------------------------------------

Interpreter::Interpreter(std::shared_ptr<const SpvModule> mod, SessionOptions opts)
    : module_(std::move(mod))
    , opts_(opts)
{
}

void Interpreter::panic(const std::string &msg)
{
    panicked_ = true;
    panic_msg_ = msg;
}

const Value &Interpreter::lookup(uint32_t id) const
{
    auto it = id_map_.find(id);
    if (it != id_map_.end())
        return it->second;
    auto jt = module_->constants.find(id);
    if (jt != module_->constants.end())
        return jt->second;
    static const Value undef = Value::make_u32(0);
    return undef;
}

void Interpreter::init_memory()
{
    memory_.clear();
    for (auto &[id, var] : module_->variables) {
        auto *ptr_type = module_->type_of(var.type_id);
        if (!ptr_type || ptr_type->kind != SpvType::Kind::Pointer)
            continue;
        auto *pt = static_cast<const PointerType *>(ptr_type);
        const SpvType *pointee = pt->pointee.get();
        Value v;
        // Image, Sampler, and SampledImage types are opaque handles.
        // Store a self-referential uint32 as the handle so OpLoad returns it.
        if (pointee
            && (pointee->kind == SpvType::Kind::Image || pointee->kind == SpvType::Kind::Sampler
                || pointee->kind == SpvType::Kind::SampledImage))
        {
            v = Value::make_u32(id);
        } else if (opts_.init_pattern == InitPattern::Zero) {
            v = zero_for_type(pointee, *module_);
        } else {
            v = pattern_for_type(pointee, *module_, opts_.init_value);
        }
        memory_[id] = std::move(v);
        // Also expose a pointer-to-variable in id_map_ so that OpAccessChain
        // and OpLoad/OpStore can look up the variable by its result id.
        id_map_[id] = Value::make_pointer(pt->storage_class, id, {});
    }
}

void Interpreter::eval_spec_constants()
{
    // Two-pass: evaluate scalars first (including overrides), then composites+ops.
    for (auto &[id, sc] : module_->spec_constants) {
        if (sc.kind != SpecConstantKind::Scalar)
            continue;
        // Check for override.
        auto oit = spec_overrides_.find(id);
        if (oit != spec_overrides_.end()) {
            id_map_[id] = oit->second;
            continue;
        }
        // Also check for SpecId decoration.
        auto &dset = module_->decorations_of(id);
        if (dset.has(SpvDecorationSpecId)) {
            uint32_t spec_id = dset.get(SpvDecorationSpecId);
            auto sit = spec_overrides_.find(spec_id);
            if (sit != spec_overrides_.end()) {
                id_map_[id] = sit->second;
                continue;
            }
        }
        // Use default value from scalar_words (not module_->constants, which
        // stores a zero placeholder rather than the actual default).
        auto *t = module_->type_of(sc.type_id);
        if (t && t->kind == SpvType::Kind::Bool) {
            id_map_[id] = Value::make_bool(!sc.scalar_words.empty() && sc.scalar_words[0] != 0);
        } else if (t && t->kind == SpvType::Kind::Int) {
            auto *it = static_cast<const IntType *>(t);
            if (it->width <= 32) {
                uint32_t raw = sc.scalar_words.empty() ? 0 : sc.scalar_words[0];
                id_map_[id] = it->is_signed ? Value::make_i32(static_cast<int32_t>(raw))
                                            : Value::make_u32(raw);
            } else {
                uint32_t lo = sc.scalar_words.size() > 0 ? sc.scalar_words[0] : 0;
                uint32_t hi = sc.scalar_words.size() > 1 ? sc.scalar_words[1] : 0;
                uint64_t raw = (static_cast<uint64_t>(hi) << 32) | lo;
                id_map_[id] = it->is_signed ? Value::make_i64(static_cast<int64_t>(raw))
                                            : Value::make_u64(raw);
            }
        } else if (t && t->kind == SpvType::Kind::Float) {
            auto *ft = static_cast<const FloatType *>(t);
            if (ft->width <= 32) {
                uint32_t raw = sc.scalar_words.empty() ? 0 : sc.scalar_words[0];
                float f;
                std::memcpy(&f, &raw, sizeof(f));
                id_map_[id] = Value::make_f32(f);
            } else {
                uint32_t lo = sc.scalar_words.size() > 0 ? sc.scalar_words[0] : 0;
                uint32_t hi = sc.scalar_words.size() > 1 ? sc.scalar_words[1] : 0;
                uint64_t raw = (static_cast<uint64_t>(hi) << 32) | lo;
                double d;
                std::memcpy(&d, &raw, sizeof(d));
                id_map_[id] = Value::make_f64(d);
            }
        }
    }

    // Evaluate composites (may depend on scalar results above).
    for (auto &[id, sc] : module_->spec_constants) {
        if (sc.kind != SpecConstantKind::Composite)
            continue;
        std::vector<Value> elems;
        for (uint32_t cid : sc.constituent_ids) elems.push_back(lookup(cid));
        id_map_[id] = Value::make_composite(std::move(elems));
    }

    // Evaluate OpSpecConstantOp (simple arithmetic/logic on spec-constants).
    // This is a best-effort implementation of the most common ops.
    for (auto &[id, sc] : module_->spec_constants) {
        if (sc.kind != SpecConstantKind::Op)
            continue;
        if (sc.op_operands.size() < 1)
            continue;
        Value a = lookup(sc.op_operands[0]);
        Value b = sc.op_operands.size() > 1 ? lookup(sc.op_operands[1]) : Value::make_u32(0);
        Value result;
        switch (sc.op_opcode) {
            case SpvOpIAdd: result = Value::make_u32(a.scalar.u32 + b.scalar.u32); break;
            case SpvOpISub: result = Value::make_u32(a.scalar.u32 - b.scalar.u32); break;
            case SpvOpIMul: result = Value::make_u32(a.scalar.u32 * b.scalar.u32); break;
            case SpvOpUDiv:
                result = Value::make_u32(b.scalar.u32 ? a.scalar.u32 / b.scalar.u32 : 0);
                break;
            case SpvOpSDiv:
                result = Value::make_i32(b.scalar.i32 ? a.scalar.i32 / b.scalar.i32 : 0);
                break;
            case SpvOpUMod:
                result = Value::make_u32(b.scalar.u32 ? a.scalar.u32 % b.scalar.u32 : 0);
                break;
            case SpvOpSMod:
                result = Value::make_i32(b.scalar.i32 ? a.scalar.i32 % b.scalar.i32 : 0);
                break;
            case SpvOpShiftLeftLogical:
                result = Value::make_u32(a.scalar.u32 << b.scalar.u32);
                break;
            case SpvOpShiftRightLogical:
                result = Value::make_u32(a.scalar.u32 >> b.scalar.u32);
                break;
            case SpvOpShiftRightArithmetic:
                result = Value::make_i32(a.scalar.i32 >> b.scalar.u32);
                break;
            case SpvOpBitwiseAnd: result = Value::make_u32(a.scalar.u32 & b.scalar.u32); break;
            case SpvOpBitwiseOr: result = Value::make_u32(a.scalar.u32 | b.scalar.u32); break;
            case SpvOpBitwiseXor: result = Value::make_u32(a.scalar.u32 ^ b.scalar.u32); break;
            case SpvOpNot: result = Value::make_u32(~a.scalar.u32); break;
            case SpvOpIEqual: result = Value::make_bool(a.scalar.u32 == b.scalar.u32); break;
            case SpvOpINotEqual: result = Value::make_bool(a.scalar.u32 != b.scalar.u32); break;
            case SpvOpULessThan: result = Value::make_bool(a.scalar.u32 < b.scalar.u32); break;
            case SpvOpSLessThan: result = Value::make_bool(a.scalar.i32 < b.scalar.i32); break;
            case SpvOpSelect:
                if (sc.op_operands.size() >= 3) {
                    Value cond = lookup(sc.op_operands[0]);
                    result = cond.scalar.b ? lookup(sc.op_operands[1]) : lookup(sc.op_operands[2]);
                }
                break;
            default:
                result = Value::make_u32(0);
                diagnostics.push_back("OpSpecConstantOp: unhandled opcode "
                                      + std::to_string(sc.op_opcode));
        }
        id_map_[id] = result;
    }
}

Result<void> Interpreter::begin(const std::string &entry_name)
{
    // Find entry point.
    const EntryPoint *ep = nullptr;
    for (auto &e : module_->entry_points) {
        if (e.name == entry_name) {
            ep = &e;
            break;
        }
    }
    if (!ep)
        return Result<void>::err("Entry point not found: " + entry_name);

    entry_function_id_ = ep->function_id;
    entry_name_ = entry_name;
    finished_ = false;
    panicked_ = false;
    panic_msg_ = {};
    id_map_.clear();
    call_stack_.clear();
    debug_var_values_.clear();
    current_scope_id_ = 0;
    current_inlined_at_id_ = 0;

    // Initialize memory for all variables.
    init_memory();

    // Apply builtin overrides.
    for (auto &[id, var] : module_->variables) {
        auto &dset = module_->decorations_of(id);
        if (dset.has(SpvDecorationBuiltIn)) {
            uint32_t bi = dset.get(SpvDecorationBuiltIn);
            auto it = builtin_overrides_.find(bi);
            if (it != builtin_overrides_.end())
                memory_[id] = it->second;
        }
    }

    // Apply staged descriptor bindings (overrides values from init_memory).
    for (auto &[key, val] : descriptor_staging_) {
        uint32_t s = key >> 16, b = key & 0xFFFF;
        for (auto &[id, var] : module_->variables) {
            if (var.storage_class != SpvStorageClassStorageBuffer
                && var.storage_class != SpvStorageClassUniform
                && var.storage_class != SpvStorageClassUniformConstant)
                continue;
            auto &dset = module_->decorations_of(id);
            if (dset.get(SpvDecorationDescriptorSet) == s && dset.get(SpvDecorationBinding) == b) {
                memory_[id] = val;
                break;
            }
        }
    }

    // Apply staged location-based input bindings.
    for (auto &[loc, val] : location_staging_) {
        for (auto &[id, var] : module_->variables) {
            if (var.storage_class != SpvStorageClassInput)
                continue;
            auto &dset = module_->decorations_of(id);
            if (dset.has(SpvDecorationBuiltIn))
                continue;
            if (dset.get(SpvDecorationLocation) == loc) {
                memory_[id] = val;
                break;
            }
        }
    }

    // Build image variable map: var_id → image staging key.
    image_var_map_.clear();
    for (auto &[key, _] : image_staging_) {
        uint32_t s = key >> 16, b = key & 0xFFFF;
        for (auto &[id, var] : module_->variables) {
            auto &dset = module_->decorations_of(id);
            if (dset.get(SpvDecorationDescriptorSet) == s && dset.get(SpvDecorationBinding) == b) {
                image_var_map_[id] = key;
                break;
            }
        }
    }

    // Evaluate spec-constants.
    eval_spec_constants();

    // Also pre-populate id_map_ with all regular constants.
    for (auto &[id, val] : module_->constants) {
        if (!id_map_.count(id))
            id_map_[id] = val;
    }

    // Set PC to first instruction of the entry function.
    auto fit = module_->functions.find(entry_function_id_);
    if (fit == module_->functions.end() || fit->second.blocks.empty())
        return Result<void>::err("Entry function has no blocks");

    pc_.function_id = entry_function_id_;
    pc_.block_label = fit->second.blocks[0].label_id;
    pc_.instr_index = 0;
    prev_block_label_ = 0;

    return Result<void> {};
}

Result<void> Interpreter::restart()
{
    id_map_.clear();
    call_stack_.clear();
    return begin(entry_name_);
}

Result<void> Interpreter::set_descriptor(uint32_t set, uint32_t binding, const Value &value)
{
    // Always stage the value so it survives begin() / init_memory() calls.
    descriptor_staging_[(set << 16) | binding] = value;

    // Also write to memory_ immediately (takes effect if begin() already ran).
    for (auto &[id, var] : module_->variables) {
        if (var.storage_class != SpvStorageClassStorageBuffer
            && var.storage_class != SpvStorageClassUniform
            && var.storage_class != SpvStorageClassUniformConstant)
            continue;
        auto &dset = module_->decorations_of(id);
        if (dset.get(SpvDecorationDescriptorSet) == set
            && dset.get(SpvDecorationBinding) == binding)
        {
            memory_[id] = value;
            return Result<void> {};
        }
    }
    return Result<void>::err("No variable at descriptor set=" + std::to_string(set)
                             + " binding=" + std::to_string(binding));
}

Result<void> Interpreter::set_input_location(uint32_t location, Value value)
{
    // Always stage the value so it survives begin() / init_memory() calls.
    location_staging_[location] = value;
    // Also write to memory_ if already initialized.
    for (auto &[id, var] : module_->variables) {
        if (var.storage_class != SpvStorageClassInput)
            continue;
        auto &dset = module_->decorations_of(id);
        if (dset.has(SpvDecorationBuiltIn))
            continue;
        if (dset.get(SpvDecorationLocation) == location) {
            memory_[id] = std::move(value);
            return Result<void> {};
        }
    }
    return Result<void>::err("No Input variable at location=" + std::to_string(location));
}

Result<void> Interpreter::set_image(uint32_t set, uint32_t binding, std::string_view path)
{
    ImageData data = load_image(path);
    if (!data.valid())
        return Result<void>::err("Failed to load image: " + std::string(path));
    uint32_t key = (set << 16) | binding;
    image_staging_[key] = std::move(data);
    // Also update the var map immediately in case begin() already ran.
    for (auto &[id, var] : module_->variables) {
        auto &dset = module_->decorations_of(id);
        if (dset.get(SpvDecorationDescriptorSet) == set
            && dset.get(SpvDecorationBinding) == binding)
        {
            image_var_map_[id] = key;
            break;
        }
    }
    return Result<void> {};
}

Result<void> Interpreter::set_builtin(SpvBuiltIn builtin, Value value)
{
    builtin_overrides_[static_cast<uint32_t>(builtin)] = std::move(value);
    return Result<void> {};
}

Result<void> Interpreter::set_spec_constant(uint32_t spec_id, Value value)
{
    spec_overrides_[spec_id] = std::move(value);
    return Result<void> {};
}

// ---- pointer resolution ----------------------------------------------------

Result<Value *> Interpreter::index_into(Value &root, const std::vector<uint32_t> &chain)
{
    Value *cur = &root;
    for (uint32_t idx : chain) {
        if (cur->kind != Value::Kind::Composite)
            return Result<Value *>::err("Access chain into non-composite value");
        if (idx >= cur->elements.size())
            return Result<Value *>::err("Access chain index " + std::to_string(idx)
                                        + " out of bounds (size="
                                        + std::to_string(cur->elements.size()) + ")");
        cur = &cur->elements[idx];
    }
    return cur;
}

Result<const Value *> Interpreter::index_into(const Value &root, const std::vector<uint32_t> &chain)
{
    const Value *cur = &root;
    for (uint32_t idx : chain) {
        if (cur->kind != Value::Kind::Composite)
            return Result<const Value *>::err("Access chain into non-composite value");
        if (idx >= cur->elements.size())
            return Result<const Value *>::err("Access chain index out of bounds");
        cur = &cur->elements[idx];
    }
    return cur;
}

Result<Value> Interpreter::load_ptr(const Value &ptr) const
{
    if (ptr.kind != Value::Kind::Pointer)
        return Result<Value>::err("load_ptr: not a pointer");
    auto mit = memory_.find(ptr.ptr_base_var);
    if (mit == memory_.end())
        return Result<Value>::err("load_ptr: variable " + std::to_string(ptr.ptr_base_var)
                                  + " not in memory");
    if (ptr.ptr_chain.empty())
        return mit->second;
    auto r = index_into(mit->second, ptr.ptr_chain);
    if (!r)
        return Result<Value>::err(r.error().message);
    return *r.value();
}

Result<void> Interpreter::store_ptr(const Value &ptr, Value val)
{
    if (ptr.kind != Value::Kind::Pointer)
        return Result<void>::err("store_ptr: not a pointer");
    auto mit = memory_.find(ptr.ptr_base_var);
    if (mit == memory_.end())
        return Result<void>::err("store_ptr: variable " + std::to_string(ptr.ptr_base_var)
                                 + " not in memory");
    if (ptr.ptr_chain.empty()) {
        mit->second = std::move(val);
        return Result<void> {};
    }
    auto r = index_into(mit->second, ptr.ptr_chain);
    if (!r)
        return Result<void>::err(r.error().message);
    *r.value() = std::move(val);
    return Result<void> {};
}

// ---- PC management ---------------------------------------------------------

bool Interpreter::advance_pc()
{
    auto fit = module_->functions.find(pc_.function_id);
    if (fit == module_->functions.end())
        return false;
    const Function &fn = fit->second;
    auto bit = fn.block_index.find(pc_.block_label);
    if (bit == fn.block_index.end())
        return false;
    const BasicBlock &bb = fn.blocks[bit->second];
    if (pc_.instr_index + 1 < bb.instructions.size()) {
        pc_.instr_index++;
        return true;
    }
    return false; // End of block (should have branched).
}

// ---- instruction execution -------------------------------------------------

StopReason Interpreter::step_instruction()
{
    if (finished_ || panicked_)
        return panicked_ ? StopReason::Panic : StopReason::EntryFinished;

    auto fit = module_->functions.find(pc_.function_id);
    if (fit == module_->functions.end()) {
        panic("Invalid function id " + std::to_string(pc_.function_id));
        return StopReason::Panic;
    }
    const Function &fn = fit->second;
    auto bit = fn.block_index.find(pc_.block_label);
    if (bit == fn.block_index.end()) {
        panic("Invalid block label " + std::to_string(pc_.block_label));
        return StopReason::Panic;
    }
    const BasicBlock &bb = fn.blocks[bit->second];
    if (pc_.instr_index >= bb.instructions.size()) {
        panic("PC past end of block");
        return StopReason::Panic;
    }

    const Instruction &inst = bb.instructions[pc_.instr_index];
    auto r = execute_one(inst);
    if (r == StopReason::Panic || r == StopReason::EntryFinished)
        return r;
    // Check breakpoints at the NEW PC (the instruction we've arrived at but
    // not yet executed).
    if (check_breakpoint_at_pc())
        return StopReason::Breakpoint;
    return r;
}

StopReason Interpreter::run_to_breakpoint()
{
    // Record the source location where we are *starting* so that we don't
    // immediately re-fire a breakpoint at the same line we just stopped on.
    SourceLoc start_loc = current_source_location();
    bool left_start_loc = !start_loc.valid(); // if no location, no skip needed
    while (!finished_ && !panicked_) {
        // Before stepping, check if we've moved to a different source location.
        if (!left_start_loc) {
            SourceLoc cur = current_source_location();
            if (cur.line != start_loc.line || cur.file != start_loc.file)
                left_start_loc = true;
        }
        auto r = step_instruction();
        if (r == StopReason::Breakpoint) {
            // Only surface the breakpoint once we've left the starting location.
            if (left_start_loc)
                return r;
            continue;
        }
        if (r != StopReason::Step)
            return r;
    }
    return panicked_ ? StopReason::Panic : StopReason::EntryFinished;
}

StopReason Interpreter::step_out()
{
    size_t target_depth = call_stack_.size();
    while (!finished_ && !panicked_) {
        auto r = step_instruction();
        if (r == StopReason::Panic || r == StopReason::EntryFinished)
            return r;
        if (call_stack_.size() < target_depth)
            return StopReason::Step;
    }
    return panicked_ ? StopReason::Panic : StopReason::EntryFinished;
}

// ---- arithmetic/logic ------------------------------------------------------

StopReason Interpreter::exec_arith_logic(const Instruction &inst)
{
    // Helper to get scalar operands
    auto get = [&](uint32_t idx) -> Value { return lookup(inst.operand(idx)); };

    Value a = get(0);
    Value b = inst.operand_count() > 1 ? get(1) : Value::make_u32(0);

    Value result;

#define OP_SCALAR_INT(op)                                       \
    if (a.kind == Value::Kind::Int32)                           \
        result = Value::make_i32(a.scalar.i32 op b.scalar.i32); \
    else if (a.kind == Value::Kind::UInt32)                     \
        result = Value::make_u32(a.scalar.u32 op b.scalar.u32); \
    else if (a.kind == Value::Kind::Int64)                      \
        result = Value::make_i64(a.scalar.i64 op b.scalar.i64); \
    else                                                        \
        result = Value::make_u64(a.scalar.u64 op b.scalar.u64)

#define OP_SCALAR_FP(op)                                        \
    if (a.kind == Value::Kind::Float32)                         \
        result = Value::make_f32(a.scalar.f32 op b.scalar.f32); \
    else                                                        \
        result = Value::make_f64(a.scalar.f64 op b.scalar.f64)

    // For vector operations, apply element-wise.
    auto vec_binop = [&](auto elem_op) -> Value
    {
        if (a.kind != Value::Kind::Composite)
            return elem_op(a, b);
        std::vector<Value> elems;
        bool b_is_scalar = (b.kind != Value::Kind::Composite);
        for (size_t i = 0; i < a.elements.size(); ++i) {
            const Value &bi = b_is_scalar ? b : b.elements[i];
            elems.push_back(elem_op(a.elements[i], bi));
        }
        return Value::make_composite(std::move(elems));
    };

    switch (inst.opcode) {
        // Integer arithmetic
        case SpvOpIAdd:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_i32(x.scalar.i32 + y.scalar.i32);
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_u32(x.scalar.u32 + y.scalar.u32);
                    if (x.kind == Value::Kind::Int64)
                        return Value::make_i64(x.scalar.i64 + y.scalar.i64);
                    return Value::make_u64(x.scalar.u64 + y.scalar.u64);
                });
            break;
        case SpvOpISub:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_i32(x.scalar.i32 - y.scalar.i32);
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_u32(x.scalar.u32 - y.scalar.u32);
                    if (x.kind == Value::Kind::Int64)
                        return Value::make_i64(x.scalar.i64 - y.scalar.i64);
                    return Value::make_u64(x.scalar.u64 - y.scalar.u64);
                });
            break;
        case SpvOpIMul:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_i32(x.scalar.i32 * y.scalar.i32);
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_u32(x.scalar.u32 * y.scalar.u32);
                    if (x.kind == Value::Kind::Int64)
                        return Value::make_i64(x.scalar.i64 * y.scalar.i64);
                    return Value::make_u64(x.scalar.u64 * y.scalar.u64);
                });
            break;
        case SpvOpUDiv:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_u32(y.scalar.u32 ? x.scalar.u32 / y.scalar.u32 : 0);
                    return Value::make_u64(y.scalar.u64 ? x.scalar.u64 / y.scalar.u64 : 0);
                });
            break;
        case SpvOpSDiv:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_i32(y.scalar.i32 ? x.scalar.i32 / y.scalar.i32 : 0);
                    return Value::make_i64(y.scalar.i64 ? x.scalar.i64 / y.scalar.i64 : 0);
                });
            break;
        case SpvOpUMod:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_u32(y.scalar.u32 ? x.scalar.u32 % y.scalar.u32 : 0);
                    return Value::make_u64(y.scalar.u64 ? x.scalar.u64 % y.scalar.u64 : 0);
                });
            break;
        case SpvOpSRem:
        case SpvOpSMod:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_i32(y.scalar.i32 ? x.scalar.i32 % y.scalar.i32 : 0);
                    return Value::make_i64(y.scalar.i64 ? x.scalar.i64 % y.scalar.i64 : 0);
                });
            break;
        case SpvOpSNegate:
            result = vec_binop(
                [](const Value &x, const Value &)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_i32(-x.scalar.i32);
                    if (x.kind == Value::Kind::Int64)
                        return Value::make_i64(-x.scalar.i64);
                    return Value::make_i32(0);
                });
            break;

        // Float arithmetic
        case SpvOpFAdd:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_f32(x.scalar.f32 + y.scalar.f32);
                    return Value::make_f64(x.scalar.f64 + y.scalar.f64);
                });
            break;
        case SpvOpFSub:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_f32(x.scalar.f32 - y.scalar.f32);
                    return Value::make_f64(x.scalar.f64 - y.scalar.f64);
                });
            break;
        case SpvOpFMul:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_f32(x.scalar.f32 * y.scalar.f32);
                    return Value::make_f64(x.scalar.f64 * y.scalar.f64);
                });
            break;
        case SpvOpFDiv:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_f32(x.scalar.f32 / y.scalar.f32);
                    return Value::make_f64(x.scalar.f64 / y.scalar.f64);
                });
            break;
        case SpvOpFMod:
        case SpvOpFRem:
            result = vec_binop(
                [&](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_f32(
                            inst.opcode == SpvOpFRem
                                ? std::fmod(x.scalar.f32, y.scalar.f32)
                                : x.scalar.f32
                                      - y.scalar.f32 * std::floor(x.scalar.f32 / y.scalar.f32));
                    return Value::make_f64(
                        inst.opcode == SpvOpFRem
                            ? std::fmod(x.scalar.f64, y.scalar.f64)
                            : x.scalar.f64
                                  - y.scalar.f64 * std::floor(x.scalar.f64 / y.scalar.f64));
                });
            break;
        case SpvOpFNegate:
            result = vec_binop(
                [](const Value &x, const Value &)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_f32(-x.scalar.f32);
                    return Value::make_f64(-x.scalar.f64);
                });
            break;

        // Dot product
        case SpvOpDot:
            {
                if (a.kind != Value::Kind::Composite)
                    break;
                float sum = 0.0f;
                double sumd = 0.0;
                bool is64 = false;
                for (size_t i = 0; i < a.elements.size(); ++i) {
                    auto &ae = a.elements[i];
                    auto &be = b.elements[i];
                    if (ae.kind == Value::Kind::Float64) {
                        sumd += ae.scalar.f64 * be.scalar.f64;
                        is64 = true;
                    } else
                        sum += ae.scalar.f32 * be.scalar.f32;
                }
                result = is64 ? Value::make_f64(sumd) : Value::make_f32(sum);
                break;
            }

        // Vector * scalar
        case SpvOpVectorTimesScalar:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_f32(x.scalar.f32 * y.scalar.f32);
                    return Value::make_f64(x.scalar.f64 * y.scalar.f64);
                });
            break;

        // Matrix * scalar
        case SpvOpMatrixTimesScalar:
            result = vec_binop(
                [](const Value &x, const Value &y) -> Value
                {
                    // x is a column (composite), y is scalar — recurse
                    if (x.kind != Value::Kind::Composite) {
                        if (x.kind == Value::Kind::Float32)
                            return Value::make_f32(x.scalar.f32 * y.scalar.f32);
                        return Value::make_f64(x.scalar.f64 * y.scalar.f64);
                    }
                    std::vector<Value> elems;
                    for (auto &e : x.elements) {
                        if (e.kind == Value::Kind::Float32)
                            elems.push_back(Value::make_f32(e.scalar.f32 * y.scalar.f32));
                        else
                            elems.push_back(Value::make_f64(e.scalar.f64 * y.scalar.f64));
                    }
                    return Value::make_composite(std::move(elems));
                });
            break;

        // Bitwise
        case SpvOpShiftLeftLogical:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32 || x.kind == Value::Kind::UInt32)
                        return Value {x.kind, {.u32 = x.scalar.u32 << y.scalar.u32}};
                    return Value {x.kind, {.u64 = x.scalar.u64 << y.scalar.u64}};
                });
            break;
        case SpvOpShiftRightLogical:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32 || x.kind == Value::Kind::UInt32)
                        return Value::make_u32(x.scalar.u32 >> y.scalar.u32);
                    return Value::make_u64(x.scalar.u64 >> y.scalar.u64);
                });
            break;
        case SpvOpShiftRightArithmetic:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_i32(x.scalar.i32 >> y.scalar.u32);
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_u32(x.scalar.u32 >> y.scalar.u32);
                    return Value::make_i64(x.scalar.i64 >> y.scalar.u64);
                });
            break;
        case SpvOpBitwiseAnd:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32 || x.kind == Value::Kind::UInt32)
                        return Value {x.kind, {.u32 = x.scalar.u32 & y.scalar.u32}};
                    return Value {x.kind, {.u64 = x.scalar.u64 & y.scalar.u64}};
                });
            break;
        case SpvOpBitwiseOr:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32 || x.kind == Value::Kind::UInt32)
                        return Value {x.kind, {.u32 = x.scalar.u32 | y.scalar.u32}};
                    return Value {x.kind, {.u64 = x.scalar.u64 | y.scalar.u64}};
                });
            break;
        case SpvOpBitwiseXor:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32 || x.kind == Value::Kind::UInt32)
                        return Value {x.kind, {.u32 = x.scalar.u32 ^ y.scalar.u32}};
                    return Value {x.kind, {.u64 = x.scalar.u64 ^ y.scalar.u64}};
                });
            break;
        case SpvOpNot:
            result = vec_binop(
                [](const Value &x, const Value &)
                {
                    if (x.kind == Value::Kind::Int32 || x.kind == Value::Kind::UInt32)
                        return Value {x.kind, {.u32 = ~x.scalar.u32}};
                    return Value {x.kind, {.u64 = ~x.scalar.u64}};
                });
            break;

        // Logical
        case SpvOpLogicalAnd:
            result = vec_binop([](const Value &x, const Value &y)
                               { return Value::make_bool(x.scalar.b && y.scalar.b); });
            break;
        case SpvOpLogicalOr:
            result = vec_binop([](const Value &x, const Value &y)
                               { return Value::make_bool(x.scalar.b || y.scalar.b); });
            break;
        case SpvOpLogicalNot:
            result = vec_binop([](const Value &x, const Value &)
                               { return Value::make_bool(!x.scalar.b); });
            break;
        case SpvOpLogicalEqual:
            result = vec_binop([](const Value &x, const Value &y)
                               { return Value::make_bool(x.scalar.b == y.scalar.b); });
            break;
        case SpvOpLogicalNotEqual:
            result = vec_binop([](const Value &x, const Value &y)
                               { return Value::make_bool(x.scalar.b != y.scalar.b); });
            break;
        case SpvOpAny:
            {
                bool any = false;
                if (a.kind == Value::Kind::Composite)
                    for (auto &e : a.elements) any |= e.scalar.b;
                else
                    any = a.scalar.b;
                result = Value::make_bool(any);
                break;
            }
        case SpvOpAll:
            {
                bool all = true;
                if (a.kind == Value::Kind::Composite)
                    for (auto &e : a.elements) all &= e.scalar.b;
                else
                    all = a.scalar.b;
                result = Value::make_bool(all);
                break;
            }

        // Integer comparisons
        case SpvOpIEqual:
            result = vec_binop([](const Value &x, const Value &y)
                               { return Value::make_bool(x.scalar.u64 == y.scalar.u64); });
            break;
        case SpvOpINotEqual:
            result = vec_binop([](const Value &x, const Value &y)
                               { return Value::make_bool(x.scalar.u64 != y.scalar.u64); });
            break;
        case SpvOpULessThan:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_bool(x.scalar.u32 < y.scalar.u32);
                    return Value::make_bool(x.scalar.u64 < y.scalar.u64);
                });
            break;
        case SpvOpULessThanEqual:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_bool(x.scalar.u32 <= y.scalar.u32);
                    return Value::make_bool(x.scalar.u64 <= y.scalar.u64);
                });
            break;
        case SpvOpUGreaterThan:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_bool(x.scalar.u32 > y.scalar.u32);
                    return Value::make_bool(x.scalar.u64 > y.scalar.u64);
                });
            break;
        case SpvOpUGreaterThanEqual:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_bool(x.scalar.u32 >= y.scalar.u32);
                    return Value::make_bool(x.scalar.u64 >= y.scalar.u64);
                });
            break;
        case SpvOpSLessThan:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_bool(x.scalar.i32 < y.scalar.i32);
                    return Value::make_bool(x.scalar.i64 < y.scalar.i64);
                });
            break;
        case SpvOpSLessThanEqual:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_bool(x.scalar.i32 <= y.scalar.i32);
                    return Value::make_bool(x.scalar.i64 <= y.scalar.i64);
                });
            break;
        case SpvOpSGreaterThan:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_bool(x.scalar.i32 > y.scalar.i32);
                    return Value::make_bool(x.scalar.i64 > y.scalar.i64);
                });
            break;
        case SpvOpSGreaterThanEqual:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_bool(x.scalar.i32 >= y.scalar.i32);
                    return Value::make_bool(x.scalar.i64 >= y.scalar.i64);
                });
            break;

        // Float comparisons (Ord = false if either NaN, Unord = true if either NaN)
        case SpvOpFOrdEqual:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_bool(x.scalar.f32 == y.scalar.f32);
                    return Value::make_bool(x.scalar.f64 == y.scalar.f64);
                });
            break;
        case SpvOpFUnordEqual:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_bool(!(x.scalar.f32 != y.scalar.f32));
                    return Value::make_bool(!(x.scalar.f64 != y.scalar.f64));
                });
            break;
        case SpvOpFOrdNotEqual:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_bool(x.scalar.f32 != y.scalar.f32);
                    return Value::make_bool(x.scalar.f64 != y.scalar.f64);
                });
            break;
        case SpvOpFOrdLessThan:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_bool(x.scalar.f32 < y.scalar.f32);
                    return Value::make_bool(x.scalar.f64 < y.scalar.f64);
                });
            break;
        case SpvOpFOrdLessThanEqual:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_bool(x.scalar.f32 <= y.scalar.f32);
                    return Value::make_bool(x.scalar.f64 <= y.scalar.f64);
                });
            break;
        case SpvOpFOrdGreaterThan:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_bool(x.scalar.f32 > y.scalar.f32);
                    return Value::make_bool(x.scalar.f64 > y.scalar.f64);
                });
            break;
        case SpvOpFOrdGreaterThanEqual:
            result = vec_binop(
                [](const Value &x, const Value &y)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_bool(x.scalar.f32 >= y.scalar.f32);
                    return Value::make_bool(x.scalar.f64 >= y.scalar.f64);
                });
            break;

        // Select
        case SpvOpSelect:
            {
                Value cond = lookup(inst.operand(0));
                Value tv = lookup(inst.operand(1));
                Value fv = lookup(inst.operand(2));
                if (cond.kind == Value::Kind::Composite) {
                    // Vector select: element-wise
                    std::vector<Value> elems;
                    for (size_t i = 0; i < cond.elements.size(); ++i)
                        elems.push_back(cond.elements[i].scalar.b ? tv.elements[i]
                                                                  : fv.elements[i]);
                    result = Value::make_composite(std::move(elems));
                } else {
                    result = cond.scalar.b ? tv : fv;
                }
                break;
            }

        // IsNan / IsInf
        case SpvOpIsNan:
            result = vec_binop(
                [](const Value &x, const Value &)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_bool(std::isnan(x.scalar.f32));
                    return Value::make_bool(std::isnan(x.scalar.f64));
                });
            break;
        case SpvOpIsInf:
            result = vec_binop(
                [](const Value &x, const Value &)
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_bool(std::isinf(x.scalar.f32));
                    return Value::make_bool(std::isinf(x.scalar.f64));
                });
            break;

        default:
            diagnostics.push_back("Unhandled arith/logic opcode " + std::to_string(inst.opcode));
            result = Value::make_u32(0);
    }

    if (inst.result_id)
        id_map_[inst.result_id] = result;
    advance_pc();
    return StopReason::Step;
}

// ---- type conversion -------------------------------------------------------

StopReason Interpreter::exec_convert(const Instruction &inst)
{
    Value a = lookup(inst.operand(0));
    Value result;

    auto convert_elem = [&](const Value &x) -> Value
    {
        switch (inst.opcode) {
            case SpvOpConvertSToF:
                if (x.kind == Value::Kind::Int32)
                    return Value::make_f32(static_cast<float>(x.scalar.i32));
                return Value::make_f64(static_cast<double>(x.scalar.i64));
            case SpvOpConvertUToF:
                if (x.kind == Value::Kind::UInt32)
                    return Value::make_f32(static_cast<float>(x.scalar.u32));
                return Value::make_f64(static_cast<double>(x.scalar.u64));
            case SpvOpConvertFToS:
                if (x.kind == Value::Kind::Float32)
                    return Value::make_i32(static_cast<int32_t>(x.scalar.f32));
                return Value::make_i64(static_cast<int64_t>(x.scalar.f64));
            case SpvOpConvertFToU:
                if (x.kind == Value::Kind::Float32)
                    return Value::make_u32(static_cast<uint32_t>(x.scalar.f32));
                return Value::make_u64(static_cast<uint64_t>(x.scalar.f64));
            case SpvOpFConvert:
                if (x.kind == Value::Kind::Float32)
                    return Value::make_f64(x.scalar.f32);
                return Value::make_f32(static_cast<float>(x.scalar.f64));
            case SpvOpSConvert:
                if (x.kind == Value::Kind::Int32)
                    return Value::make_i64(x.scalar.i32);
                return Value::make_i32(static_cast<int32_t>(x.scalar.i64));
            case SpvOpUConvert:
                if (x.kind == Value::Kind::UInt32)
                    return Value::make_u64(x.scalar.u32);
                return Value::make_u32(static_cast<uint32_t>(x.scalar.u64));
            case SpvOpBitcast:
                {
                    // Keep the bit pattern, reinterpret the type based on result type.
                    auto *t = module_->type_of(inst.type_id);
                    if (!t)
                        return x;
                    if (t->kind == SpvType::Kind::Float) {
                        auto *ft = static_cast<const FloatType *>(t);
                        if (ft->width <= 32) {
                            float f;
                            uint32_t u = x.scalar.u32;
                            std::memcpy(&f, &u, 4);
                            return Value::make_f32(f);
                        }
                        double d;
                        uint64_t u = x.scalar.u64;
                        std::memcpy(&d, &u, 8);
                        return Value::make_f64(d);
                    }
                    if (t->kind == SpvType::Kind::Int) {
                        auto *it = static_cast<const IntType *>(t);
                        if (it->width <= 32)
                            return it->is_signed ? Value::make_i32(x.scalar.i32)
                                                 : Value::make_u32(x.scalar.u32);
                        return it->is_signed ? Value::make_i64(x.scalar.i64)
                                             : Value::make_u64(x.scalar.u64);
                    }
                    return x;
                }
            default: return x;
        }
    };

    if (a.kind == Value::Kind::Composite) {
        std::vector<Value> elems;
        for (auto &e : a.elements) elems.push_back(convert_elem(e));
        result = Value::make_composite(std::move(elems));
    } else {
        result = convert_elem(a);
    }

    if (inst.result_id)
        id_map_[inst.result_id] = result;
    advance_pc();
    return StopReason::Step;
}

// ---- memory ----------------------------------------------------------------

StopReason Interpreter::exec_memory(const Instruction &inst)
{
    switch (inst.opcode) {
        case SpvOpVariable:
            {
                // Function-scope variable allocation.
                // words: [type_id, result_id, storage_class, optional_init]
                uint32_t sc = inst.operand(0);
                auto *ptr_type = module_->type_of(inst.type_id);
                Value init;
                if (inst.operand_count() > 1) {
                    init = lookup(inst.operand(1));
                } else if (ptr_type && ptr_type->kind == SpvType::Kind::Pointer) {
                    auto *pt = static_cast<const PointerType *>(ptr_type);
                    if (opts_.init_pattern == InitPattern::Zero)
                        init = zero_for_type(pt->pointee.get(), *module_);
                    else
                        init = pattern_for_type(pt->pointee.get(), *module_, opts_.init_value);
                }
                memory_[inst.result_id] = std::move(init);
                // The variable's "address" is stored as a pointer value.
                id_map_[inst.result_id] = Value::make_pointer(sc, inst.result_id);
                break;
            }

        case SpvOpLoad:
            {
                // words: [type_id, result_id, pointer_id, optional_memory_access]
                Value ptr = lookup(inst.operand(0));
                auto r = load_ptr(ptr);
                if (!r) {
                    panic(r.error().message);
                    return StopReason::Panic;
                }
                id_map_[inst.result_id] = std::move(*r);
                break;
            }

        case SpvOpStore:
            {
                // words: [pointer_id, value_id, optional_memory_access]
                Value ptr = lookup(inst.operand(0));
                Value val = lookup(inst.operand(1));
                auto r = store_ptr(ptr, std::move(val));
                if (!r) {
                    panic(r.error().message);
                    return StopReason::Panic;
                }
                break;
            }

        case SpvOpAccessChain:
        case SpvOpInBoundsAccessChain:
            {
                // words: [type_id, result_id, base_ptr_id, index_ids...]
                Value base = lookup(inst.operand(0));
                if (base.kind != Value::Kind::Pointer) {
                    panic("OpAccessChain base is not a pointer");
                    return StopReason::Panic;
                }
                std::vector<uint32_t> new_chain = base.ptr_chain;
                for (uint32_t i = 1; i < inst.operand_count(); ++i) {
                    Value idx = lookup(inst.operand(i));
                    // Resolve index to a concrete uint32.
                    uint32_t idx_val = 0;
                    if (idx.kind == Value::Kind::UInt32)
                        idx_val = idx.scalar.u32;
                    else if (idx.kind == Value::Kind::Int32)
                        idx_val = static_cast<uint32_t>(idx.scalar.i32);
                    else if (idx.kind == Value::Kind::Int64)
                        idx_val = static_cast<uint32_t>(idx.scalar.i64);
                    else if (idx.kind == Value::Kind::UInt64)
                        idx_val = static_cast<uint32_t>(idx.scalar.u64);
                    new_chain.push_back(idx_val);
                }
                id_map_[inst.result_id] = Value::make_pointer(base.ptr_storage_class,
                                                              base.ptr_base_var,
                                                              std::move(new_chain));
                break;
            }

        case SpvOpCopyObject:
            {
                id_map_[inst.result_id] = lookup(inst.operand(0));
                break;
            }

        default: diagnostics.push_back("Unhandled memory opcode " + std::to_string(inst.opcode));
    }
    advance_pc();
    return StopReason::Step;
}

// ---- composite operations --------------------------------------------------

StopReason Interpreter::exec_composite(const Instruction &inst)
{
    switch (inst.opcode) {
        case SpvOpCompositeConstruct:
            {
                std::vector<Value> elems;
                for (uint32_t i = 0; i < inst.operand_count(); ++i)
                    elems.push_back(lookup(inst.operand(i)));
                id_map_[inst.result_id] = Value::make_composite(std::move(elems));
                break;
            }

        case SpvOpCompositeExtract:
            {
                // words: [type_id, result_id, composite_id, indices...]
                Value comp = lookup(inst.operand(0));
                for (uint32_t i = 1; i < inst.operand_count(); ++i) {
                    uint32_t idx = inst.operand(i);
                    if (comp.kind != Value::Kind::Composite || idx >= comp.elements.size()) {
                        panic("OpCompositeExtract index out of bounds");
                        return StopReason::Panic;
                    }
                    comp = comp.elements[idx];
                }
                id_map_[inst.result_id] = comp;
                break;
            }

        case SpvOpCompositeInsert:
            {
                // words: [type_id, result_id, object_id, composite_id, indices...]
                Value obj = lookup(inst.operand(0));
                Value comp = lookup(inst.operand(1));
                // Build access chain from indices.
                std::vector<uint32_t> chain;
                for (uint32_t i = 2; i < inst.operand_count(); ++i)
                    chain.push_back(inst.operand(i));
                auto r = index_into(comp, chain);
                if (!r) {
                    panic(r.error().message);
                    return StopReason::Panic;
                }
                *r.value() = std::move(obj);
                id_map_[inst.result_id] = std::move(comp);
                break;
            }

        case SpvOpVectorExtractDynamic:
            {
                Value vec = lookup(inst.operand(0));
                Value idx = lookup(inst.operand(1));
                uint32_t i = idx.scalar.u32;
                if (vec.kind != Value::Kind::Composite || i >= vec.elements.size()) {
                    panic("OpVectorExtractDynamic index out of bounds");
                    return StopReason::Panic;
                }
                id_map_[inst.result_id] = vec.elements[i];
                break;
            }

        case SpvOpVectorInsertDynamic:
            {
                Value vec = lookup(inst.operand(0));
                Value obj = lookup(inst.operand(1));
                Value idx = lookup(inst.operand(2));
                uint32_t i = idx.scalar.u32;
                if (vec.kind != Value::Kind::Composite || i >= vec.elements.size()) {
                    panic("OpVectorInsertDynamic index out of bounds");
                    return StopReason::Panic;
                }
                vec.elements[i] = std::move(obj);
                id_map_[inst.result_id] = std::move(vec);
                break;
            }

        case SpvOpVectorShuffle:
            {
                // words: [type_id, result_id, vec1_id, vec2_id, component_indices...]
                Value v1 = lookup(inst.operand(0));
                Value v2 = lookup(inst.operand(1));
                std::vector<Value> elems;
                for (uint32_t i = 2; i < inst.operand_count(); ++i) {
                    uint32_t idx = inst.operand(i);
                    if (idx == 0xFFFFFFFFu) {
                        elems.push_back(Value::make_u32(0)); // undefined
                    } else {
                        size_t v1_size = v1.kind == Value::Kind::Composite ? v1.elements.size() : 1;
                        if (idx < v1_size)
                            elems.push_back(v1.kind == Value::Kind::Composite ? v1.elements[idx]
                                                                              : v1);
                        else
                            elems.push_back(v2.kind == Value::Kind::Composite
                                                ? v2.elements[idx - v1_size]
                                                : v2);
                    }
                }
                id_map_[inst.result_id] = Value::make_composite(std::move(elems));
                break;
            }

        case SpvOpTranspose:
            {
                Value mat = lookup(inst.operand(0));
                if (mat.kind != Value::Kind::Composite || mat.elements.empty())
                    break;
                size_t cols = mat.elements.size();
                size_t rows = (mat.elements[0].kind == Value::Kind::Composite)
                                ? mat.elements[0].elements.size()
                                : 1;
                std::vector<Value> result_cols;
                for (size_t r = 0; r < rows; ++r) {
                    std::vector<Value> col;
                    for (size_t c = 0; c < cols; ++c) col.push_back(mat.elements[c].elements[r]);
                    result_cols.push_back(Value::make_composite(std::move(col)));
                }
                id_map_[inst.result_id] = Value::make_composite(std::move(result_cols));
                break;
            }

        default: diagnostics.push_back("Unhandled composite opcode " + std::to_string(inst.opcode));
    }
    advance_pc();
    return StopReason::Step;
}

// ---- control flow ----------------------------------------------------------

StopReason Interpreter::exec_control_flow(const Instruction &inst)
{
    switch (inst.opcode) {
        case SpvOpSelectionMerge:
        case SpvOpLoopMerge:
            // Structured control flow hints — no action needed for execution.
            advance_pc();
            return StopReason::Step;

        case SpvOpBranch:
            {
                uint32_t target = inst.operand(0);
                prev_block_label_ = pc_.block_label;
                pc_.block_label = target;
                pc_.instr_index = 0;
                return StopReason::Step;
            }

        case SpvOpBranchConditional:
            {
                Value cond = lookup(inst.operand(0));
                uint32_t tt = inst.operand(1);
                uint32_t ft = inst.operand(2);
                prev_block_label_ = pc_.block_label;
                pc_.block_label = cond.scalar.b ? tt : ft;
                pc_.instr_index = 0;
                return StopReason::Step;
            }

        case SpvOpSwitch:
            {
                Value selector = lookup(inst.operand(0));
                uint32_t default_target = inst.operand(1);
                uint32_t target = default_target;
                // Remaining operands are (literal, label) pairs.
                for (uint32_t i = 2; i + 1 < inst.operand_count(); i += 2) {
                    if (selector.scalar.u32 == inst.operand(i)) {
                        target = inst.operand(i + 1);
                        break;
                    }
                }
                prev_block_label_ = pc_.block_label;
                pc_.block_label = target;
                pc_.instr_index = 0;
                return StopReason::Step;
            }

        case SpvOpPhi:
            {
                // words: [type_id, result_id, (value_id, label_id)...]
                Value result = Value::make_u32(0);
                for (uint32_t i = 0; i + 1 < inst.operand_count(); i += 2) {
                    if (inst.operand(i + 1) == prev_block_label_) {
                        result = lookup(inst.operand(i));
                        break;
                    }
                }
                id_map_[inst.result_id] = result;
                advance_pc();
                return StopReason::Step;
            }

        case SpvOpReturn:
            {
                if (call_stack_.empty()) {
                    finished_ = true;
                    return StopReason::EntryFinished;
                }
                CallFrame frame = call_stack_.back();
                call_stack_.pop_back();
                pc_ = frame.return_pc;
                advance_pc();
                return StopReason::Step;
            }

        case SpvOpReturnValue:
            {
                Value ret = lookup(inst.operand(0));
                if (call_stack_.empty()) {
                    finished_ = true;
                    return StopReason::EntryFinished;
                }
                CallFrame frame = call_stack_.back();
                call_stack_.pop_back();
                if (frame.return_value_id)
                    id_map_[frame.return_value_id] = std::move(ret);
                pc_ = frame.return_pc;
                advance_pc();
                return StopReason::Step;
            }

        case SpvOpUnreachable: panic("OpUnreachable executed"); return StopReason::Panic;

        case SpvOpFunctionCall:
            {
                // words: [type_id, result_id, function_id, arg_ids...]
                uint32_t callee_id = inst.operand(0);
                auto fit = module_->functions.find(callee_id);
                if (fit == module_->functions.end()) {
                    panic("Call to unknown function " + std::to_string(callee_id));
                    return StopReason::Panic;
                }
                const Function &callee = fit->second;

                // Bind arguments to parameter ids.
                for (size_t i = 0; i < callee.parameters.size(); ++i) {
                    auto [param_id, param_type_id] = callee.parameters[i];
                    id_map_[param_id] = lookup(inst.operand(1 + i));
                }

                // Push return frame.
                PC return_pc = pc_;
                call_stack_.push_back({return_pc, inst.result_id});

                // Jump to callee entry block.
                if (callee.blocks.empty()) {
                    panic("Call to function with no blocks");
                    return StopReason::Panic;
                }
                prev_block_label_ = 0;
                pc_.function_id = callee_id;
                pc_.block_label = callee.blocks[0].label_id;
                pc_.instr_index = 0;
                return StopReason::Step;
            }

        default:
            diagnostics.push_back("Unhandled control flow opcode " + std::to_string(inst.opcode));
            advance_pc();
            return StopReason::Step;
    }
}

// ---- image instructions ----------------------------------------------------

StopReason Interpreter::exec_image(const Instruction &inst)
{
    auto get = [&](uint32_t idx) -> Value { return lookup(inst.operand(idx)); };

    switch (inst.opcode) {
        case SpvOpSampledImage:
            {
                // %result = OpSampledImage %SampledImageType %image %sampler
                // Keep the image handle; ignore sampler state (default bilinear assumed).
                id_map_[inst.result_id] = get(0);
                break;
            }

        case SpvOpImage:
            {
                // %result = OpImage %ImageType %sampled_image
                // Extract underlying image handle from a combined sampled-image.
                id_map_[inst.result_id] = get(0);
                break;
            }

        case SpvOpImageSampleImplicitLod:
        case SpvOpImageSampleExplicitLod:
            {
                // %result = OpImageSampleXxxLod %ResType %sampled_image %coord [ImageOperands]
                Value si_handle = get(0);
                Value coord = get(1);
                uint32_t var_id = si_handle.scalar.u32;

                auto it = image_var_map_.find(var_id);
                if (it == image_var_map_.end()) {
                    diagnostics.push_back("OpImageSample*: no image bound at variable %"
                                          + std::to_string(var_id) + "; returning zero");
                    auto *rt = module_->type_of(inst.type_id);
                    id_map_[inst.result_id] = zero_for_type(rt, *module_);
                    break;
                }
                const ImageData &img = image_staging_.at(it->second);
                float u = 0.0f, v = 0.0f;
                if (coord.kind == Value::Kind::Composite && coord.elements.size() >= 2) {
                    u = coord.elements[0].scalar.f32;
                    v = coord.elements[1].scalar.f32;
                } else if (coord.kind == Value::Kind::Float32) {
                    u = coord.scalar.f32;
                }
                float rgba[4];
                img.sample_bilinear(u, v, rgba);
                id_map_[inst.result_id] = Value::make_composite({Value::make_f32(rgba[0]),
                                                                 Value::make_f32(rgba[1]),
                                                                 Value::make_f32(rgba[2]),
                                                                 Value::make_f32(rgba[3])});
                break;
            }

        case SpvOpImageFetch:
        case SpvOpImageRead:
            {
                // %result = OpImageFetch/Read %ResType %image %coord [ImageOperands]
                Value img_handle = get(0);
                Value coord = get(1);
                uint32_t var_id = img_handle.scalar.u32;

                auto it = image_var_map_.find(var_id);
                if (it == image_var_map_.end()) {
                    diagnostics.push_back("OpImageFetch/Read: no image bound at variable %"
                                          + std::to_string(var_id) + "; returning zero");
                    auto *rt = module_->type_of(inst.type_id);
                    id_map_[inst.result_id] = zero_for_type(rt, *module_);
                    break;
                }
                const ImageData &img = image_staging_.at(it->second);
                int x = 0, y = 0;
                if (coord.kind == Value::Kind::Composite && coord.elements.size() >= 2) {
                    const Value &cx = coord.elements[0];
                    const Value &cy = coord.elements[1];
                    x = cx.kind == Value::Kind::Int32  ? cx.scalar.i32
                      : cx.kind == Value::Kind::UInt32 ? static_cast<int>(cx.scalar.u32)
                                                       : 0;
                    y = cy.kind == Value::Kind::Int32  ? cy.scalar.i32
                      : cy.kind == Value::Kind::UInt32 ? static_cast<int>(cy.scalar.u32)
                                                       : 0;
                }
                float rgba[4];
                img.fetch_texel(x, y, rgba);
                id_map_[inst.result_id] = Value::make_composite({Value::make_f32(rgba[0]),
                                                                 Value::make_f32(rgba[1]),
                                                                 Value::make_f32(rgba[2]),
                                                                 Value::make_f32(rgba[3])});
                break;
            }

        case SpvOpImageWrite:
            {
                // OpImageWrite %image %coord %texel [ImageOperands] — no result id.
                // Not needed for debugging read-only inputs; silently ignore.
                break;
            }

        case SpvOpImageQuerySizeLod:
        case SpvOpImageQuerySize:
            {
                // %result = OpImageQuerySizeLod %ResType %image [%lod]
                Value img_handle = get(0);
                uint32_t var_id = img_handle.scalar.u32;
                int32_t w = 1, h = 1;
                auto it = image_var_map_.find(var_id);
                if (it != image_var_map_.end()) {
                    const ImageData &img = image_staging_.at(it->second);
                    w = img.width;
                    h = img.height;
                }
                id_map_[inst.result_id] = Value::make_composite(
                    {Value::make_u32(static_cast<uint32_t>(w)),
                     Value::make_u32(static_cast<uint32_t>(h))});
                break;
            }

        case SpvOpImageQueryLevels:
        case SpvOpImageQuerySamples:
            // Return 1 mip level / 1 sample (no mip chain or MSAA in our model).
            id_map_[inst.result_id] = Value::make_u32(1);
            break;

        default:
            diagnostics.push_back("Unimplemented image opcode " + std::to_string(inst.opcode)
                                  + "; returning zero");
            if (inst.result_id) {
                auto *rt = module_->type_of(inst.type_id);
                id_map_[inst.result_id] = zero_for_type(rt, *module_);
            }
    }
    advance_pc();
    return StopReason::Step;
}

// ---- derivative instructions -----------------------------------------------

// Return a zero value of the same scalar/vector shape as `v`.
static Value zero_like(const Value &v)
{
    if (v.kind == Value::Kind::Composite) {
        std::vector<Value> elems;
        elems.reserve(v.elements.size());
        for (const auto &e : v.elements) elems.push_back(zero_like(e));
        return Value::make_composite(std::move(elems));
    }
    return v.kind == Value::Kind::Float64 ? Value::make_f64(0.0) : Value::make_f32(0.0f);
}

StopReason Interpreter::exec_derivative(const Instruction &inst)
{
    // Without a 2×2 helper quad, all partial derivatives are undefined.
    // We return zero (correct for Compute; approximation for Fragment).
    diagnostics.push_back(
        "derivative op at %" + std::to_string(inst.result_id)
        + " returns zero (single-invocation mode; quad execution not yet implemented)");
    Value p = lookup(inst.operand(0));
    if (inst.result_id)
        id_map_[inst.result_id] = zero_like(p);
    advance_pc();
    return StopReason::Step;
}

// ---- extension instructions ------------------------------------------------

StopReason Interpreter::exec_ext_inst(const Instruction &inst)
{
    // words: [type_id, result_id, ext_set_id, instruction_id, operand_ids...]
    uint32_t ext_set_id = inst.operand(0);
    uint32_t ext_inst = inst.operand(1);

    auto eit = module_->ext_imports.find(ext_set_id);
    if (eit == module_->ext_imports.end()) {
        diagnostics.push_back("OpExtInst: unknown ext set id " + std::to_string(ext_set_id));
        if (inst.result_id)
            id_map_[inst.result_id] = Value::make_u32(0);
        advance_pc();
        return StopReason::Step;
    }

    if (eit->second == "GLSL.std.450") {
        // Frexp (51) writes the exponent through a pointer operand; handle before
        // the general dispatch which cannot perform pointer stores.
        if (ext_inst == 51 /* Frexp */ && inst.operand_count() >= 4) {
            Value x = lookup(inst.operand(2));
            Value exp_ptr = lookup(inst.operand(3));
            auto do_frexp = [](const Value &v, Value &exp_out) -> Value
            {
                int e;
                if (v.kind == Value::Kind::Float32) {
                    float sig = std::frexp(v.scalar.f32, &e);
                    exp_out = Value::make_i32(e);
                    return Value::make_f32(sig);
                }
                double sig = std::frexp(v.scalar.f64, &e);
                exp_out = Value::make_i32(e);
                return Value::make_f64(sig);
            };
            Value sig, exp_val;
            if (x.kind == Value::Kind::Composite) {
                std::vector<Value> sigs, exps;
                for (auto &e : x.elements) {
                    Value ev;
                    sigs.push_back(do_frexp(e, ev));
                    exps.push_back(std::move(ev));
                }
                sig = Value::make_composite(std::move(sigs));
                exp_val = Value::make_composite(std::move(exps));
            } else {
                sig = do_frexp(x, exp_val);
            }
            auto sr = store_ptr(exp_ptr, std::move(exp_val));
            if (!sr)
                diagnostics.push_back("Frexp: " + sr.error().message);
            if (inst.result_id)
                id_map_[inst.result_id] = std::move(sig);
            advance_pc();
            return StopReason::Step;
        }

        std::vector<Value> args;
        for (uint32_t i = 2; i < inst.operand_count(); ++i) args.push_back(lookup(inst.operand(i)));
        Value result = dispatch_glsl_std_450(ext_inst, args, diagnostics);
        if (inst.result_id)
            id_map_[inst.result_id] = std::move(result);
    } else if (eit->second.rfind("NonSemantic.", 0) == 0) {
        if (eit->second == "NonSemantic.Shader.DebugInfo.100") {
            if (ext_inst == 23 /* DebugScope */ && inst.operand_count() >= 3) {
                // operand(2) = scope (DebugFunction result-id)
                // operand(3) = optional DebugInlinedAt result-id
                current_scope_id_ = inst.operand(2);
                current_inlined_at_id_ = inst.operand_count() >= 4 ? inst.operand(3) : 0;
            } else if (ext_inst == 24 /* DebugNoScope */) {
                current_scope_id_ = 0;
                current_inlined_at_id_ = 0;
            } else if (ext_inst == 29 /* DebugValue */ && inst.operand_count() >= 5) {
                // operand(2) = DebugLocalVariable result-id
                // operand(3) = current SSA value result-id
                // operand(4) = DebugExpression (ignored)
                // operand(5+) = optional member-index path (e.g. struct field or
                //               vector component).  When present, only that member
                //               of the composite should be updated.
                uint32_t debug_var_id = inst.operand(2);
                uint32_t value_id = inst.operand(3);
                Value new_val = lookup(value_id);

                if (inst.operand_count() > 5) {
                    // Partial update: navigate the index path, growing composite
                    // elements as needed, then assign only the targeted element.
                    // Note: operand(5+) are SPIR-V result IDs, not literal integers;
                    // they must be resolved via lookup() to obtain their numeric value.
                    Value &root = debug_var_values_[debug_var_id];
                    if (root.kind != Value::Kind::Composite)
                        root = Value::make_composite({});
                    Value *target = &root;
                    for (uint32_t op_i = 5; op_i < inst.operand_count(); ++op_i) {
                        Value idx_val = lookup(inst.operand(op_i));
                        uint32_t idx = 0;
                        if (idx_val.kind == Value::Kind::Int32)
                            idx = static_cast<uint32_t>(idx_val.scalar.i32);
                        else if (idx_val.kind == Value::Kind::UInt32)
                            idx = idx_val.scalar.u32;
                        if (target->kind != Value::Kind::Composite) {
                            target->elements.clear();
                            target->kind = Value::Kind::Composite;
                        }
                        if (idx >= target->elements.size())
                            target->elements.resize(idx + 1);
                        if (op_i == inst.operand_count() - 1) {
                            target->elements[idx] = std::move(new_val);
                        } else {
                            target = &target->elements[idx];
                        }
                    }
                } else {
                    // Full assignment (no member indices).
                    debug_var_values_[debug_var_id] = std::move(new_val);
                }
            }
        }
        if (inst.result_id)
            id_map_[inst.result_id] = Value::make_u32(0);
    } else {
        diagnostics.push_back("OpExtInst: unsupported extension set " + eit->second);
        if (inst.result_id)
            id_map_[inst.result_id] = Value::make_u32(0);
    }
    advance_pc();
    return StopReason::Step;
}

// ---- top-level dispatch ----------------------------------------------------

StopReason Interpreter::execute_one(const Instruction &inst)
{
    // Opcodes with no side effects and no result that we can skip.
    switch (inst.opcode) {
        case SpvOpNop:
        case SpvOpLine:
        case SpvOpNoLine:
        case SpvOpModuleProcessed: advance_pc(); return StopReason::Step;

        case SpvOpUndef:
            if (inst.result_id)
                id_map_[inst.result_id] = Value::make_u32(0);
            advance_pc();
            return StopReason::Step;

        // Arithmetic & logic
        case SpvOpIAdd:
        case SpvOpISub:
        case SpvOpIMul:
        case SpvOpUDiv:
        case SpvOpSDiv:
        case SpvOpUMod:
        case SpvOpSRem:
        case SpvOpSMod:
        case SpvOpFAdd:
        case SpvOpFSub:
        case SpvOpFMul:
        case SpvOpFDiv:
        case SpvOpFMod:
        case SpvOpFRem:
        case SpvOpSNegate:
        case SpvOpFNegate:
        case SpvOpVectorTimesScalar:
        case SpvOpMatrixTimesScalar:
        case SpvOpDot:
        case SpvOpShiftLeftLogical:
        case SpvOpShiftRightLogical:
        case SpvOpShiftRightArithmetic:
        case SpvOpBitwiseAnd:
        case SpvOpBitwiseOr:
        case SpvOpBitwiseXor:
        case SpvOpNot:
        case SpvOpLogicalAnd:
        case SpvOpLogicalOr:
        case SpvOpLogicalNot:
        case SpvOpLogicalEqual:
        case SpvOpLogicalNotEqual:
        case SpvOpAny:
        case SpvOpAll:
        case SpvOpIEqual:
        case SpvOpINotEqual:
        case SpvOpULessThan:
        case SpvOpULessThanEqual:
        case SpvOpUGreaterThan:
        case SpvOpUGreaterThanEqual:
        case SpvOpSLessThan:
        case SpvOpSLessThanEqual:
        case SpvOpSGreaterThan:
        case SpvOpSGreaterThanEqual:
        case SpvOpFOrdEqual:
        case SpvOpFUnordEqual:
        case SpvOpFOrdNotEqual:
        case SpvOpFUnordNotEqual:
        case SpvOpFOrdLessThan:
        case SpvOpFOrdLessThanEqual:
        case SpvOpFOrdGreaterThan:
        case SpvOpFOrdGreaterThanEqual:
        case SpvOpSelect:
        case SpvOpIsNan:
        case SpvOpIsInf: return exec_arith_logic(inst);

        // Conversions
        case SpvOpConvertSToF:
        case SpvOpConvertUToF:
        case SpvOpConvertFToS:
        case SpvOpConvertFToU:
        case SpvOpFConvert:
        case SpvOpSConvert:
        case SpvOpUConvert:
        case SpvOpBitcast: return exec_convert(inst);

        // Memory
        case SpvOpVariable:
        case SpvOpLoad:
        case SpvOpStore:
        case SpvOpAccessChain:
        case SpvOpInBoundsAccessChain:
        case SpvOpCopyObject: return exec_memory(inst);

        // Composite
        case SpvOpCompositeConstruct:
        case SpvOpCompositeExtract:
        case SpvOpCompositeInsert:
        case SpvOpVectorExtractDynamic:
        case SpvOpVectorInsertDynamic:
        case SpvOpVectorShuffle:
        case SpvOpTranspose: return exec_composite(inst);

        // Control flow
        case SpvOpBranch:
        case SpvOpBranchConditional:
        case SpvOpSwitch:
        case SpvOpPhi:
        case SpvOpReturn:
        case SpvOpReturnValue:
        case SpvOpUnreachable:
        case SpvOpFunctionCall:
        case SpvOpSelectionMerge:
        case SpvOpLoopMerge: return exec_control_flow(inst);

        // Image instructions
        case SpvOpSampledImage:
        case SpvOpImage:
        case SpvOpImageSampleImplicitLod:
        case SpvOpImageSampleExplicitLod:
        case SpvOpImageSampleDrefImplicitLod:
        case SpvOpImageSampleDrefExplicitLod:
        case SpvOpImageSampleProjImplicitLod:
        case SpvOpImageSampleProjExplicitLod:
        case SpvOpImageSampleProjDrefImplicitLod:
        case SpvOpImageSampleProjDrefExplicitLod:
        case SpvOpImageFetch:
        case SpvOpImageGather:
        case SpvOpImageDrefGather:
        case SpvOpImageRead:
        case SpvOpImageWrite:
        case SpvOpImageQuerySizeLod:
        case SpvOpImageQuerySize:
        case SpvOpImageQueryLevels:
        case SpvOpImageQuerySamples: return exec_image(inst);

        // Derivative instructions (return zero in single-invocation mode)
        case SpvOpDPdx:
        case SpvOpDPdy:
        case SpvOpFwidth:
        case SpvOpDPdxFine:
        case SpvOpDPdyFine:
        case SpvOpFwidthFine:
        case SpvOpDPdxCoarse:
        case SpvOpDPdyCoarse:
        case SpvOpFwidthCoarse: return exec_derivative(inst);

        // Extension instructions
        case SpvOpExtInst: return exec_ext_inst(inst);

        default:
            diagnostics.push_back("Unimplemented opcode " + std::to_string(inst.opcode)
                                  + " at result-id " + std::to_string(inst.result_id));
            if (inst.result_id)
                id_map_[inst.result_id] = Value::make_u32(0);
            advance_pc();
            return StopReason::Step;
    }
}

// ---- inspection ------------------------------------------------------------

std::vector<std::pair<std::string, Value>> Interpreter::local_variables() const
{
    std::vector<std::pair<std::string, Value>> result;
    // Walk through all memory entries for Function storage class.
    for (auto &[id, val] : memory_) {
        auto vit = module_->variables.find(id);
        if (vit == module_->variables.end())
            continue;
        if (vit->second.storage_class != SpvStorageClassFunction)
            continue;
        result.emplace_back(vit->second.name.empty() ? "%" + std::to_string(id) : vit->second.name,
                            val);
    }
    // Also include SSA debug variable values from DebugValue instructions.
    for (auto &[debug_var_id, val] : debug_var_values_) {
        auto dit = module_->debug_local_vars.find(debug_var_id);
        std::string name = (dit != module_->debug_local_vars.end() && !dit->second.empty())
                             ? dit->second
                             : "%" + std::to_string(debug_var_id);
        result.emplace_back(std::move(name), val);
    }
    std::sort(result.begin(),
              result.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
    return result;
}

std::vector<std::pair<std::string, Value>> Interpreter::output_variables() const
{
    std::vector<std::pair<std::string, Value>> result;
    for (auto &[id, var] : module_->variables) {
        if (var.storage_class != SpvStorageClassOutput
            && var.storage_class != SpvStorageClassStorageBuffer)
            continue;
        auto it = memory_.find(id);
        if (it == memory_.end())
            continue;
        result.emplace_back(var.name.empty() ? "%" + std::to_string(id) : var.name, it->second);
    }
    return result;
}

Result<Value> Interpreter::read_descriptor(uint32_t set, uint32_t binding) const
{
    for (auto &[id, var] : module_->variables) {
        auto &dset = module_->decorations_of(id);
        if (dset.get(SpvDecorationDescriptorSet) == set
            && dset.get(SpvDecorationBinding) == binding)
        {
            auto it = memory_.find(id);
            if (it == memory_.end())
                return Result<Value>::err("Descriptor not in memory");
            return it->second;
        }
    }
    return Result<Value>::err("No descriptor at set=" + std::to_string(set)
                              + " binding=" + std::to_string(binding));
}

// ---- debug info helpers ----------------------------------------------------

SourceLoc Interpreter::current_source_location() const
{
    if (finished_ || !pc_.valid())
        return {};
    auto fit = module_->functions.find(pc_.function_id);
    if (fit == module_->functions.end())
        return {};
    const Function &fn = fit->second;
    auto bit = fn.block_index.find(pc_.block_label);
    if (bit == fn.block_index.end())
        return {};
    const BasicBlock &bb = fn.blocks[bit->second];
    if (pc_.instr_index < bb.source_locs.size())
        return bb.source_locs[pc_.instr_index];
    return {};
}

bool Interpreter::check_breakpoint_at_pc() const
{
    if (breakpoints_.empty() || finished_ || panicked_)
        return false;
    auto fit = module_->functions.find(pc_.function_id);
    if (fit == module_->functions.end())
        return false;
    const Function &fn = fit->second;
    auto bit = fn.block_index.find(pc_.block_label);
    if (bit == fn.block_index.end())
        return false;
    const BasicBlock &bb = fn.blocks[bit->second];
    if (pc_.instr_index >= bb.instructions.size())
        return false;

    const Instruction &inst = bb.instructions[pc_.instr_index];

    // Don't fire source breakpoints on debug metadata instructions (OpLine,
    // OpNoLine, NonSemantic ExtInst).  The breakpoint should fire on the first
    // *real* instruction of the target line so that DebugValue assignments and
    // DebugScope updates that precede it on the same line have already executed
    // and local variable state is up to date.
    bool is_metadata = false;
    if (inst.opcode == SpvOpLine || inst.opcode == SpvOpNoLine) {
        is_metadata = true;
    } else if (inst.opcode == SpvOpExtInst && inst.operand_count() >= 1) {
        uint32_t ext_set_id = inst.operand(0);
        auto eit = module_->ext_imports.find(ext_set_id);
        if (eit != module_->ext_imports.end() && eit->second.rfind("NonSemantic.", 0) == 0)
            is_metadata = true;
    }

    SourceLoc loc = (pc_.instr_index < bb.source_locs.size()) ? bb.source_locs[pc_.instr_index]
                                                              : SourceLoc {};

    for (const auto &bp : breakpoints_) {
        if (!bp.active)
            continue;
        // Result-id breakpoints always fire regardless of instruction type.
        if (bp.result_id != 0 && inst.result_id == bp.result_id)
            return true;
        // Source breakpoints skip metadata instructions.
        if (is_metadata)
            continue;
        // Source breakpoint: match file suffix + line.
        if (!bp.file.empty() && bp.line != 0 && loc.line == bp.line) {
            // Accept if the recorded file ends with the requested file path
            // (allows matching "foo.hlsl" against "/full/path/to/foo.hlsl").
            if (loc.file == bp.file
                || (loc.file.size() >= bp.file.size()
                    && loc.file.compare(loc.file.size() - bp.file.size(), bp.file.size(), bp.file)
                           == 0))
                return true;
        }
    }
    return false;
}

StopReason Interpreter::step_source_line()
{
    SourceLoc start_loc = current_source_location();
    while (!finished_ && !panicked_) {
        auto r = step_instruction();
        // Always propagate fatal results.
        if (r == StopReason::Panic || r == StopReason::EntryFinished)
            return r;
        SourceLoc new_loc = current_source_location();
        // If we didn't have a starting location, any instruction advance suffices.
        if (!start_loc.valid())
            return r;
        // Keep stepping through instructions that have no debug location (line==0).
        // These arise in branch successor blocks that lack a DebugLine, e.g. the
        // false-branch path after a skipped if-body.  Stopping at such an
        // instruction would report ":0" instead of the next real source line.
        if (!new_loc.valid())
            continue;
        // Only stop (for Step or Breakpoint) once we've moved to a different line.
        if (new_loc.line != start_loc.line || new_loc.file != start_loc.file)
            return r;
        // Still on the starting line — suppress Breakpoint and keep stepping.
    }
    return panicked_ ? StopReason::Panic : StopReason::EntryFinished;
}

StopReason Interpreter::step_over_source_line()
{
    SourceLoc start_loc = current_source_location();
    size_t start_depth = call_stack_.size();
    while (!finished_ && !panicked_) {
        auto r = step_instruction();
        if (r == StopReason::Panic || r == StopReason::EntryFinished)
            return r;
        // Don't evaluate the source location if we've descended into a callee.
        if (call_stack_.size() > start_depth)
            continue;
        SourceLoc new_loc = current_source_location();
        if (!start_loc.valid())
            return r;
        // Keep stepping through instructions that have no debug location (line==0).
        if (!new_loc.valid())
            continue;
        if (new_loc.line != start_loc.line || new_loc.file != start_loc.file)
            return r;
    }
    return panicked_ ? StopReason::Panic : StopReason::EntryFinished;
}

uint32_t Interpreter::add_breakpoint(std::string file, uint32_t line)
{
    Breakpoint bp;
    bp.id = next_bp_id_++;
    bp.file = std::move(file);
    bp.line = line;
    breakpoints_.push_back(bp);
    return bp.id;
}

uint32_t Interpreter::add_breakpoint_at_id(uint32_t result_id)
{
    Breakpoint bp;
    bp.id = next_bp_id_++;
    bp.result_id = result_id;
    breakpoints_.push_back(bp);
    return bp.id;
}

void Interpreter::remove_breakpoint(uint32_t bp_id)
{
    breakpoints_.erase(std::remove_if(breakpoints_.begin(),
                                      breakpoints_.end(),
                                      [bp_id](const Breakpoint &b) { return b.id == bp_id; }),
                       breakpoints_.end());
}

} // namespace spvdb
