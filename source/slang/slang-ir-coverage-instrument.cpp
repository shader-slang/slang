#include "slang-ir-coverage-instrument.h"

#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

namespace
{

// The coverage buffer is synthesized as an AST-level VarDecl at
// semantic-check time by `maybeSynthesizeCoverageBufferDecl` (see
// slang-check-synthesize-coverage.cpp). By the time this IR pass
// runs, the buffer is a normal reflection-visible `IRGlobalParam`
// with a binding assigned by the regular parameter-binding phase —
// indistinguishable from a user-declared one. This pass only locates
// it by name and rewrites `IncrementCoverageCounter` ops against it.
static const char kCoverageBufferName[] = "__slang_coverage";

// Locate the coverage buffer's `IRGlobalParam`. Post-AST-synthesis
// the buffer is always present when tracing is enabled, but we still
// validate the type defensively so a user-declared `__slang_coverage`
// with the wrong shape produces a diagnostic rather than invalid IR.
static IRGlobalParam* findCoverageBuffer(IRModule* module, DiagnosticSink* sink)
{
    for (auto inst : module->getGlobalInsts())
    {
        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;
        auto nameHint = param->findDecoration<IRNameHintDecoration>();
        if (!nameHint || nameHint->getName() != UnownedStringSlice(kCoverageBufferName))
            continue;
        auto bufferType = as<IRHLSLRWStructuredBufferType>(param->getDataType());
        if (!bufferType)
        {
            if (sink)
                sink->diagnoseRaw(
                    Severity::Warning,
                    UnownedStringSlice("'__slang_coverage' must be 'RWStructuredBuffer<uint>' "
                                       "for -trace-coverage; ignoring."));
            return nullptr;
        }
        auto elementType = as<IRBasicType>(bufferType->getElementType());
        if (!elementType || elementType->getBaseType() != BaseType::UInt)
        {
            if (sink)
                sink->diagnoseRaw(
                    Severity::Warning,
                    UnownedStringSlice("'__slang_coverage' element type must be 'uint' for "
                                       "-trace-coverage; ignoring."));
            return nullptr;
        }
        return param;
    }
    return nullptr;
}

// Read the (space, binding) the parameter-binding pass assigned to
// the coverage buffer. These are the real values the reflection API
// will report and hosts will see via slang-rhi binding.
static void readBufferBinding(IRGlobalParam* coverageBuffer, int32_t& outSpace, int32_t& outBinding)
{
    outSpace = -1;
    outBinding = -1;
    auto layoutDecor = coverageBuffer->findDecoration<IRLayoutDecoration>();
    if (!layoutDecor)
        return;
    auto varLayout = as<IRVarLayout>(layoutDecor->getLayout());
    if (!varLayout)
        return;
    if (auto a = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
        outSpace = (int32_t)a->getOffset();
    if (auto a = varLayout->findOffsetAttr(LayoutResourceKind::DescriptorTableSlot))
        outBinding = (int32_t)a->getOffset();
    // For HLSL/D3D-style targets the binding comes in as UAV register.
    if (outBinding < 0)
    {
        if (auto a = varLayout->findOffsetAttr(LayoutResourceKind::UnorderedAccess))
            outBinding = (int32_t)a->getOffset();
    }
}

// Collect every IncrementCoverageCounter op in the module. Deterministic
// traversal: module-scope insts in declaration order, then each
// function's blocks in order, then each block's insts in position order.
static void collectCoverageCounterOps(IRModule* module, List<IRInst*>& out)
{
    auto visitFunc = [&](IRFunc* func)
    {
        for (auto block : func->getBlocks())
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
                if (inst->getOp() == kIROp_IncrementCoverageCounter)
                    out.add(inst);
    };

    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
            visitFunc(func);
        else if (auto generic = as<IRGeneric>(inst))
        {
            if (auto inner = as<IRFunc>(findGenericReturnVal(generic)))
                visitFunc(inner);
        }
    }
}

// Resolve a counter op's source position from its built-in sourceLoc.
// Returns false when the location is not valid, e.g. for synthetic/
// unknown sources.
static bool resolveHumaneLoc(
    SourceManager* sourceManager,
    IRInst* counterOp,
    String& outFile,
    uint32_t& outLine)
{
    if (!sourceManager || !counterOp->sourceLoc.isValid())
        return false;
    auto humane = sourceManager->getHumaneLoc(counterOp->sourceLoc, SourceLocType::Emit);
    if (humane.line <= 0)
        return false;
    outFile = humane.pathInfo.foundPath;
    outLine = (uint32_t)humane.line;
    return true;
}

struct CoverageInstrumenter
{
    IRModule* module;
    IRGlobalParam* coverageBuffer;
    SourceManager* sourceManager;
    ArtifactPostEmitMetadata& outMetadata;
    IRType* uintType;
    IRType* uintPtrType;
    IRType* intType;

    CoverageInstrumenter(
        IRModule* m,
        IRGlobalParam* buf,
        SourceManager* sm,
        ArtifactPostEmitMetadata& md)
        : module(m), coverageBuffer(buf), sourceManager(sm), outMetadata(md)
    {
        IRBuilder tmpBuilder(module);
        uintType = tmpBuilder.getUIntType();
        uintPtrType = tmpBuilder.getPtrType(uintType);
        intType = tmpBuilder.getIntType();
    }

    // Lower a single IncrementCoverageCounter op to an atomic add on
    // `coverageBuffer[slot]`. Appends a metadata entry for `slot` and
    // removes the op.
    void lowerCounterOp(IRInst* counterOp, UInt slot)
    {
        CoverageTracingEntry entry;
        resolveHumaneLoc(sourceManager, counterOp, entry.file, entry.line);
        outMetadata.m_coverageEntries.add(entry);

        IRBuilder builder(module);
        builder.setInsertBefore(counterOp);

        IRInst* getElemArgs[] = {
            coverageBuffer,
            builder.getIntValue(intType, (IRIntegerValue)slot),
        };
        IRInst* slotPtr = builder.emitIntrinsicInst(
            uintPtrType,
            kIROp_RWStructuredBufferGetElementPtr,
            2,
            getElemArgs);

        // Emit `AtomicAdd(slotPtr, 1, relaxed)` — lowered by each
        // backend emitter to its native atomic-increment idiom
        // (InterlockedAdd on HLSL, atomicAdd on GLSL, OpAtomicIAdd on
        // SPIR-V, etc.). Correct under GPU concurrency.
        IRInst* atomicArgs[] = {
            slotPtr,
            builder.getIntValue(uintType, 1),
            builder.getIntValue(intType, (IRIntegerValue)kIRMemoryOrder_Relaxed),
        };
        builder.emitIntrinsicInst(uintType, kIROp_AtomicAdd, 3, atomicArgs);

        counterOp->removeAndDeallocate();
    }

    void run(List<IRInst*> const& counterOps)
    {
        outMetadata.m_coverageEntries.reserve(counterOps.getCount());
        // Each counter op gets its own slot: the op's identity IS the
        // UID, and we assign a consecutive index in traversal order.
        // Multiple ops on the same source line get distinct slots; the
        // LCOV converter aggregates per (file, line) at the host side
        // via summation.
        for (UInt slot = 0; slot < (UInt)counterOps.getCount(); ++slot)
            lowerCounterOp(counterOps[slot], slot);
    }
};

} // anonymous namespace

void instrumentCoverage(
    IRModule* module,
    DiagnosticSink* sink,
    bool enabled,
    ArtifactPostEmitMetadata& outMetadata)
{
    // Collect any counter ops so stale ones from cached modules can't
    // leak into the backend when the flag is off.
    List<IRInst*> counterOps;
    collectCoverageCounterOps(module, counterOps);

    if (!enabled)
    {
        // Flag off: drop any counter ops without emitting atomics.
        for (auto op : counterOps)
            op->removeAndDeallocate();
        return;
    }

    if (counterOps.getCount() == 0)
        return;

    // The AST-time synthesizer guarantees `__slang_coverage` exists
    // when the flag is on (or the user declared one themselves).
    // If it doesn't, something earlier in the pipeline failed; bail
    // gracefully rather than producing invalid IR.
    auto buffer = findCoverageBuffer(module, sink);
    if (!buffer)
    {
        for (auto op : counterOps)
            op->removeAndDeallocate();
        return;
    }

    // Record the buffer's binding on the metadata so hosts can query
    // it via ICoverageTracingMetadata. Binding was assigned by the
    // normal parameter-binding phase, so it reflects whatever the
    // front-end chose (not a hardcoded reservation).
    readBufferBinding(
        buffer,
        outMetadata.m_coverageBufferSpace,
        outMetadata.m_coverageBufferBinding);

    CoverageInstrumenter instrumenter(
        module,
        buffer,
        sink ? sink->getSourceManager() : nullptr,
        outMetadata);
    instrumenter.run(counterOps);
}

} // namespace Slang
