#include "slang-ir-check-unsupported-inst.h"

#include "../core/slang-type-text-util.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-target.h"

namespace Slang
{

// Targets that emit "standalone" C++/CUDA — they do not inject `slang.h` into
// the generated source and so do not have access to `Slang::String`. Using
// `String` on these targets produces invalid C++ output (#11297). The
// `Host*` cousins do include `slang.h`, so `String` resolves to
// `Slang::String` and is allowed.
static bool _isStandaloneCppLikeTarget(TargetRequest* target)
{
    switch (target->getTarget())
    {
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::CUDASource:
        return true;
    default:
        return false;
    }
}

// Return true if `inst`'s type is the `String` type.
static bool _isStringTyped(IRInst* inst)
{
    return inst && inst->getDataType() && inst->getDataType()->getOp() == kIROp_StringType;
}

// Find the first instruction in `func` that exposes a `String`-typed
// value on the cpp/cuda emit path: a `String` parameter in the function
// signature, or a `kIROp_Call` whose return type or any operand is
// `String`.
//
// We restrict body-walking to `kIROp_Call` rather than scanning all
// block-level insts because several non-emit IR ops legitimately carry
// `String` operands that never reach the cpp/cuda backend — most
// notably `kIROp_StaticAssert` (the message is consumed by
// `checkStaticAssert` and the inst is then dropped). Calls, on the
// other hand, *are* emitted: a String-using call site survives into
// the generated source and produces the undeclared-`String` failure
// from #11297. Function-signature params are also emitted verbatim
// (`void f(String s)`) so they're checked separately.
//
// `IRStringLit`s always carry `kIROp_StringType` regardless of whether
// the calling context expects `String` or `NativeString` — see
// `IRBuilder::getStringValue` — so a literal's type alone is not a
// reliable user-vs-builtin signal. A Call with a String-typed operand
// is, because intrinsics that take `NativeString` (e.g. `printf`,
// `static_assert`) rewrite the operand at conversion time.
static IRInst* _findStringUseInFunc(IRFunc* func)
{
    for (auto param : func->getParams())
    {
        if (_isStringTyped(param))
            return param;
    }
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() != kIROp_Call)
                continue;
            if (_isStringTyped(inst))
                return inst;
            const UInt operandCount = inst->getOperandCount();
            for (UInt i = 0; i < operandCount; ++i)
            {
                IRInst* operand = inst->getOperand(i);
                if (_isStringTyped(operand))
                    return inst;
            }
        }
    }
    return nullptr;
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
// `slang.h`). Detect block-level uses of `String`-typed values so that
// the user gets a clear front-end error instead of a downstream g++
// error like:
//     int64_t _S3 = "123".getLength();
//     // -> request for member 'getLength' in '"123"', non-class type
// (#11297). Diagnose at most once per compile.
static void _checkStringTypeUse(
    IRModule* module,
    TargetRequest* target,
    DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        IRFunc* func = nullptr;
        if (auto f = as<IRFunc>(globalInst))
            func = f;
        else if (auto generic = as<IRGeneric>(globalInst))
            func = as<IRFunc>(findGenericReturnVal(generic));

        if (!func || !func->getFirstBlock())
            continue;

        if (auto stringUse = _findStringUseInFunc(func))
        {
            SourceLoc loc =
                stringUse->sourceLoc.isValid() ? stringUse->sourceLoc : func->sourceLoc;
            sink->diagnose(Diagnostics::StringTypeNotSupportedOnTarget{
                .target = String(TypeTextUtil::getCompileTargetName(
                    SlangCompileTarget(target->getTarget()))),
                .location = loc});
            return;
        }
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
