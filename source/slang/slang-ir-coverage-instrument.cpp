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

static const char kCoverageBufferName[] = "__slang_coverage";

// Reserved register space + binding for the synthesized coverage
// buffer. Chosen to sit high enough that it is unlikely to collide
// with user resources. Will become configurable once the feature
// grows a proper binding-CLI option.
static const UInt kCoverageReservedSpace = 31;
static const UInt kCoverageReservedBinding = 0;

// Return true if any existing module-scope global parameter already
// occupies the reserved `(space, binding)` we would assign to the
// synthesized coverage buffer. Used to warn before synthesis rather
// than aliasing a user resource silently.
static bool hasResourceAtReservedBinding(IRModule* module)
{
    for (auto inst : module->getGlobalInsts())
    {
        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;
        auto layoutDecor = param->findDecoration<IRLayoutDecoration>();
        if (!layoutDecor)
            continue;
        auto varLayout = as<IRVarLayout>(layoutDecor->getLayout());
        if (!varLayout)
            continue;
        auto spaceAttr = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace);
        auto bindingAttr = varLayout->findOffsetAttr(LayoutResourceKind::DescriptorTableSlot);
        if (!spaceAttr || !bindingAttr)
            continue;
        if ((UInt)spaceAttr->getOffset() == kCoverageReservedSpace &&
            (UInt)bindingAttr->getOffset() == kCoverageReservedBinding)
            return true;
    }
    return false;
}

// Collect every IncrementCoverageCounter op in the module across all
// functions. Deterministic traversal order: module-scope insts in
// declaration order, then for each function its blocks in declaration
// order, then each block's instructions in position order.
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

// Synthesize an implicit `RWStructuredBuffer<uint> __slang_coverage`
// global parameter with a reserved layout. Follows the pattern in
// `lowerDynamicResourceHeap` — we manually build an IRTypeLayout /
// IRVarLayout so the buffer survives later uniform-collection passes.
// Emits a warning if any existing user resource already occupies the
// reserved binding; synthesis proceeds anyway but aliasing risk is
// documented to the user.
static IRGlobalParam* synthesizeCoverageBuffer(IRModule* module, DiagnosticSink* sink)
{
    if (hasResourceAtReservedBinding(module) && sink)
    {
        sink->diagnoseRaw(
            Severity::Warning,
            UnownedStringSlice("-trace-coverage: an existing resource occupies the reserved "
                               "binding (space=31, binding=0); coverage buffer may alias it."));
    }

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());

    auto uintType = builder.getUIntType();
    IRInst* elemOperand = uintType;
    auto bufferType = (IRType*)builder.getType(kIROp_HLSLRWStructuredBufferType, 1, &elemOperand);

    auto param = builder.createGlobalParam(bufferType);
    builder.addNameHintDecoration(param, UnownedStringSlice(kCoverageBufferName));

    IRTypeLayout::Builder typeLayoutBuilder(&builder);
    typeLayoutBuilder.addResourceUsage(LayoutResourceKind::DescriptorTableSlot, LayoutSize(1));
    auto typeLayout = typeLayoutBuilder.build();

    IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
    varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::RegisterSpace)->offset =
        kCoverageReservedSpace;
    varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::DescriptorTableSlot)->offset =
        kCoverageReservedBinding;
    auto varLayout = varLayoutBuilder.build();
    builder.addLayoutDecoration(param, varLayout);

    return param;
}

// Resolve a counter op's source position from its built-in sourceLoc.
// Returns false (with `outFile` / `outLine` untouched) when the
// location is not valid, e.g. for synthetic/unknown sources.
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
    // then removes the op.
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

    // Always synthesize the coverage buffer. Intentionally does not
    // look for a user-declared `__slang_coverage` — a reserved name
    // plus name-hint discovery is unreliable (e.g. after debug-info
    // stripping or name mangling), and binding conflicts are better
    // handled via a dedicated CLI option in a follow-up.
    auto buffer = synthesizeCoverageBuffer(module, sink);
    if (!buffer)
    {
        for (auto op : counterOps)
            op->removeAndDeallocate();
        return;
    }

    // Record the chosen binding on the metadata so hosts can query it
    // via ICoverageTracingMetadata without having to walk reflection.
    outMetadata.m_coverageBufferSpace = (int32_t)kCoverageReservedSpace;
    outMetadata.m_coverageBufferBinding = (int32_t)kCoverageReservedBinding;

    CoverageInstrumenter instrumenter(
        module,
        buffer,
        sink ? sink->getSourceManager() : nullptr,
        outMetadata);
    instrumenter.run(counterOps);
}

} // namespace Slang
