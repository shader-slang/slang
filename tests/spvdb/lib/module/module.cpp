#include "module.h"
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace spvdb
{

// ---- helpers ---------------------------------------------------------------

static std::string extract_string(const std::vector<uint32_t> &words, uint32_t start = 0)
{
    std::string s;
    for (size_t i = start; i < words.size(); ++i) {
        uint32_t w = words[i];
        for (int b = 0; b < 4; ++b) {
            char c = static_cast<char>((w >> (b * 8)) & 0xFF);
            if (c == '\0')
                return s;
            s += c;
        }
    }
    return s;
}

// ---- module builder --------------------------------------------------------

struct ModuleBuilder
{
    std::shared_ptr<SpvModule> m;

    explicit ModuleBuilder()
        : m(std::make_shared<SpvModule>())
    {
    }

    // ---- type helpers -------------------------------------------------------

    Value make_zero_value(uint32_t type_id)
    {
        auto *t = m->type_of(type_id);
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
                    // Find element type_id (search types map for the element type ptr)
                    uint32_t elem_tid = 0;
                    for (auto &[id, tp] : m->types) {
                        if (tp.get() == vt->element_type.get()) {
                            elem_tid = id;
                            break;
                        }
                    }
                    std::vector<Value> elems(vt->count, make_zero_value(elem_tid));
                    return Value::make_composite(std::move(elems));
                }
            case SpvType::Kind::Matrix:
                {
                    auto *mt = static_cast<const MatrixType *>(t);
                    uint32_t col_tid = 0;
                    for (auto &[id, tp] : m->types) {
                        if (tp.get() == mt->column_type.get()) {
                            col_tid = id;
                            break;
                        }
                    }
                    std::vector<Value> cols(mt->columns, make_zero_value(col_tid));
                    return Value::make_composite(std::move(cols));
                }
            case SpvType::Kind::Array:
                {
                    auto *at = static_cast<const ArrayType *>(t);
                    uint32_t elem_tid = 0;
                    for (auto &[id, tp] : m->types) {
                        if (tp.get() == at->element_type.get()) {
                            elem_tid = id;
                            break;
                        }
                    }
                    std::vector<Value> elems(at->length, make_zero_value(elem_tid));
                    return Value::make_composite(std::move(elems));
                }
            case SpvType::Kind::Struct:
                {
                    auto *st = static_cast<const StructType *>(t);
                    std::vector<Value> members;
                    for (auto &mem : st->members) {
                        // Find member type_id
                        uint32_t mem_tid = 0;
                        for (auto &[id, tp] : m->types) {
                            if (tp.get() == mem.type.get()) {
                                mem_tid = id;
                                break;
                            }
                        }
                        members.push_back(make_zero_value(mem_tid));
                    }
                    return Value::make_composite(std::move(members));
                }
            default: return Value::make_u32(0);
        }
    }

    // ---- constant evaluation ------------------------------------------------

    Value eval_constant(const Instruction &inst)
    {
        switch (inst.opcode) {
            case SpvOpConstantTrue: return Value::make_bool(true);
            case SpvOpConstantFalse: return Value::make_bool(false);
            case SpvOpConstantNull: return make_zero_value(inst.type_id);
            case SpvOpConstant:
                {
                    auto *t = m->type_of(inst.type_id);
                    if (!t)
                        return Value::make_u32(0);
                    if (t->kind == SpvType::Kind::Int) {
                        auto *it = static_cast<const IntType *>(t);
                        if (it->width <= 32) {
                            uint32_t raw = inst.words.size() > 2 ? inst.words[2] : 0;
                            return it->is_signed ? Value::make_i32(static_cast<int32_t>(raw))
                                                 : Value::make_u32(raw);
                        } else {
                            uint32_t lo = inst.words.size() > 2 ? inst.words[2] : 0;
                            uint32_t hi = inst.words.size() > 3 ? inst.words[3] : 0;
                            uint64_t raw = (static_cast<uint64_t>(hi) << 32) | lo;
                            return it->is_signed ? Value::make_i64(static_cast<int64_t>(raw))
                                                 : Value::make_u64(raw);
                        }
                    } else if (t->kind == SpvType::Kind::Float) {
                        auto *ft = static_cast<const FloatType *>(t);
                        if (ft->width <= 32) {
                            uint32_t raw = inst.words.size() > 2 ? inst.words[2] : 0;
                            float f;
                            std::memcpy(&f, &raw, sizeof(f));
                            return Value::make_f32(f);
                        } else {
                            uint32_t lo = inst.words.size() > 2 ? inst.words[2] : 0;
                            uint32_t hi = inst.words.size() > 3 ? inst.words[3] : 0;
                            uint64_t raw = (static_cast<uint64_t>(hi) << 32) | lo;
                            double d;
                            std::memcpy(&d, &raw, sizeof(d));
                            return Value::make_f64(d);
                        }
                    }
                    return Value::make_u32(0);
                }
            case SpvOpConstantComposite:
                {
                    // words: [type_id, result_id, constituent_ids...]
                    std::vector<Value> elems;
                    for (uint32_t i = 2; i < inst.words.size(); ++i) {
                        uint32_t cid = inst.words[i];
                        auto it = m->constants.find(cid);
                        elems.push_back(it != m->constants.end() ? it->second : Value::make_u32(0));
                    }
                    return Value::make_composite(std::move(elems));
                }
            default: return Value::make_u32(0);
        }
    }

    // ---- dispatch -----------------------------------------------------------

    Result<void> process(const Instruction &inst)
    {
        switch (inst.opcode) {
            // --- Capabilities / extensions (informational) ---
            case SpvOpCapability:
                m->capabilities.push_back(static_cast<SpvCapability>(inst.words[0]));
                break;

            case SpvOpExtInstImport:
                // words: [result_id, string...]
                m->ext_imports[inst.result_id] = extract_string(inst.words, 1);
                break;

            // --- Entry points ---
            case SpvOpEntryPoint:
                {
                    // words: [execution_model, function_id, name..., interface_ids...]
                    EntryPoint ep;
                    ep.execution_model = static_cast<SpvExecutionModel>(inst.words[0]);
                    ep.function_id = inst.words[1];

                    // Name is a null-terminated string starting at words[2].
                    uint32_t name_start = 2;
                    std::string name;
                    size_t consumed = 0;
                    for (size_t i = name_start; i < inst.words.size(); ++i) {
                        uint32_t w = inst.words[i];
                        bool done = false;
                        for (int b = 0; b < 4; ++b) {
                            char c = static_cast<char>((w >> (b * 8)) & 0xFF);
                            if (c == '\0') {
                                done = true;
                                consumed = i - name_start + 1;
                                break;
                            }
                            name += c;
                        }
                        if (done)
                            break;
                    }
                    ep.name = name;

                    // Remaining words are interface variable ids.
                    for (size_t i = name_start + consumed; i < inst.words.size(); ++i)
                        ep.interface_ids.push_back(inst.words[i]);

                    m->entry_points.push_back(std::move(ep));
                    break;
                }

            // --- String literals (used by debug info) ---
            case SpvOpString:
                // words: [result_id, string...]
                m->strings[inst.result_id] = extract_string(inst.words, 1);
                break;

            // --- Global NonSemantic debug info (DebugSource etc.) ---
            case SpvOpExtInst:
                {
                    // words: [type_id, result_id, ext_set_id, inst_id, operands...]
                    if (inst.words.size() >= 5) {
                        uint32_t ext_set = inst.words[2];
                        uint32_t debug_op = inst.words[3];
                        auto eit = m->ext_imports.find(ext_set);
                        if (eit != m->ext_imports.end()
                            && eit->second == "NonSemantic.Shader.DebugInfo.100")
                        {
                            if (debug_op == 35 /* DebugSource */) {
                                // operand[0] = file OpString result-id
                                uint32_t str_id = inst.words[4];
                                auto sit = m->strings.find(str_id);
                                if (sit != m->strings.end())
                                    m->debug_sources[inst.result_id] = sit->second;
                            } else if (debug_op == 20 /* DebugFunction */) {
                                // words[4]=name_string_id, words[5]=type_id,
                                // words[6]=source_id, words[7]=line_constant_id, ...
                                if (inst.words.size() >= 7) {
                                    SpvModule::DebugFunctionInfo dfi;
                                    auto sit = m->strings.find(inst.words[4]);
                                    if (sit != m->strings.end())
                                        dfi.name = sit->second;
                                    dfi.source_id = inst.words[6];
                                    m->debug_functions[inst.result_id] = std::move(dfi);
                                }
                            } else if (debug_op == 25 /* DebugInlinedAt */) {
                                // words[4]=line_constant_id, words[5]=scope_id,
                                // words[6]=optional parent DebugInlinedAt
                                if (inst.words.size() >= 6) {
                                    SpvModule::DebugInlinedAtInfo dia;
                                    uint32_t line_cid = inst.words[4];
                                    auto lit = m->constants.find(line_cid);
                                    if (lit != m->constants.end()) {
                                        if (lit->second.kind == Value::Kind::UInt32)
                                            dia.line = lit->second.scalar.u32;
                                        else if (lit->second.kind == Value::Kind::Int32)
                                            dia.line = static_cast<uint32_t>(
                                                lit->second.scalar.i32);
                                    }
                                    dia.scope_id = inst.words[5];
                                    dia.inlined_at_id = inst.words.size() >= 7 ? inst.words[6] : 0;
                                    m->debug_inlined_at[inst.result_id] = std::move(dia);
                                }
                            } else if (debug_op == 26 /* DebugLocalVariable */) {
                                // words[4] = name OpString result-id
                                if (inst.words.size() >= 5) {
                                    uint32_t str_id = inst.words[4];
                                    auto sit = m->strings.find(str_id);
                                    if (sit != m->strings.end())
                                        m->debug_local_vars[inst.result_id] = sit->second;
                                }
                            }
                        }
                    }
                    break;
                }

            // --- Names ---
            case SpvOpName:
                // words: [target_id, name...]
                m->names[inst.words[0]] = extract_string(inst.words, 1);
                break;

            case SpvOpMemberName:
                {
                    // words: [type_id, member_index, name...]
                    uint32_t tid = inst.words[0];
                    uint32_t midx = inst.words[1];
                    uint64_t key = (static_cast<uint64_t>(tid) << 32) | midx;
                    m->member_names[key] = extract_string(inst.words, 2);
                    break;
                }

            // --- Decorations ---
            case SpvOpDecorate:
                {
                    // words: [target_id, decoration, operands...]
                    uint32_t target = inst.words[0];
                    Decoration d;
                    d.kind = static_cast<SpvDecoration>(inst.words[1]);
                    for (size_t i = 2; i < inst.words.size(); ++i)
                        d.operands.push_back(inst.words[i]);
                    m->decorations[target].decorations.push_back(std::move(d));
                    break;
                }

            case SpvOpMemberDecorate:
                {
                    // words: [structure_type_id, member_index, decoration, operands...]
                    uint32_t tid = inst.words[0];
                    uint32_t midx = inst.words[1];
                    uint64_t key = (static_cast<uint64_t>(tid) << 32) | midx;
                    Decoration d;
                    d.kind = static_cast<SpvDecoration>(inst.words[2]);
                    for (size_t i = 3; i < inst.words.size(); ++i)
                        d.operands.push_back(inst.words[i]);
                    m->member_decorations[key].decorations.push_back(std::move(d));
                    break;
                }

            // --- Types ---
            case SpvOpTypeVoid: m->types[inst.result_id] = std::make_shared<VoidType>(); break;

            case SpvOpTypeBool: m->types[inst.result_id] = std::make_shared<BoolType>(); break;

            case SpvOpTypeInt:
                {
                    // words: [result_id, width, signedness]
                    uint32_t width = inst.words[1];
                    bool is_signed = inst.words[2] != 0;
                    m->types[inst.result_id] = std::make_shared<IntType>(width, is_signed);
                    break;
                }

            case SpvOpTypeFloat:
                {
                    // words: [result_id, width]
                    m->types[inst.result_id] = std::make_shared<FloatType>(inst.words[1]);
                    break;
                }

            case SpvOpTypeVector:
                {
                    // words: [result_id, component_type_id, count]
                    auto elem = m->types.count(inst.words[1]) ? m->types[inst.words[1]] : nullptr;
                    m->types[inst.result_id] = std::make_shared<VectorType>(elem, inst.words[2]);
                    break;
                }

            case SpvOpTypeMatrix:
                {
                    // words: [result_id, column_type_id, column_count]
                    auto col = m->types.count(inst.words[1]) ? m->types[inst.words[1]] : nullptr;
                    m->types[inst.result_id] = std::make_shared<MatrixType>(col, inst.words[2]);
                    break;
                }

            case SpvOpTypeArray:
                {
                    // words: [result_id, element_type_id, length_id]
                    auto elem = m->types.count(inst.words[1]) ? m->types[inst.words[1]] : nullptr;
                    uint32_t len = 0;
                    auto cit = m->constants.find(inst.words[2]);
                    if (cit != m->constants.end()) {
                        auto &cv = cit->second;
                        if (cv.kind == Value::Kind::UInt32)
                            len = cv.scalar.u32;
                        else if (cv.kind == Value::Kind::Int32)
                            len = static_cast<uint32_t>(cv.scalar.i32);
                    }
                    m->types[inst.result_id] = std::make_shared<ArrayType>(elem, len);
                    break;
                }

            case SpvOpTypeRuntimeArray:
                {
                    // words: [result_id, element_type_id]
                    auto elem = m->types.count(inst.words[1]) ? m->types[inst.words[1]] : nullptr;
                    m->types[inst.result_id] = std::make_shared<RuntimeArrayType>(elem);
                    break;
                }

            case SpvOpTypeStruct:
                {
                    // words: [result_id, member_type_ids...]
                    auto st = std::make_shared<StructType>();
                    st->name = m->name_of(inst.result_id);
                    for (uint32_t i = 1; i < inst.words.size(); ++i) {
                        StructMember mem;
                        uint32_t mt_id = inst.words[i];
                        mem.type = m->types.count(mt_id) ? m->types[mt_id] : nullptr;
                        mem.name = m->member_name_of(inst.result_id, i - 1);
                        st->members.push_back(std::move(mem));
                    }
                    m->types[inst.result_id] = st;
                    break;
                }

            case SpvOpTypePointer:
                {
                    // words: [result_id, storage_class, type_id]
                    uint32_t sc = inst.words[1];
                    auto pointee = m->types.count(inst.words[2]) ? m->types[inst.words[2]]
                                                                 : nullptr;
                    m->types[inst.result_id] = std::make_shared<PointerType>(sc, pointee);
                    break;
                }

            case SpvOpTypeFunction:
                {
                    // words: [result_id, return_type_id, param_type_ids...]
                    auto ft = std::make_shared<FunctionType>();
                    ft->return_type = m->types.count(inst.words[1]) ? m->types[inst.words[1]]
                                                                    : nullptr;
                    for (size_t i = 2; i < inst.words.size(); ++i)
                        ft->param_types.push_back(
                            m->types.count(inst.words[i]) ? m->types[inst.words[i]] : nullptr);
                    m->types[inst.result_id] = ft;
                    break;
                }

            case SpvOpTypeImage:
                {
                    // words: [result_id, sampled_type_id, dim, depth, arrayed, ms, sampled, image_format, ...]
                    auto img = std::make_shared<ImageType>();
                    img->sampled_type = m->types.count(inst.words[1]) ? m->types[inst.words[1]]
                                                                      : nullptr;
                    img->dim = inst.words[2];
                    img->depth = inst.words[3];
                    img->arrayed = inst.words[4];
                    img->ms = inst.words[5];
                    img->sampled = inst.words[6];
                    img->image_format = inst.words[7];
                    m->types[inst.result_id] = img;
                    break;
                }

            case SpvOpTypeSampler:
                m->types[inst.result_id] = std::make_shared<SamplerType>();
                break;

            case SpvOpTypeSampledImage:
                {
                    auto si = std::make_shared<SampledImageType>();
                    si->image_type = m->types.count(inst.words[1]) ? m->types[inst.words[1]]
                                                                   : nullptr;
                    m->types[inst.result_id] = si;
                    break;
                }

            // --- Constants ---
            case SpvOpConstantTrue:
            case SpvOpConstantFalse:
            case SpvOpConstantNull:
            case SpvOpConstant:
            case SpvOpConstantComposite: m->constants[inst.result_id] = eval_constant(inst); break;

            // --- Spec constants (stored unevaluated for now) ---
            case SpvOpSpecConstantTrue:
            case SpvOpSpecConstantFalse:
            case SpvOpSpecConstant:
            case SpvOpSpecConstantComposite:
            case SpvOpSpecConstantOp:
                {
                    SpecConstant sc;
                    sc.result_id = inst.result_id;
                    sc.type_id = inst.type_id;
                    if (inst.opcode == SpvOpSpecConstantTrue
                        || inst.opcode == SpvOpSpecConstantFalse)
                    {
                        sc.kind = SpecConstantKind::Scalar;
                        sc.scalar_words.push_back(inst.opcode == SpvOpSpecConstantTrue ? 1 : 0);
                    } else if (inst.opcode == SpvOpSpecConstant) {
                        sc.kind = SpecConstantKind::Scalar;
                        for (size_t i = 2; i < inst.words.size(); ++i)
                            sc.scalar_words.push_back(inst.words[i]);
                    } else if (inst.opcode == SpvOpSpecConstantComposite) {
                        sc.kind = SpecConstantKind::Composite;
                        for (size_t i = 2; i < inst.words.size(); ++i)
                            sc.constituent_ids.push_back(inst.words[i]);
                    } else { // OpSpecConstantOp
                        sc.kind = SpecConstantKind::Op;
                        sc.op_opcode = static_cast<SpvOp>(inst.words[2]);
                        for (size_t i = 3; i < inst.words.size(); ++i)
                            sc.op_operands.push_back(inst.words[i]);
                    }
                    m->spec_constants[inst.result_id] = std::move(sc);
                    // Also put a placeholder in constants (will be overridden at session creation).
                    m->constants[inst.result_id] = make_zero_value(inst.type_id);
                    break;
                }

            // --- Global variables ---
            case SpvOpVariable:
                {
                    // words: [type_id, result_id, storage_class, optional_initializer]
                    Variable var;
                    var.result_id = inst.result_id;
                    var.type_id = inst.type_id;
                    var.storage_class = inst.words[2];
                    if (inst.words.size() > 3)
                        var.initializer_id = inst.words[3];
                    var.name = m->name_of(inst.result_id);
                    var.decorations = m->decorations_of(inst.result_id);
                    m->variables[inst.result_id] = std::move(var);
                    break;
                }

            // --- Functions: delegate to function builder ---
            case SpvOpFunction:
            case SpvOpFunctionParameter:
            case SpvOpFunctionEnd:
            case SpvOpLabel:
                // Handled in the second pass below.
                break;

            // Everything else: ignore (OpMemoryModel, OpExecutionMode, etc.)
            default: break;
        }
        return Result<void> {};
    }
};

// ---- post-process: apply member decorations to struct types ---------------

static void apply_member_decorations(SpvModule &m)
{
    for (auto &[id, tp] : m.types) {
        if (tp->kind != SpvType::Kind::Struct)
            continue;
        auto *st = static_cast<StructType *>(tp.get());
        for (uint32_t mi = 0; mi < st->members.size(); ++mi) {
            auto &mem = st->members[mi];
            auto &dset = m.member_decorations_of(id, mi);
            mem.offset = dset.get(SpvDecorationOffset);
            mem.matrix_stride = dset.get(SpvDecorationMatrixStride);
            mem.row_major = dset.has(SpvDecorationRowMajor);
            // Also pick up ArrayStride for array element types (handled via type).
            if (mem.name.empty())
                mem.name = m.member_name_of(id, mi);
        }
        if (st->name.empty())
            st->name = m.name_of(id);
    }
    // Apply ArrayStride from decoration to array types.
    for (auto &[id, tp] : m.types) {
        auto &dset = m.decorations_of(id);
        if (tp->kind == SpvType::Kind::Array) {
            auto *at = static_cast<ArrayType *>(tp.get());
            if (at->array_stride == 0)
                at->array_stride = dset.get(SpvDecorationArrayStride);
        } else if (tp->kind == SpvType::Kind::RuntimeArray) {
            auto *rat = static_cast<RuntimeArrayType *>(tp.get());
            if (rat->array_stride == 0)
                rat->array_stride = dset.get(SpvDecorationArrayStride);
        }
    }
}

// ---- function builder second pass -----------------------------------------

static void build_functions(SpvModule &m, const ParsedModule &parsed)
{
    Function *current_fn = nullptr;
    BasicBlock *current_bb = nullptr;

    for (auto &inst : parsed.instructions) {
        switch (inst.opcode) {
            case SpvOpFunction:
                {
                    // words: [return_type_id, result_id, function_control, function_type_id]
                    Function fn;
                    fn.result_id = inst.result_id;
                    fn.return_type_id = inst.type_id; // type_id == return type
                    fn.function_type_id = inst.words[3];
                    fn.name = m.name_of(fn.result_id);
                    m.functions[fn.result_id] = std::move(fn);
                    m.function_order.push_back(inst.result_id);
                    current_fn = &m.functions[inst.result_id];
                    current_bb = nullptr;
                    break;
                }
            case SpvOpFunctionParameter:
                {
                    if (current_fn) {
                        // words: [type_id, result_id]
                        current_fn->parameters.emplace_back(inst.result_id, inst.type_id);
                    }
                    break;
                }
            case SpvOpFunctionEnd:
                current_fn = nullptr;
                current_bb = nullptr;
                break;

            case SpvOpLabel:
                {
                    if (!current_fn)
                        break;
                    BasicBlock bb;
                    bb.label_id = inst.result_id;
                    current_fn->blocks.push_back(std::move(bb));
                    size_t idx = current_fn->blocks.size() - 1;
                    current_fn->block_index[inst.result_id] = idx;
                    current_bb = &current_fn->blocks[idx];
                    break;
                }

            default:
                if (current_bb) {
                    current_bb->instructions.push_back(inst);
                    // After storing, invalidate pointers if vector reallocated.
                    // We avoid this by not using raw pointers after any push_back —
                    // re-derive current_bb from function_order + blocks vector.
                    // Actually, we need to fix this: push_back can invalidate.
                    // Re-derive current_bb safely:
                    if (current_fn) {
                        if (!current_fn->blocks.empty())
                            current_bb = &current_fn->blocks.back();
                    }
                }
                break;
        }
    }
}

// ---- debug annotation third pass -------------------------------------------

static void build_debug_annotations(SpvModule &m)
{
    // Find the NonSemantic.Shader.DebugInfo.100 extension set id (if any).
    uint32_t nonsemantic_id = 0;
    for (auto &[id, name] : m.ext_imports) {
        if (name == "NonSemantic.Shader.DebugInfo.100") {
            nonsemantic_id = id;
            break;
        }
    }

    for (auto fn_id : m.function_order) {
        auto &fn = m.functions[fn_id];
        for (auto &bb : fn.blocks) {
            bb.source_locs.reserve(bb.instructions.size());
            SourceLoc current_loc;

            for (auto &inst : bb.instructions) {
                // OpLine: words = [file_string_id, line, column]
                if (inst.opcode == SpvOpLine && inst.words.size() >= 3) {
                    uint32_t str_id = inst.words[0];
                    uint32_t line = inst.words[1];
                    uint32_t col = inst.words[2];
                    auto sit = m.strings.find(str_id);
                    if (sit != m.strings.end()) {
                        current_loc.file = sit->second;
                        current_loc.line = line;
                        current_loc.column = col;
                    }
                } else if (inst.opcode == SpvOpNoLine) {
                    current_loc = {};
                } else if (inst.opcode == SpvOpExtInst && nonsemantic_id != 0
                           && inst.words.size() >= 4 && inst.words[2] == nonsemantic_id)
                {
                    uint32_t debug_op = inst.words[3];
                    if (debug_op == 103 /* DebugLine */ && inst.words.size() >= 9) {
                        // words: [type, result, set, 103, source_id,
                        //         line_start, line_end, col_start, col_end]
                        uint32_t source_id = inst.words[4];
                        uint32_t line_cid = inst.words[5];
                        uint32_t col_cid = inst.words[7];
                        auto lit = m.constants.find(line_cid);
                        auto cit = m.constants.find(col_cid);
                        auto get_u32 = [](const Value &v) -> uint32_t
                        {
                            if (v.kind == Value::Kind::UInt32)
                                return v.scalar.u32;
                            if (v.kind == Value::Kind::Int32)
                                return static_cast<uint32_t>(v.scalar.i32);
                            return 0;
                        };
                        uint32_t line_val = (lit != m.constants.end()) ? get_u32(lit->second) : 0;
                        uint32_t col_val = (cit != m.constants.end()) ? get_u32(cit->second) : 0;
                        auto dsit = m.debug_sources.find(source_id);
                        if (dsit != m.debug_sources.end() && line_val != 0) {
                            current_loc.file = dsit->second;
                            current_loc.line = line_val;
                            current_loc.column = col_val;
                        }
                    } else if (debug_op == 104 /* DebugNoLine */) {
                        current_loc = {};
                    }
                }
                bb.source_locs.push_back(current_loc);
            }
        }
    }
}

// ---- entry point -----------------------------------------------------------

Result<std::shared_ptr<SpvModule>> build_module(const ParsedModule &parsed)
{
    ModuleBuilder builder;
    builder.m->spv_version = parsed.version;
    builder.m->spv_generator = parsed.generator;
    builder.m->id_bound = parsed.id_bound;

    // First pass: types, constants, variables, decorations, names.
    for (auto &inst : parsed.instructions) {
        auto r = builder.process(inst);
        if (!r)
            return Result<std::shared_ptr<SpvModule>>::err(r.error().message);
    }

    // Apply member decorations to struct types (needs decorations to be loaded).
    apply_member_decorations(*builder.m);

    // Second pass: build functions + basic blocks.
    build_functions(*builder.m, parsed);

    // Third pass: annotate each instruction with its source location.
    build_debug_annotations(*builder.m);

    return builder.m;
}

} // namespace spvdb
