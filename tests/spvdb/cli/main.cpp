#include "../lib/api/spvdb.h"
#include "../lib/api/spvdb_session.h" // complete Session definition for unique_ptr
#include <cstdio>
#include <fstream>
#include <iostream>
#include <replxx.hxx>
#include <sstream>
#include <string>
#include <vector>

using namespace spvdb;
using Replxx = replxx::Replxx;

// ---- SPIR-V opcode name table -----------------------------------------------

static const char *spv_op_name(SpvOp op)
{
    switch (op) {
        case SpvOpNop: return "Nop";
        case SpvOpUndef: return "Undef";
        case SpvOpLine: return "Line";
        case SpvOpNoLine: return "NoLine";
        case SpvOpModuleProcessed: return "ModuleProcessed";
        case SpvOpExtInst: return "ExtInst";
        case SpvOpExtInstImport: return "ExtInstImport";
        case SpvOpTypeVoid: return "TypeVoid";
        case SpvOpTypeBool: return "TypeBool";
        case SpvOpTypeInt: return "TypeInt";
        case SpvOpTypeFloat: return "TypeFloat";
        case SpvOpTypeVector: return "TypeVector";
        case SpvOpTypeMatrix: return "TypeMatrix";
        case SpvOpTypeArray: return "TypeArray";
        case SpvOpTypeRuntimeArray: return "TypeRuntimeArray";
        case SpvOpTypeStruct: return "TypeStruct";
        case SpvOpTypePointer: return "TypePointer";
        case SpvOpTypeFunction: return "TypeFunction";
        case SpvOpTypeImage: return "TypeImage";
        case SpvOpTypeSampler: return "TypeSampler";
        case SpvOpTypeSampledImage: return "TypeSampledImage";
        case SpvOpConstant: return "Constant";
        case SpvOpConstantTrue: return "ConstantTrue";
        case SpvOpConstantFalse: return "ConstantFalse";
        case SpvOpConstantNull: return "ConstantNull";
        case SpvOpConstantComposite: return "ConstantComposite";
        case SpvOpSpecConstant: return "SpecConstant";
        case SpvOpSpecConstantTrue: return "SpecConstantTrue";
        case SpvOpSpecConstantFalse: return "SpecConstantFalse";
        case SpvOpSpecConstantComposite: return "SpecConstantComposite";
        case SpvOpSpecConstantOp: return "SpecConstantOp";
        case SpvOpVariable: return "Variable";
        case SpvOpLoad: return "Load";
        case SpvOpStore: return "Store";
        case SpvOpAccessChain: return "AccessChain";
        case SpvOpInBoundsAccessChain: return "InBoundsAccessChain";
        case SpvOpCopyObject: return "CopyObject";
        case SpvOpFunction: return "Function";
        case SpvOpFunctionParameter: return "FunctionParameter";
        case SpvOpFunctionCall: return "FunctionCall";
        case SpvOpFunctionEnd: return "FunctionEnd";
        case SpvOpLabel: return "Label";
        case SpvOpBranch: return "Branch";
        case SpvOpBranchConditional: return "BranchConditional";
        case SpvOpSwitch: return "Switch";
        case SpvOpReturn: return "Return";
        case SpvOpReturnValue: return "ReturnValue";
        case SpvOpUnreachable: return "Unreachable";
        case SpvOpPhi: return "Phi";
        case SpvOpLoopMerge: return "LoopMerge";
        case SpvOpSelectionMerge: return "SelectionMerge";
        case SpvOpIAdd: return "IAdd";
        case SpvOpISub: return "ISub";
        case SpvOpIMul: return "IMul";
        case SpvOpUDiv: return "UDiv";
        case SpvOpSDiv: return "SDiv";
        case SpvOpUMod: return "UMod";
        case SpvOpSRem: return "SRem";
        case SpvOpSMod: return "SMod";
        case SpvOpFAdd: return "FAdd";
        case SpvOpFSub: return "FSub";
        case SpvOpFMul: return "FMul";
        case SpvOpFDiv: return "FDiv";
        case SpvOpFMod: return "FMod";
        case SpvOpFRem: return "FRem";
        case SpvOpFNegate: return "FNegate";
        case SpvOpSNegate: return "SNegate";
        case SpvOpVectorTimesScalar: return "VectorTimesScalar";
        case SpvOpMatrixTimesScalar: return "MatrixTimesScalar";
        case SpvOpMatrixTimesVector: return "MatrixTimesVector";
        case SpvOpVectorTimesMatrix: return "VectorTimesMatrix";
        case SpvOpMatrixTimesMatrix: return "MatrixTimesMatrix";
        case SpvOpOuterProduct: return "OuterProduct";
        case SpvOpDot: return "Dot";
        case SpvOpCompositeConstruct: return "CompositeConstruct";
        case SpvOpCompositeExtract: return "CompositeExtract";
        case SpvOpCompositeInsert: return "CompositeInsert";
        case SpvOpVectorExtractDynamic: return "VectorExtractDynamic";
        case SpvOpVectorInsertDynamic: return "VectorInsertDynamic";
        case SpvOpVectorShuffle: return "VectorShuffle";
        case SpvOpTranspose: return "Transpose";
        case SpvOpConvertSToF: return "ConvertSToF";
        case SpvOpConvertUToF: return "ConvertUToF";
        case SpvOpConvertFToS: return "ConvertFToS";
        case SpvOpConvertFToU: return "ConvertFToU";
        case SpvOpFConvert: return "FConvert";
        case SpvOpSConvert: return "SConvert";
        case SpvOpUConvert: return "UConvert";
        case SpvOpBitcast: return "Bitcast";
        case SpvOpShiftLeftLogical: return "ShiftLeftLogical";
        case SpvOpShiftRightLogical: return "ShiftRightLogical";
        case SpvOpShiftRightArithmetic: return "ShiftRightArithmetic";
        case SpvOpBitwiseAnd: return "BitwiseAnd";
        case SpvOpBitwiseOr: return "BitwiseOr";
        case SpvOpBitwiseXor: return "BitwiseXor";
        case SpvOpNot: return "Not";
        case SpvOpLogicalAnd: return "LogicalAnd";
        case SpvOpLogicalOr: return "LogicalOr";
        case SpvOpLogicalNot: return "LogicalNot";
        case SpvOpLogicalEqual: return "LogicalEqual";
        case SpvOpLogicalNotEqual: return "LogicalNotEqual";
        case SpvOpAny: return "Any";
        case SpvOpAll: return "All";
        case SpvOpIEqual: return "IEqual";
        case SpvOpINotEqual: return "INotEqual";
        case SpvOpULessThan: return "ULessThan";
        case SpvOpULessThanEqual: return "ULessThanEqual";
        case SpvOpUGreaterThan: return "UGreaterThan";
        case SpvOpUGreaterThanEqual: return "UGreaterThanEqual";
        case SpvOpSLessThan: return "SLessThan";
        case SpvOpSLessThanEqual: return "SLessThanEqual";
        case SpvOpSGreaterThan: return "SGreaterThan";
        case SpvOpSGreaterThanEqual: return "SGreaterThanEqual";
        case SpvOpFOrdEqual: return "FOrdEqual";
        case SpvOpFOrdNotEqual: return "FOrdNotEqual";
        case SpvOpFOrdLessThan: return "FOrdLessThan";
        case SpvOpFOrdLessThanEqual: return "FOrdLessThanEqual";
        case SpvOpFOrdGreaterThan: return "FOrdGreaterThan";
        case SpvOpFOrdGreaterThanEqual: return "FOrdGreaterThanEqual";
        case SpvOpFUnordEqual: return "FUnordEqual";
        case SpvOpFUnordNotEqual: return "FUnordNotEqual";
        case SpvOpFUnordLessThan: return "FUnordLessThan";
        case SpvOpFUnordLessThanEqual: return "FUnordLessThanEqual";
        case SpvOpFUnordGreaterThan: return "FUnordGreaterThan";
        case SpvOpFUnordGreaterThanEqual: return "FUnordGreaterThanEqual";
        case SpvOpSelect: return "Select";
        case SpvOpIsNan: return "IsNan";
        case SpvOpIsInf: return "IsInf";
        case SpvOpDPdx: return "DPdx";
        case SpvOpDPdy: return "DPdy";
        case SpvOpFwidth: return "Fwidth";
        case SpvOpDPdxFine: return "DPdxFine";
        case SpvOpDPdyFine: return "DPdyFine";
        case SpvOpFwidthFine: return "FwidthFine";
        case SpvOpDPdxCoarse: return "DPdxCoarse";
        case SpvOpDPdyCoarse: return "DPdyCoarse";
        case SpvOpFwidthCoarse: return "FwidthCoarse";
        case SpvOpAtomicLoad: return "AtomicLoad";
        case SpvOpAtomicStore: return "AtomicStore";
        case SpvOpAtomicExchange: return "AtomicExchange";
        case SpvOpAtomicIAdd: return "AtomicIAdd";
        case SpvOpAtomicISub: return "AtomicISub";
        case SpvOpAtomicAnd: return "AtomicAnd";
        case SpvOpAtomicOr: return "AtomicOr";
        case SpvOpAtomicXor: return "AtomicXor";
        case SpvOpAtomicSMin: return "AtomicSMin";
        case SpvOpAtomicSMax: return "AtomicSMax";
        case SpvOpAtomicUMin: return "AtomicUMin";
        case SpvOpAtomicUMax: return "AtomicUMax";
        default: return nullptr;
    }
}

// Format one instruction as a human-readable string.
// Shows: [%result = ]OpName [%type] [%op0 %op1 ...]
static std::string format_instruction(const Instruction &inst, const SpvModule &m)
{
    std::ostringstream s;
    // Result prefix
    if (inst.result_id) {
        auto it = m.names.find(inst.result_id);
        s << "%";
        if (it != m.names.end() && !it->second.empty())
            s << it->second;
        else
            s << inst.result_id;
        s << " = ";
    } else {
        s << "         "; // alignment for instructions with no result
    }
    // Opcode name
    const char *oname = spv_op_name(inst.opcode);
    s << "Op";
    if (oname)
        s << oname;
    else
        s << inst.opcode;
    // Type operand (if present)
    if (inst.type_id) {
        auto it = m.names.find(inst.type_id);
        s << " %";
        if (it != m.names.end() && !it->second.empty())
            s << it->second;
        else
            s << inst.type_id;
    }
    // Remaining operands (using operand() which skips type+result)
    for (uint32_t i = 0; i < inst.operand_count(); ++i) {
        uint32_t w = inst.operand(i);
        auto it = m.names.find(w);
        s << " %";
        if (it != m.names.end() && !it->second.empty())
            s << it->second;
        else
            s << w;
    }
    return s.str();
}

// ---- state -----------------------------------------------------------------

static std::shared_ptr<Module> g_module;
static std::unique_ptr<Session> g_session;
static std::string g_entry;
static bool g_panicked = false; // set when any --run ends in Panic

// ---- helpers ---------------------------------------------------------------

static std::vector<std::string> split(const std::string &s)
{
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

static std::string value_to_string(const Value &v)
{
    switch (v.kind) {
        case Value::Kind::Bool: return v.scalar.b ? "true" : "false";
        case Value::Kind::Int32: return std::to_string(v.scalar.i32);
        case Value::Kind::UInt32: return std::to_string(v.scalar.u32);
        case Value::Kind::Float32: return std::to_string(v.scalar.f32);
        case Value::Kind::Int64: return std::to_string(v.scalar.i64);
        case Value::Kind::UInt64: return std::to_string(v.scalar.u64);
        case Value::Kind::Float64: return std::to_string(v.scalar.f64);
        case Value::Kind::Composite:
            {
                bool named = !v.member_names.empty() && v.member_names.size() == v.elements.size();
                std::string s = "{";
                for (size_t i = 0; i < v.elements.size(); ++i) {
                    if (i)
                        s += ", ";
                    if (named && !v.member_names[i].empty())
                        s += v.member_names[i] + "=";
                    s += value_to_string(v.elements[i]);
                }
                return s + "}";
            }
        case Value::Kind::Pointer:
            return "ptr(sc=" + std::to_string(v.ptr_storage_class) + ", base=%"
                 + std::to_string(v.ptr_base_var) + ")";
    }
    return "?";
}

static void ensure_session()
{
    if (!g_session && g_module && !g_entry.empty()) {
        auto r = create_session(g_module, g_entry);
        if (r)
            g_session = std::move(*r);
        else
            std::cerr << "Error creating session: " << r.error().message << "\n";
    }
}

// ---- command handlers ------------------------------------------------------

static void cmd_file(const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        std::cout << "Usage: file <path.spv>\n";
        return;
    }
    auto r = load_module_from_file(args[1]);
    if (!r) {
        std::cerr << "Error: " << r.error().message << "\n";
        return;
    }
    g_module = std::move(*r);
    g_session = nullptr;
    g_entry = {};
    std::cout << "Loaded " << args[1] << "\n";
    // Auto-list entry points.
    auto eps = list_entry_points(*g_module);
    std::cout << eps.size() << " entry point(s):\n";
    for (auto &ep : eps) {
        const char *model = ep.execution_model == ExecutionModel::GLCompute ? "GLCompute"
                          : ep.execution_model == ExecutionModel::Vertex    ? "Vertex"
                          : ep.execution_model == ExecutionModel::Fragment  ? "Fragment"
                                                                            : "Other";
        std::cout << "  " << ep.name << " (" << model << ")\n";
    }
}

static void cmd_info_entries()
{
    if (!g_module) {
        std::cout << "No module loaded.\n";
        return;
    }
    auto eps = list_entry_points(*g_module);
    for (auto &ep : eps) {
        const char *model = ep.execution_model == ExecutionModel::GLCompute ? "GLCompute"
                          : ep.execution_model == ExecutionModel::Vertex    ? "Vertex"
                          : ep.execution_model == ExecutionModel::Fragment  ? "Fragment"
                                                                            : "Other";
        std::cout << ep.name << " (" << model << ")\n";
    }
}

static void cmd_entry(const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        std::cout << "Usage: entry <name>\n";
        return;
    }
    g_entry = args[1];
    g_session = nullptr;
    std::cout << "Entry point set to: " << g_entry << "\n";
}

static void cmd_run()
{
    if (!g_module) {
        std::cout << "No module loaded.\n";
        return;
    }
    if (g_entry.empty()) {
        std::cout << "No entry point selected. Use: entry <name>\n";
        return;
    }
    // Reuse the existing session (preserving descriptor bindings and
    // breakpoints) if one exists; only create a fresh one if we don't have one.
    if (g_session) {
        auto r = spvdb::restart(*g_session);
        if (!r) {
            std::cerr << "Error restarting session: " << r.error().message << "\n";
            return;
        }
    } else {
        auto r = create_session(g_module, g_entry);
        if (!r) {
            std::cerr << "Error: " << r.error().message << "\n";
            return;
        }
        g_session = std::move(*r);
    }
    auto reason = spvdb::run(*g_session);
    switch (reason) {
        case StopReason::EntryFinished: std::cout << "Program finished normally.\n"; break;
        case StopReason::Breakpoint: std::cout << "Breakpoint hit.\n"; break;
        case StopReason::Panic:
            std::cerr << "Panic: " << panic_message(*g_session) << "\n";
            g_panicked = true;
            for (auto &frame : backtrace(*g_session)) {
                std::cerr << "  in " << frame.function_name;
                if (frame.loc.line)
                    std::cerr << " at " << frame.loc.file << ":" << frame.loc.line;
                std::cerr << "\n";
            }
            break;
        default: break;
    }
}

static void print_stop_reason(StopReason reason)
{
    switch (reason) {
        case StopReason::EntryFinished: std::cout << "Program finished normally.\n"; break;
        case StopReason::Panic: std::cerr << "Panic: " << panic_message(*g_session) << "\n"; break;
        case StopReason::Breakpoint:
            {
                std::cout << "Breakpoint hit";
                auto loc = current_location(*g_session);
                if (loc.line)
                    std::cout << " at " << loc.file << ":" << loc.line;
                std::cout << "\n";
                break;
            }
        default:
            {
                // For step/next: print current location if known.
                auto loc = current_location(*g_session);
                if (loc.line)
                    std::cout << loc.file << ":" << loc.line << "\n";
                break;
            }
    }
}

static void cmd_stepi()
{
    if (!g_session) {
        ensure_session();
        if (!g_session)
            return;
    }
    auto reason = step_instruction(*g_session);
    print_stop_reason(reason);
}

static void cmd_step()
{
    if (!g_session) {
        ensure_session();
        if (!g_session)
            return;
    }
    auto reason = step(*g_session);
    print_stop_reason(reason);
}

static void cmd_next()
{
    if (!g_session) {
        ensure_session();
        if (!g_session)
            return;
    }
    auto reason = step_over(*g_session);
    print_stop_reason(reason);
}

static void cmd_continue()
{
    if (!g_session) {
        ensure_session();
        if (!g_session)
            return;
    }
    auto reason = spvdb::run(*g_session);
    print_stop_reason(reason);
}

static void cmd_finish()
{
    if (!g_session) {
        std::cout << "No active session.\n";
        return;
    }
    auto reason = step_out(*g_session);
    print_stop_reason(reason);
}

static void cmd_break(const std::vector<std::string> &args)
{
    if (!g_module) {
        std::cout << "No module loaded.\n";
        return;
    }
    ensure_session();
    if (!g_session)
        return;
    if (args.size() < 2) {
        std::cout << "Usage: break <file>:<line>  or  break %<result-id>\n";
        return;
    }
    const std::string &loc = args[1];
    if (!loc.empty() && loc[0] == '%') {
        uint32_t rid = static_cast<uint32_t>(std::stoul(loc.substr(1)));
        auto r = set_breakpoint_at_id(*g_session, rid);
        if (r)
            std::cout << "Breakpoint " << r->id << " at %" << rid << "\n";
        return;
    }
    auto colon = loc.rfind(':');
    if (colon == std::string::npos) {
        std::cout << "Usage: break <file>:<line>\n";
        return;
    }
    std::string file = loc.substr(0, colon);
    uint32_t line = static_cast<uint32_t>(std::stoul(loc.substr(colon + 1)));
    auto r = set_breakpoint(*g_session, file, line);
    if (r)
        std::cout << "Breakpoint " << r->id << " at " << file << ":" << line << "\n";
}

static void cmd_delete(const std::vector<std::string> &args)
{
    if (!g_session) {
        std::cout << "No active session.\n";
        return;
    }
    if (args.size() < 2) {
        std::cout << "Usage: delete <bp-id>\n";
        return;
    }
    uint32_t id = static_cast<uint32_t>(std::stoul(args[1]));
    remove_breakpoint(*g_session, BreakpointId {id});
    std::cout << "Deleted breakpoint " << id << "\n";
}

static void cmd_info_breakpoints()
{
    std::cout << "(breakpoint list not yet queryable via public API)\n";
}

static void cmd_backtrace()
{
    if (!g_session) {
        std::cout << "No active session.\n";
        return;
    }
    auto frames = backtrace(*g_session);
    if (frames.empty()) {
        std::cout << "(no stack frames)\n";
        return;
    }
    for (size_t i = 0; i < frames.size(); ++i) {
        std::cout << "#" << i << "  " << frames[i].function_name;
        if (frames[i].loc.line)
            std::cout << " at " << frames[i].loc.file << ":" << frames[i].loc.line;
        std::cout << "\n";
    }
}

static void cmd_where()
{
    cmd_backtrace();
}

static void cmd_info_outputs()
{
    if (!g_session) {
        std::cout << "No active session.\n";
        return;
    }
    auto outs = output_variables(*g_session);
    if (outs.empty()) {
        std::cout << "(no output variables)\n";
        return;
    }
    for (auto &v : outs) std::cout << v.name << " = " << value_to_string(v.value) << "\n";
}

static void cmd_info_locals()
{
    if (!g_session) {
        std::cout << "No active session.\n";
        return;
    }
    auto vars = local_variables(*g_session);
    if (vars.empty()) {
        std::cout << "(no local variables)\n";
        return;
    }
    for (auto &v : vars) std::cout << v.name << " = " << value_to_string(v.value) << "\n";
}

static void cmd_print(const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        std::cout << "Usage: print <var>\n";
        return;
    }
    if (!g_session) {
        std::cout << "No active session.\n";
        return;
    }
    auto r = evaluate_variable(*g_session, args[1]);
    if (!r) {
        std::cerr << r.error().message << "\n";
        return;
    }
    std::cout << r->name << " = " << value_to_string(r->value) << "\n";
}

static void cmd_set_input(const std::vector<std::string> &args)
{
    // set input loc <location> <json_literal_or_filename>
    // set input <set> <binding> <json_literal_or_filename>
    if (args.size() >= 3 && args[2] == "loc") {
        // Location-based input binding for vertex/fragment shaders.
        if (args.size() < 5) {
            std::cout << "Usage: set input loc <location> <json>\n";
            return;
        }
        if (!g_module) {
            std::cout << "No module loaded.\n";
            return;
        }
        ensure_session();
        if (!g_session)
            return;
        uint32_t loc_idx = std::stoul(args[3]);
        std::string data = args[4];
        for (size_t i = 5; i < args.size(); ++i) data += " " + args[i];
        auto r = set_input_location_json(*g_session, loc_idx, data);
        if (!r)
            std::cerr << "Error: " << r.error().message << "\n";
        else
            std::cout << "Set input location " << loc_idx << "\n";
        return;
    }

    // Descriptor-based input binding (set + binding).
    if (args.size() < 5) {
        std::cout << "Usage: set input <set> <binding> <json>  OR  set input loc <loc> <json>\n";
        return;
    }
    if (!g_module) {
        std::cout << "No module loaded.\n";
        return;
    }
    ensure_session();
    if (!g_session)
        return;

    uint32_t s_idx = std::stoul(args[2]);
    uint32_t b_idx = std::stoul(args[3]);
    std::string data = args[4];
    // Concatenate remaining args (in case JSON has spaces).
    for (size_t i = 5; i < args.size(); ++i) data += " " + args[i];

    // Detect inline JSON vs image file vs JSON file.
    auto has_image_ext = [](const std::string &p)
    {
        static const char *exts[] =
            {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".hdr", ".exr", nullptr};
        // Case-insensitive suffix match.
        for (int i = 0; exts[i]; ++i) {
            std::string ext = exts[i];
            if (p.size() >= ext.size()) {
                std::string suf = p.substr(p.size() - ext.size());
                for (auto &c : suf) c = static_cast<char>(std::tolower(c));
                if (suf == ext)
                    return true;
            }
        }
        return false;
    };

    if (!data.empty() && (data[0] == '[' || data[0] == '{')) {
        auto r = set_descriptor_json(*g_session, s_idx, b_idx, data);
        if (!r)
            std::cerr << "Error: " << r.error().message << "\n";
        else
            std::cout << "Set descriptor " << s_idx << ":" << b_idx << "\n";
    } else if (has_image_ext(data)) {
        auto r = set_image(*g_session, s_idx, b_idx, data);
        if (!r)
            std::cerr << "Error: " << r.error().message << "\n";
        else
            std::cout << "Set image " << s_idx << ":" << b_idx << " from " << data << "\n";
    } else {
        // Treat as JSON filename — load and pass.
        std::ifstream f(data);
        if (!f) {
            std::cerr << "Cannot open file: " << data << "\n";
            return;
        }
        std::string json((std::istreambuf_iterator<char>(f)), {});
        auto r = set_descriptor_json(*g_session, s_idx, b_idx, json);
        if (!r)
            std::cerr << "Error: " << r.error().message << "\n";
        else
            std::cout << "Set descriptor " << s_idx << ":" << b_idx << " from file\n";
    }
}

static void cmd_set_builtin(const std::vector<std::string> &args)
{
    // set builtin <name> <val0> [val1] [val2]
    if (args.size() < 4) {
        std::cout << "Usage: set builtin <name> <val...>\n";
        return;
    }
    ensure_session();
    if (!g_session)
        return;
    // Map common builtin names to SpvBuiltIn values.
    static const struct
    {
        const char *name;
        uint32_t id;
    } kBuiltins[] = {
        {"GlobalInvocationID", SpvBuiltInGlobalInvocationId},
        {"LocalInvocationID", SpvBuiltInLocalInvocationId},
        {"WorkgroupID", SpvBuiltInWorkgroupId},
        {"LocalInvocationIndex", SpvBuiltInLocalInvocationIndex},
        {"VertexIndex", SpvBuiltInVertexIndex},
        {"InstanceIndex", SpvBuiltInInstanceIndex},
        {"Position", SpvBuiltInPosition},
        {"PointSize", SpvBuiltInPointSize},
        {"FragCoord", SpvBuiltInFragCoord},
        {"FrontFacing", SpvBuiltInFrontFacing},
        {"FragDepth", SpvBuiltInFragDepth},
        {"NumWorkgroups", SpvBuiltInNumWorkgroups},
        {"SubgroupSize", SpvBuiltInSubgroupSize},
    };
    uint32_t bi = 0;
    bool found = false;
    for (auto &kb : kBuiltins) {
        if (args[2] == kb.name) {
            bi = kb.id;
            found = true;
            break;
        }
    }
    if (!found) {
        std::cerr << "Unknown builtin: " << args[2] << "\n";
        return;
    }

    // Parse value: three components = vec3/uvec3, one component = scalar.
    if (args.size() >= 6) {
        // Try uint first, then float.
        try {
            Value v = Value::make_composite({Value::make_u32(std::stoul(args[3])),
                                             Value::make_u32(std::stoul(args[4])),
                                             Value::make_u32(std::stoul(args[5]))});
            set_builtin(*g_session, bi, v);
        } catch (...) {
            Value v = Value::make_composite({Value::make_f32(std::stof(args[3])),
                                             Value::make_f32(std::stof(args[4])),
                                             Value::make_f32(std::stof(args[5]))});
            set_builtin(*g_session, bi, v);
        }
    } else if (args.size() >= 7) {
        // Four-component (e.g., FragCoord as vec4).
        Value v = Value::make_composite({Value::make_f32(std::stof(args[3])),
                                         Value::make_f32(std::stof(args[4])),
                                         Value::make_f32(std::stof(args[5])),
                                         Value::make_f32(std::stof(args[6]))});
        set_builtin(*g_session, bi, v);
    } else {
        try {
            set_builtin(*g_session, bi, Value::make_u32(std::stoul(args[3])));
        } catch (...) {
            set_builtin(*g_session, bi, Value::make_f32(std::stof(args[3])));
        }
    }
    std::cout << "Set builtin " << args[2] << "\n";
}

static void cmd_disassemble()
{
    if (!g_session) {
        std::cout << "No active session.\n";
        return;
    }
    const PC &pc = g_session->interp.current_pc();
    if (!pc.valid()) {
        std::cout << "Execution not started.\n";
        return;
    }
    const SpvModule &m = *g_session->module;
    auto fit = m.functions.find(pc.function_id);
    if (fit == m.functions.end()) {
        std::cout << "Current function not found.\n";
        return;
    }
    const Function &fn = fit->second;

    // Print header.
    std::cout << "Function " << (fn.name.empty() ? "%" + std::to_string(pc.function_id) : fn.name)
              << ":\n";

    // Show the current block and the two blocks that follow it (if any).
    auto bit = fn.block_index.find(pc.block_label);
    if (bit == fn.block_index.end())
        return;
    size_t start_idx = bit->second;
    size_t end_idx = std::min(start_idx + 3, fn.blocks.size());

    for (size_t bi2 = start_idx; bi2 < end_idx; ++bi2) {
        const BasicBlock &bb = fn.blocks[bi2];
        auto lab_it = m.names.find(bb.label_id);
        std::cout << "\n%"
                  << (lab_it != m.names.end() && !lab_it->second.empty()
                          ? lab_it->second
                          : std::to_string(bb.label_id))
                  << ":\n";
        for (size_t i = 0; i < bb.instructions.size(); ++i) {
            bool is_current = (bi2 == start_idx && i == pc.instr_index);
            std::cout << (is_current ? " ==> " : "     ")
                      << format_instruction(bb.instructions[i], m);
            // Show source location annotation if available.
            if (i < bb.source_locs.size() && bb.source_locs[i].valid())
                std::cout << "    ; " << bb.source_locs[i].file << ":" << bb.source_locs[i].line;
            std::cout << "\n";
        }
    }
}

static void cmd_list(const std::vector<std::string> &args)
{
    if (!g_session) {
        std::cout << "No active session.\n";
        return;
    }
    auto loc = current_location(*g_session);
    if (!loc.line) {
        std::cout << "No source location available.\n";
        return;
    }

    // Optionally override line number.
    uint32_t center_line = loc.line;
    if (args.size() >= 2) {
        try {
            center_line = static_cast<uint32_t>(std::stoul(args[1]));
        } catch (...) {
        }
    }

    const int CONTEXT = 5;
    uint32_t first = center_line > (uint32_t) CONTEXT ? center_line - CONTEXT : 1;
    uint32_t last = center_line + CONTEXT;

    std::ifstream f(loc.file);
    if (!f) {
        std::cout << "Source file not found: " << loc.file << "\n";
        std::cout << "Current location: " << loc.file << ":" << loc.line << "\n";
        return;
    }

    std::string line;
    uint32_t ln = 0;
    while (std::getline(f, line)) {
        ++ln;
        if (ln < first)
            continue;
        if (ln > last)
            break;
        std::cout << (ln == center_line ? "=> " : "   ") << ln << "\t" << line << "\n";
    }
}

static void cmd_set_specconst(const std::vector<std::string> &args)
{
    if (args.size() < 4) {
        std::cout << "Usage: set specconst <id> <val>\n";
        return;
    }
    ensure_session();
    if (!g_session)
        return;
    uint32_t spec_id = std::stoul(args[2]);
    // Try int, then float.
    try {
        set_spec_constant(*g_session, spec_id, static_cast<int32_t>(std::stoi(args[3])));
        std::cout << "Set specconst " << spec_id << " = " << args[3] << "\n";
    } catch (...) {
        set_spec_constant(*g_session, spec_id, std::stof(args[3]));
        std::cout << "Set specconst " << spec_id << " = " << args[3] << "\n";
    }
}

// ---- dispatch --------------------------------------------------------------

static bool dispatch(const std::string &line); // forward declaration

static void cmd_source(const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        std::cout << "Usage: source <file>\n";
        return;
    }
    std::ifstream f(args[1]);
    if (!f) {
        std::cerr << "Cannot open file: " << args[1] << "\n";
        return;
    }
    std::string line;
    while (std::getline(f, line)) {
        // Strip leading whitespace and skip blank lines and comments.
        auto start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos)
            continue;
        if (line[start] == '#')
            continue;
        std::string trimmed = line.substr(start);
        std::cout << "(spvdb) " << trimmed << "\n";
        if (!dispatch(trimmed))
            break;
    }
}

static bool dispatch(const std::string &line)
{
    auto tokens = split(line);
    if (tokens.empty())
        return true;

    const std::string &cmd = tokens[0];

    if (cmd == "quit" || cmd == "q" || cmd == "exit")
        return false;

    if (cmd == "file") {
        cmd_file(tokens);
        return true;
    }
    if (cmd == "entry") {
        cmd_entry(tokens);
        return true;
    }
    if (cmd == "run" || cmd == "r") {
        cmd_run();
        return true;
    }
    if (cmd == "continue" || cmd == "c") {
        cmd_continue();
        return true;
    }
    if (cmd == "step" || cmd == "s") {
        cmd_step();
        return true;
    }
    if (cmd == "next" || cmd == "n") {
        cmd_next();
        return true;
    }
    if (cmd == "finish") {
        cmd_finish();
        return true;
    }
    if (cmd == "stepi" || cmd == "si") {
        cmd_stepi();
        return true;
    }
    if (cmd == "break" || cmd == "b") {
        cmd_break(tokens);
        return true;
    }
    if (cmd == "delete" || cmd == "d") {
        cmd_delete(tokens);
        return true;
    }
    if (cmd == "backtrace" || cmd == "bt" || cmd == "where") {
        cmd_backtrace();
        return true;
    }
    if (cmd == "print" || cmd == "p") {
        cmd_print(tokens);
        return true;
    }
    if (cmd == "info") {
        if (tokens.size() > 1 && tokens[1] == "entries") {
            cmd_info_entries();
            return true;
        }
        if (tokens.size() > 1 && tokens[1] == "outputs") {
            cmd_info_outputs();
            return true;
        }
        if (tokens.size() > 1 && tokens[1] == "locals") {
            cmd_info_locals();
            return true;
        }
        if (tokens.size() > 1 && tokens[1] == "breakpoints") {
            cmd_info_breakpoints();
            return true;
        }
        std::cout << "info subcommands: entries, outputs, locals, breakpoints\n";
        return true;
    }
    if (cmd == "set") {
        if (tokens.size() > 1 && tokens[1] == "input") {
            cmd_set_input(tokens);
            return true;
        }
        if (tokens.size() > 1 && tokens[1] == "builtin") {
            cmd_set_builtin(tokens);
            return true;
        }
        if (tokens.size() > 1 && tokens[1] == "specconst") {
            cmd_set_specconst(tokens);
            return true;
        }
        std::cout << "set subcommands: input, builtin, specconst\n";
        return true;
    }
    if (cmd == "source") {
        cmd_source(tokens);
        return true;
    }
    if (cmd == "disassemble" || cmd == "dis") {
        cmd_disassemble();
        return true;
    }
    if (cmd == "list" || cmd == "l") {
        cmd_list(tokens);
        return true;
    }
    if (cmd == "help" || cmd == "h" || cmd == "?") {
        std::cout << "Commands:\n"
                     "  file <path.spv>               Load SPIR-V module\n"
                     "  info entries                  List entry points\n"
                     "  entry <name>                  Select entry point\n"
                     "  run / r                       Execute from the start\n"
                     "  continue / c                  Continue execution to next breakpoint\n"
                     "  step / s                      Step one source line (into calls)\n"
                     "  next / n                      Step one source line (over calls)\n"
                     "  finish                        Run until current function returns\n"
                     "  stepi / si                    Step one SPIR-V instruction\n"
                     "  break <file>:<line>           Set a source breakpoint\n"
                     "  break %<id>                   Set a result-id breakpoint\n"
                     "  delete <bp-id>                Remove a breakpoint\n"
                     "  info breakpoints              List breakpoints\n"
                     "  backtrace / bt                Print call stack\n"
                     "  info locals                   Print local variables\n"
                     "  info outputs                  Print output variables\n"
                     "  print <var> / p <var>         Print a variable\n"
                     "  set input <s> <b> <json>      Bind descriptor (set+binding) from JSON\n"
                     "  set input loc <loc> <json>    Bind input by Location decoration\n"
                     "  set builtin <name> <val>      Set a built-in value\n"
                     "  set specconst <id> <val>      Override a spec constant\n"
                     "  source <file>                 Execute commands from a script file\n"
                     "  disassemble / dis             Show SPIR-V near current instruction\n"
                     "  list [line] / l [line]        Show source lines near current location\n"
                     "  quit / q                      Exit\n";
        return true;
    }

    std::cerr << "Unknown command: " << cmd << " (try 'help')\n";
    return true;
}

// ---- main ------------------------------------------------------------------

int main(int argc, char **argv)
{
    // Argument structure:
    //
    //   spvdb [session-options] [--] [file.spv] [commands...]
    //
    // Session options are flags that configure interpreter-wide behaviour and
    // must appear before the SPIR-V file (or the explicit '--' separator).
    // Currently the only session option is -h/--help.
    //
    // After the optional '--', the first non-flag argument is treated as an
    // implied '--file <path>'.  Every subsequent argument is a command that is
    // executed in the order it appears.  If at least one command is present,
    // spvdb exits when the sequence is exhausted; otherwise it enters the
    // interactive replxx session.

    // ---- Phase 1: session options ----------------------------------------
    // Consume flags until we hit '--', a positional arg, or a command flag.

    bool show_help = false;
    bool explicit_sep = false; // '--' was given
    int i = 1;

    for (; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--") {
            explicit_sep = true;
            ++i;
            break;
        }
        if (a == "-h" || a == "--help") {
            show_help = true;
            continue;
        }
        // Anything else (positional or command flag) ends the session-option
        // phase; leave i pointing at it.
        break;
    }

    if (show_help) {
        std::cout << "Usage: spvdb [session-options] [--] [file.spv] [commands...]\n"
                     "\n"
                     "Session options (before the file / '--' separator):\n"
                     "  -h, --help                    Show this help message\n"
                     "\n"
                     "Commands (executed left-to-right after the file):\n"
                     "  --file <path.spv>             Load a SPIR-V module\n"
                     "  --entry <name>                Select entry point\n"
                     "  --run                         Run the current entry point\n"
                     "  --source <file> / -x <file>   Execute a spvdb script\n"
                     "  --entrypoints                 Print entry points of the loaded module\n"
                     "\n"
                     "If any commands are given, spvdb exits after processing them.\n"
                     "Without commands, spvdb enters interactive mode (with the file pre-loaded).\n"
                     "\n"
                     "Examples:\n"
                     "  spvdb shader.spv                            Interactive session\n"
                     "  spvdb shader.spv --entry main --run         Run entry point and exit\n"
                     "  spvdb shader.spv --entrypoints              List entry points and exit\n"
                     "  spvdb a.spv --run --file b.spv --run        Run a.spv then b.spv\n"
                     "  spvdb shader.spv --source setup.spvdbrc     Execute script and exit\n";
        return 0;
    }

    // ---- Phase 2: command sequence ----------------------------------------

    bool had_commands = explicit_sep;

    // First positional arg (no leading '-') = implied --file.
    if (i < argc && argv[i][0] != '-') {
        std::vector<std::string> args = {"file", argv[i++]};
        cmd_file(args);
    }

    // Process remaining args as ordered commands.
    for (; i < argc; ++i) {
        had_commands = true;
        std::string a = argv[i];

        if ((a == "--file") && i + 1 < argc) {
            std::vector<std::string> args = {"file", argv[++i]};
            cmd_file(args);
        } else if (a == "--entry" && i + 1 < argc) {
            std::vector<std::string> args = {"entry", argv[++i]};
            cmd_entry(args);
        } else if (a == "--run") {
            cmd_run();
        } else if ((a == "--source" || a == "-x") && i + 1 < argc) {
            std::vector<std::string> args = {"source", argv[++i]};
            cmd_source(args);
        } else if (a == "--entrypoints") {
            cmd_info_entries();
        } else {
            std::cerr << "Unknown command: " << a << "  (try -h)\n";
            return 1;
        }
    }

    if (had_commands)
        return g_panicked ? 1 : 0;

    // ---- Interactive replxx mode -----------------------------------------

    Replxx rx;
    rx.set_word_break_characters(" \t\n");
    rx.history_load(".spvdb_history");

    std::cout << "spvdb — SPIR-V debugger  (type 'help' for commands)\n";

    while (true) {
        const char *line_raw = rx.input("(spvdb) ");
        if (!line_raw)
            break; // EOF / Ctrl-D
        std::string line(line_raw);
        if (line.empty())
            continue;
        rx.history_add(line);
        if (!dispatch(line))
            break;
    }

    rx.history_save(".spvdb_history");
    return 0;
}
