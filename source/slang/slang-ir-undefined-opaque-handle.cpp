#include "slang-ir-undefined-opaque-handle.h"

#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-legalize-types.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
// A `LoadFromUninitializedMemory` yields an arbitrary value of its type, and a pass may choose one
// particular value for all of its uses. For an opaque handle such as a `Texture2D`, SPIR-V for
// Vulkan has no undefined value at all: spirv-val rejects `OpUndef` of an image, sampler or
// acceleration-structure type (VUID-StandaloneSpirv-OpUndef-12464). We therefore choose, once for
// each undefined handle and so consistently for all of its uses, a handle to a descriptor that was
// never written. Dynamically accessing such a handle is undefined behavior on Vulkan, so each
// operation that reads or writes through it may yield any result and have no effect.
//
// Consider:
//
//     Texture2D<float4> t;
//     float4 r = 0;
//     if (c)
//         r = t.SampleLevel(s, uv, 0);
//
// By this point the sample is a `SPIRVAsm` block that reads `makeCombinedTextureSampler(t, s)`.
// We replace the combined sampler with another undefined handle, replace the block's result with a
// fresh `LoadFromUninitializedMemory<float4>`, and remove the block, which leaves `t` unused. A
// result is a frozen value rather than `Poison`, so we introduce no poison. Undefined values of
// other types are not touched, so their correlation and loop stability are unchanged.

static bool isUndefinedOpaqueHandle(IRInst* inst)
{
    return as<IRUndefined>(inst) && isLeafUnstorableOpaqueHandleType(inst->getDataType());
}

/// Return whether `inst` derives a new opaque handle from one of its operands, such as combining a
/// texture with a sampler. Such a handle is undefined whenever its source is.
static bool doesOpDeriveOpaqueHandle(IROp op)
{
    switch (op)
    {
    case kIROp_MakeCombinedTextureSampler:
    case kIROp_CombinedTextureSamplerGetTexture:
    case kIROp_CombinedTextureSamplerGetSampler:
        return true;
    default:
        return false;
    }
}

/// Call `f` on every undefined opaque handle in `module`, at module scope and in function bodies.
template<typename F>
static void forEachUndefinedOpaqueHandle(IRModule* module, F const& f)
{
    List<IRInst*> handles;
    for (auto globalInst : module->getGlobalInsts())
    {
        if (isUndefinedOpaqueHandle(globalInst))
            handles.add(globalInst);
        auto code = as<IRGlobalValueWithCode>(globalInst);
        if (!code)
            continue;
        for (auto block : code->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (isUndefinedOpaqueHandle(inst))
                    handles.add(inst);
            }
        }
    }
    for (auto handle : handles)
        f(handle);
}

struct UndefinedOpaqueHandleContext
{
    IRModule* module;

    /// Values whose uses still need rewriting: undefined handles, and addresses into the contents
    /// of an undefined handle (from `ImageSubscript`, for example).
    List<IRInst*> worklist;

    /// Calls that pass an undefined handle and that we may inline.
    List<IRCall*> callsToInline;

    /// Replace `inst`, which accesses an undefined handle or depends on one, with a fresh
    /// `LoadFromUninitializedMemory` of its type, and remove it. A void `inst` is removed. If the
    /// fresh value is itself an undefined handle or an address, it is queued in turn.
    void replaceWithUndefinedValue(IRInst* inst)
    {
        if (inst->getOp() == kIROp_MakeCombinedTextureSampler)
            forwardCombinedTextureSamplerProjections(inst);
        auto type = inst->getDataType();
        if (inst->hasUses() && type && !as<IRVoidType>(type))
        {
            if (as<IRPtrTypeBase>(type))
            {
                // An address computed from an undefined handle has no meaningful value, but its
                // users load or store through it, so we rewrite those users instead.
                worklist.add(inst);
                return;
            }
            IRBuilder builder(module);
            builder.setInsertBefore(inst);
            auto undefinedValue = builder.emitLoadFromUninitializedMemory(inst->getFullType());
            inst->replaceUsesWith(undefinedValue);
            if (isUndefinedOpaqueHandle(undefinedValue))
                worklist.add(undefinedValue);
        }
        inst->removeAndDeallocate();
    }

    /// Replace each texture or sampler projection of `combined`, a `makeCombinedTextureSampler`,
    /// with the corresponding operand. Each projection is forwarded to its own operand, so a
    /// defined texture or sampler keeps its value when `combined` itself is replaced.
    void forwardCombinedTextureSamplerProjections(IRInst* combined)
    {
        List<IRInst*> projections;
        for (auto use = combined->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (user->getOp() == kIROp_CombinedTextureSamplerGetTexture ||
                user->getOp() == kIROp_CombinedTextureSamplerGetSampler)
            {
                if (!projections.contains(user))
                    projections.add(user);
            }
        }
        for (auto projection : projections)
        {
            auto operand = combined->getOperand(
                projection->getOp() == kIROp_CombinedTextureSamplerGetTexture ? 0 : 1);
            projection->replaceUsesWith(operand);
            projection->removeAndDeallocate();
            // The operand has gained uses, so an undefined one is processed again.
            if (isUndefinedOpaqueHandle(operand))
                worklist.add(operand);
        }
    }

    /// Return whether `call`, which passes an undefined handle, may be inlined so that the callee's
    /// accesses through the handle become visible here. We respect `[noinline]`, and a call from a
    /// function to itself is not inlined; that also bounds inlining through a recursive cycle.
    static bool canInlineCall(IRCall* call)
    {
        auto callee = as<IRFunc>(call->getCallee());
        if (!callee || !callee->getFirstBlock() || callee == getParentFunc(call))
            return false;
        return !callee->findDecoration<IRNoInlineDecoration>();
    }

    /// Rewrite the uses of `value`, which is an undefined handle or an address into one.
    ///
    /// A use that reads or writes through `value` is replaced by an undefined result, or removed
    /// if it has no result. That covers a `SPIRVAsm` block, a dedicated resource read such as
    /// `ImageGatherOffset`, a load, store or atomic through an address, and an operation that
    /// derives another handle. A call that passes `value` is queued for inlining. A use that only
    /// passes `value` on, such as a `select`, a block argument or a store of the handle itself into
    /// a variable, is left as it is; `diagnoseUndefinedOpaqueHandles` reports a handle that such a
    /// use keeps alive.
    void processValue(IRInst* value)
    {
        bool isAddress = as<IRPtrTypeBase>(value->getDataType()) != nullptr;

        // Rewriting a user removes its uses, so we collect the users first.
        List<IRInst*> accessingInsts;
        List<IRInst*> debugInsts;
        HashSet<IRInst*> seen;
        for (auto use = value->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            IRInst* accessingInst = nullptr;
            if (auto operand = as<IRSPIRVAsmOperand>(user))
                accessingInst = operand->getAsmBlock();
            else if (isDebugInfoInst(user))
            {
                if (seen.add(user))
                    debugInsts.add(user);
                continue;
            }
            else if (auto call = as<IRCall>(user))
            {
                if (use != call->getCalleeUse() && canInlineCall(call) && seen.add(call))
                    callsToInline.add(call);
                continue;
            }
            else if (isAddress)
            {
                // Through an address, every use of it as operand 0 (a load, store, atomic or
                // further projection) accesses the resource. A store of the address itself as a
                // value is passed on instead.
                if (use == user->getOperands() && !as<IRTerminatorInst>(user))
                    accessingInst = user;
            }
            else if (
                doesOpReadResourceContents(user->getOp()) ||
                doesOpDeriveOpaqueHandle(user->getOp()) ||
                (doesOpProduceResourceContentAddress(user->getOp()) && use == user->getOperands()))
            {
                accessingInst = user;
            }
            if (accessingInst && seen.add(accessingInst))
                accessingInsts.add(accessingInst);
        }

        // A debug instruction that records the value of an undefined handle carries no
        // information, and would otherwise keep the handle alive.
        for (auto debugInst : debugInsts)
            debugInst->removeAndDeallocate();
        for (auto inst : accessingInsts)
            replaceWithUndefinedValue(inst);
        if (!value->hasUses())
            value->removeAndDeallocate();
    }
};

void eliminateUsesOfUndefinedOpaqueHandles(IRModule* module)
{
    UndefinedOpaqueHandleContext context;
    context.module = module;

    // Inlining a call copies the callee's body, including any undefined handles of its own, into
    // the caller, so after each round of inlining we look for undefined handles again.
    for (;;)
    {
        forEachUndefinedOpaqueHandle(module, [&](IRInst* handle) { context.worklist.add(handle); });
        while (context.worklist.getCount())
        {
            auto value = context.worklist.getLast();
            context.worklist.removeLast();
            context.processValue(value);
        }
        // A call that passes several undefined handles is queued once for each of them.
        bool inlinedAnyCall = false;
        HashSet<IRCall*> inlinedCalls;
        for (auto call : context.callsToInline)
        {
            if (inlinedCalls.add(call))
                inlinedAnyCall |= inlineCall(call);
        }
        context.callsToInline.clear();
        if (!inlinedAnyCall)
            break;
    }
}

void diagnoseUndefinedOpaqueHandles(IRModule* module, DiagnosticSink* sink)
{
    forEachUndefinedOpaqueHandle(
        module,
        [&](IRInst* handle)
        {
            sink->diagnose(Diagnostics::UseOfUninitializedOpaqueHandle{
                .handleType = handle->getDataType(),
                .location = findBestSourceLocFromUses(handle)});
        });
}
} // namespace Slang
