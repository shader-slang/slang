// slang-ir-check-optional-none-usage.cpp
#include "slang-ir-check-optional-none-usage.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

// Diagnose a `none` value of an `Optional<T>` whose payload T transitively
// contains an opaque handle (texture/sampler/buffer). The front-end already
// rejects this (E30902) for every case where the concrete payload type is
// visible at semantic-check time, but a generic member accessor such as
// `struct S<T> { Optional<T> f() { return none; } }` is checked once with T
// still abstract, so the guard passes; the concrete `Optional<Texture2D>` only
// appears during IR generic specialization. Without this check the
// `MakeOptionalNone` reaches `lowerOptionalType`, which synthesizes a
// `defaultConstruct` of the opaque payload to fill the `hasValue == false`
// placeholder slot — a value no backend can emit (SPIR-V raises the E99997
// "unhandled local inst" internal error). We run after specialization and
// before lowering, so the payload type is concrete. We emit E41037, the IR-layer
// variant of the AST rule E30902: same summary text so users see one consistent
// rule, but an IR-typed span (`~type:IRInst`) so the diagnostic renders in the
// same rich style as the sibling optional-none IR check (E41027) rather than the
// classic style the AST-typed E30902 struct would force from an IR pass. (Only
// `none` is checked: `MakeOptionalValue` carries a real payload operand and never
// triggers the synthesized-`defaultConstruct` ICE.)
static void checkNoneOfOpaqueOptional(IRMakeOptionalNone* inst, DiagnosticSink* sink)
{
    auto optType = as<IROptionalType>(inst->getDataType());
    if (!optType)
        return;
    auto valueType = optType->getValueType();
    if (isOpaqueType(valueType, nullptr))
    {
        sink->diagnose(Diagnostics::OptionalCannotWrapResourceTypeIr{
            .type = valueType,
            .location = inst->sourceLoc,
        });
    }
}

static void checkForOptionalNoneUsage(IRFunc* func, DiagnosticSink* sink)
{
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_GetOptionalValue &&
                inst->getOperand(0)->getOp() == kIROp_MakeOptionalNone)
            {
                sink->diagnose(Diagnostics::AccessingValueOfNoneOptional{
                    .type = inst->getDataType(),
                    .location = inst->sourceLoc,
                });
            }
            else if (auto noneInst = as<IRMakeOptionalNone>(inst))
            {
                checkNoneOfOpaqueOptional(noneInst, sink);
            }
        }
    }
}

void checkForOptionalNoneUsage(IRModule* module, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_Func:
            checkForOptionalNoneUsage(as<IRFunc>(globalInst), sink);
            break;
        case kIROp_Generic:
            {
                auto generic = as<IRGeneric>(globalInst);
                auto innerFunc = as<IRFunc>(findGenericReturnVal(generic));
                if (innerFunc)
                    checkForOptionalNoneUsage(innerFunc, sink);
                break;
            }
        default:
            break;
        }
    }
}

} // namespace Slang
