#include "slang-ir-check-unsupported-inst.h"

#include "../core/slang-dictionary.h"
#include "../core/slang-type-text-util.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-target.h"

namespace Slang
{

// Targets that emit "standalone" C++/CUDA — they do not inject `slang.h`
// (or `slang-cpp-types-core.h`'s `String` wrapper) into the generated
// source, so a `String` identifier reaching emit produces undeclared-
// identifier C++ output (#11297). The `Host*` cousins do include
// `slang.h`, so `String` resolves to `Slang::String` and is allowed.
//
// `CUDAHeader` (`-target cuh`) and `CUDASource` go through the same
// `CUDASourceEmitter` and use `prelude/slang-cuda-prelude.h`, so both
// must be diagnosed; same pairing for `CPPSource` / `CPPHeader`.
static bool _isStandaloneCppLikeTarget(TargetRequest* target)
{
    switch (target->getTarget())
    {
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
        return true;
    default:
        return false;
    }
}

// Hint text appended after the main error sentence. `NativeString`'s
// `getLength` / `getBuffer` are gated `[require(cpp_llvm)]`
// (`core.meta.slang:2119,2134`) and `-target host-cpp` is a CPU target,
// so neither is actionable advice for cuda kernel authors. Keep the
// suggestion target-appropriate.
static UnownedStringSlice _stringTypeHintFor(TargetRequest* target)
{
    switch (target->getTarget())
    {
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
        return UnownedStringSlice::fromLiteral(
            "use 'NativeString' instead, or compile with -target host-cpp");
    default:
        return UnownedStringSlice::fromLiteral(
            "the standalone cpp/cuda preludes do not include the Slang runtime");
    }
}

// Peel `Out<T>` / `InOut<T>` / `Ptr<T>` / `Ref<T>` wrappers (all of
// which derive from `IRPtrTypeBase`) and answer whether the underlying
// type is `kIROp_StringType`.
static bool _isStringLikeType(IRType* type)
{
    while (auto ptr = as<IRPtrTypeBase>(type))
        type = ptr->getValueType();
    return type && type->getOp() == kIROp_StringType;
}

// Return true if `inst`'s data type is `String` (after peeling pointer-
// like wrappers — an `out String` parameter has `Out<String>` as its
// data type).
static bool _isStringTyped(IRInst* inst)
{
    return inst && _isStringLikeType(inst->getDataType());
}

// IR ops that legitimately carry a `String`-typed `IRStringLit` operand
// at this stage of lowering but never produce a runtime `String` value
// in the generated source. Either the front-end consumes their string
// and the inst is dropped before emit (`StaticAssert`, `RequirePrelude`,
// `RequireTargetExtension`), or the string is inlined as raw asm /
// metadata rather than as a `String` value (`GenericAsm`). Block
// parameters (`kIROp_Param`) are skipped during the body walk because
// they are reported separately via `func->getParams()`.
static bool _isPreEmitStringConsumer(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_Param:
    case kIROp_StaticAssert:
    case kIROp_RequirePrelude:
    case kIROp_RequireTargetExtension:
    case kIROp_GenericAsm:
        return true;
    default:
        return false;
    }
}

// Walk `func` and call `report(offendingInst)` for every distinct
// position where a `String`-typed value reaches the cpp/cuda emit
// pipeline. Three categories are checked:
//
// 1. Function-signature parameters (`void f(String s)`,
//    `void f(out String s)`). These are emitted verbatim into the
//    target source. Body-less declarations still need this check —
//    `extern void log(String);` emitted to `-target cpp-header`
//    prints `void log(String)` into the `.h`.
// 2. Function return type. `String foo()` emits `String foo()`.
// 3. Block-level instructions. We skip a small allowlist of pre-emit
//    consumers (`StaticAssert`, `RequirePrelude`,
//    `RequireTargetExtension`, `GenericAsm`) whose `String`-typed
//    operands are dropped or inlined as raw asm/metadata before
//    emit; everything else is checked. Operand types are inspected
//    because `IRStringLit` is always `String`-typed regardless of
//    whether the consuming intrinsic expects `String` or
//    `NativeString` (see `IRBuilder::getStringValue`).
template<typename Report>
static void _walkStringUsesInFunc(IRFunc* func, Report report)
{
    for (auto param : func->getParams())
    {
        if (_isStringTyped(param))
            report(param);
    }

    if (auto funcType = as<IRFuncType>(func->getDataType()))
    {
        if (_isStringLikeType(funcType->getResultType()))
            report(func);
    }

    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (_isPreEmitStringConsumer(inst))
                continue;
            if (_isStringTyped(inst))
            {
                report(inst);
                continue;
            }
            const UInt operandCount = inst->getOperandCount();
            for (UInt i = 0; i < operandCount; ++i)
            {
                if (_isStringTyped(inst->getOperand(i)))
                {
                    report(inst);
                    break;
                }
            }
        }
    }
}

void checkUnsupportedInst(TargetRequest* target, IRFunc* func, DiagnosticSink* sink)
{
    SLANG_UNUSED(target);
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            switch (inst->getOp())
            {
            case kIROp_GetArrayLength:
                sink->diagnose(
                    Diagnostics::AttemptToQuerySizeOfUnsizedArray{.location = inst->sourceLoc});
                break;
            }
        }
    }
}

// On standalone cpp/cuda targets, `String` resolves to an undeclared
// identifier in the generated source (the prelude does not include
// `slang.h`). Detect uses of `String`-typed values so that the user
// gets a clear front-end error instead of a downstream g++ error like:
//     int64_t _S3 = "123".getLength();
//     // -> request for member 'getLength' in '"123"', non-class type
// (#11297). Diagnose every offending function — sibling checks in this
// file (e.g. `kIROp_GetArrayLength` above) iterate to completion, so we
// follow the same pattern rather than stopping after the first hit.
static void _checkStringTypeUse(IRModule* module, TargetRequest* target, DiagnosticSink* sink)
{
    const UnownedStringSlice hint = _stringTypeHintFor(target);
    const String targetName = String(
        TypeTextUtil::getCompileTargetName(SlangCompileTarget(target->getTarget())));

    for (auto globalInst : module->getGlobalInsts())
    {
        IRFunc* func = nullptr;
        if (auto f = as<IRFunc>(globalInst))
            func = f;
        else if (auto generic = as<IRGeneric>(globalInst))
            func = as<IRFunc>(findInnerMostGenericReturnVal(generic));

        if (!func)
            continue;

        // IR lowering can split one source-level expression across
        // several String-typed insts (e.g. `let s = call f()` produces
        // both a `let` and a `call` that share a source loc). Dedupe
        // by raw source-loc within a function so the user sees one
        // diagnostic per source position rather than per IR inst.
        HashSet<SourceLoc::RawValue> reportedLocs;
        _walkStringUsesInFunc(
            func,
            [&](IRInst* stringUse)
            {
                SourceLoc loc =
                    stringUse->sourceLoc.isValid() ? stringUse->sourceLoc : func->sourceLoc;
                if (!reportedLocs.add(loc.getRaw()))
                    return;
                sink->diagnose(Diagnostics::StringTypeNotSupportedOnTarget{
                    .target = targetName,
                    .hint = String(hint),
                    .location = loc});
            });
    }
}

void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink)
{
    if (_isStandaloneCppLikeTarget(target))
        _checkStringTypeUse(module, target, sink);

    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_VectorType:
        case kIROp_MatrixType:
            {
                if (!as<IRBasicType>(globalInst->getOperand(0)) &&
                    !as<IRPackedFloatType>(globalInst->getOperand(0)))
                {
                    sink->diagnose(Diagnostics::UnsupportedBuiltinType{
                        .type = globalInst,
                        .location = findFirstUseLoc(globalInst)});
                }
                break;
            }
        case kIROp_Func:
            checkUnsupportedInst(target, as<IRFunc>(globalInst), sink);
            break;
        case kIROp_Generic:
            {
                auto generic = as<IRGeneric>(globalInst);
                auto innerFunc = as<IRFunc>(findGenericReturnVal(generic));
                if (innerFunc)
                    checkUnsupportedInst(target, innerFunc, sink);
                break;
            }
        default:
            break;
        }
    }
}

} // namespace Slang
