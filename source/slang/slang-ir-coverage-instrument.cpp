#include "slang-ir-coverage-instrument.h"

#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-target.h"

#include <limits>

namespace Slang
{

namespace
{

// Well-known buffer name. Surfaced via `IRNameHintDecoration` and
// matches the manifest / sidecar key that hosts read.
static const char kCoverageBufferName[] = "__slang_coverage";

// Choose the resource kind under which the coverage buffer is bound
// for `target`. Each target family speaks a different vocabulary
// for buffer slots, and parameter binding only routes a kind through
// to the emitter that the emitter actually handles:
//   - D3D / HLSL: `UnorderedAccess` (UAV register `u<N>`)
//   - Metal: `MetalBuffer` (`[[buffer(N)]]`)
//   - Khronos / Vulkan / SPIR-V / GLSL / WebGPU / CPU / CUDA:
//     `DescriptorTableSlot`
// Setting the wrong kind triggers either an "unhandled HLSL register
// type" emit error, a SPIR-V "conflicting resource uses" assertion,
// or — on Metal — a silent failure to assign `[[buffer(N)]]` to the
// synthesized parameter.
static LayoutResourceKind selectCoverageResourceKind(TargetRequest* targetRequest)
{
    if (isD3DTarget(targetRequest))
        return LayoutResourceKind::UnorderedAccess;
    if (isMetalTarget(targetRequest))
        return LayoutResourceKind::MetalBuffer;
    return LayoutResourceKind::DescriptorTableSlot;
}

// Locate a user-declared global parameter whose name hint matches
// `__slang_coverage`. The IR coverage pass otherwise unconditionally
// synthesizes its own buffer with that name, which would silently
// shadow a user declaration; the caller surfaces a diagnostic when
// this returns non-null.
static IRGlobalParam* findUserDeclaredCoverageBuffer(IRModule* module)
{
    for (auto inst : module->getGlobalInsts())
    {
        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;
        auto nameHint = param->findDecoration<IRNameHintDecoration>();
        if (!nameHint)
            continue;
        if (nameHint->getName() == UnownedTerminatedStringSlice(kCoverageBufferName))
            return param;
    }
    return nullptr;
}

// Locate a global parameter whose layout already occupies the given
// `(space, binding)` pair for resource kind `kind`. Returns null if
// no collision. Used by the explicit-binding path of
// `instrumentCoverage` to reject `-trace-coverage-binding` values
// that would emit two parameters at the same slot — DXC and
// spirv-val both reject the resulting program with cryptic errors,
// so a Slang-level diagnostic anchored at the colliding declaration
// is more debuggable.
static IRGlobalParam* findCollidingParam(
    IRModule* module,
    LayoutResourceKind kind,
    int space,
    int binding)
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
        UInt paramSpace = 0;
        if (auto a = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
            paramSpace = a->getOffset();
        if ((int)paramSpace != space)
            continue;
        if (auto a = varLayout->findOffsetAttr(kind))
        {
            if ((int)a->getOffset() == binding)
                return param;
        }
    }
    return nullptr;
}

// Pick a binding offset in space 0 that doesn't collide with any
// existing global param's offset for `kind`. Walks the module once
// and returns max-occupied + 1, or 0 when nothing else claims a slot
// of that kind in space 0.
static int pickFreeBindingForCoverage(IRModule* module, LayoutResourceKind kind)
{
    int maxOccupied = -1;
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
        UInt paramSpace = 0;
        if (auto a = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
            paramSpace = a->getOffset();
        if (paramSpace != 0)
            continue;
        if (auto a = varLayout->findOffsetAttr(kind))
        {
            int offset = (int)a->getOffset();
            if (offset > maxOccupied)
                maxOccupied = offset;
        }
    }
    // Guard against signed overflow on a malformed module that pins a
    // user binding at INT_MAX; `INT_MAX + 1` is undefined behavior in
    // C++ and would wrap to `INT_MIN`, producing a negative offset
    // downstream. Returning -1 lets callers signal the failure
    // explicitly.
    if (maxOccupied == std::numeric_limits<int>::max())
        return -1;
    return maxOccupied + 1;
}

// Construct the layout for the synthesized coverage buffer at the
// given (space, binding) for `kind`. One resource kind per layout —
// pre-target — matches what AST→IR lowering produces for a buffer
// declared with `register(uN, spaceM)` (D3D) or
// `[[vk::binding(N, M)]]` (Vulkan).
static IRVarLayout* createCoverageBufferVarLayout(
    IRBuilder& builder,
    TargetRequest* targetRequest,
    LayoutResourceKind kind,
    int space,
    int binding)
{
    IRTypeLayout::Builder typeLayoutBuilder(&builder);
    typeLayoutBuilder.addResourceUsage(kind, 1);
    // On targets that pack global parameters into a single struct
    // (CPU/CUDA), `collectGlobalUniformParameters` only picks up
    // fields whose type layout reports `Uniform` usage. Without this
    // the synthesized buffer would skip packing and reappear as a
    // standalone kernel parameter, breaking the standard 3-arg ABI
    // (varying, entryPointParams, globalParams). Graphics targets
    // (D3D / Khronos) reject the entire packaging step earlier and
    // are unaffected.
    if (isCPUTarget(targetRequest) || isCUDATarget(targetRequest))
    {
        typeLayoutBuilder.addResourceUsage(LayoutResourceKind::Uniform, sizeof(void*));
    }
    auto typeLayout = typeLayoutBuilder.build();

    IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
    varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::RegisterSpace)->offset = (UInt)space;
    varLayoutBuilder.findOrAddResourceInfo(kind)->offset = (UInt)binding;
    return varLayoutBuilder.build();
}

// Synthesize the coverage buffer as a fresh `IRGlobalParam` in the
// linked module. No AST decl exists for this buffer; it enters the
// pipeline at IR time so user-facing AST/reflection paths never see
// it. Backend emit treats it identically to any other
// `RWStructuredBuffer<uint>` global param.
static IRGlobalParam* synthesizeCoverageBuffer(
    IRModule* module,
    TargetRequest* targetRequest,
    int explicitBinding,
    int explicitSpace,
    int& outSpace,
    int& outBinding)
{
    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());

    auto kind = selectCoverageResourceKind(targetRequest);

    // Resolve `(space, binding)` before allocating any IR so a
    // failure (auto-allocator can't find a free slot) leaves no
    // orphan inst behind.
    int space, binding;
    if (explicitSpace >= 0 && explicitBinding >= 0)
    {
        space = explicitSpace;
        binding = explicitBinding;
    }
    else
    {
        space = 0;
        binding = pickFreeBindingForCoverage(module, kind);
        if (binding < 0)
            return nullptr;
    }

    IRType* uintType = builder.getUIntType();
    IRInst* typeOperands[2] = {uintType, builder.getType(kIROp_DefaultBufferLayoutType)};
    auto bufferType = (IRType*)builder.getType(kIROp_HLSLRWStructuredBufferType, 2, typeOperands);

    auto param = builder.createGlobalParam(bufferType);
    builder.addNameHintDecoration(param, UnownedTerminatedStringSlice(kCoverageBufferName));

    auto varLayout = createCoverageBufferVarLayout(builder, targetRequest, kind, space, binding);
    builder.addLayoutDecoration(param, varLayout);

    outSpace = space;
    outBinding = binding;
    return param;
}

// Find the inner `IRStructTypeLayout` inside a program-scope var
// layout. The scope layout may be a bare struct or wrapped in a
// parameter group (`ConstantBuffer<...>`), depending on target
// policy; the field-attr list we want to extend lives on the inner
// struct in either case.
static IRStructTypeLayout* findScopeStructTypeLayout(IRVarLayout* scopeVarLayout)
{
    auto typeLayout = scopeVarLayout->getTypeLayout();
    if (auto groupLayout = as<IRParameterGroupTypeLayout>(typeLayout))
        typeLayout = groupLayout->getElementVarLayout()->getTypeLayout();
    return as<IRStructTypeLayout>(typeLayout);
}

// Rebuild the program-scope var layout to include our synthesized
// buffer as an additional struct field. The field's key is the
// buffer's `IRGlobalParam` itself (the convention
// `collectGlobalUniformParameters` decodes). When the scope layout
// involves a parameter group wrapper, we rebuild the inner element
// var layout and the wrapper as well so the var layout's invariants
// hold.
//
// Returns the new scope var layout. If the existing layout shape
// can't be extended (no struct layout to add to), returns the input
// unchanged — the buffer still lives as a standalone `IRGlobalParam`,
// which is correct for graphics targets that don't pack globals.
static IRVarLayout* extendScopeLayoutWithCoverageBuffer(
    IRBuilder& builder,
    IRVarLayout* oldScopeVarLayout,
    IRGlobalParam* coverageBuffer)
{
    if (!oldScopeVarLayout)
        return oldScopeVarLayout;
    auto oldStructTypeLayout = findScopeStructTypeLayout(oldScopeVarLayout);
    if (!oldStructTypeLayout)
        return oldScopeVarLayout;

    // Today `synthesizeCoverageBuffer` always attaches the layout
    // decoration before this is called; if a future refactor splits
    // or reorders those steps the decoration could be temporarily
    // absent. Return the unchanged scope layout in that case rather
    // than null-dereferencing — a `SLANG_ASSERT` would only catch
    // this in debug builds.
    auto layoutDecor = coverageBuffer->findDecoration<IRLayoutDecoration>();
    if (!layoutDecor)
        return oldScopeVarLayout;
    auto coverageVarLayout = cast<IRVarLayout>(layoutDecor->getLayout());

    // Build a new struct type layout: copy old fields, then add ours.
    IRStructTypeLayout::Builder newStructTypeLayoutBuilder(&builder);
    newStructTypeLayoutBuilder.addResourceUsageFrom(oldStructTypeLayout);
    for (auto oldFieldAttr : oldStructTypeLayout->getFieldLayoutAttrs())
        newStructTypeLayoutBuilder.addField(oldFieldAttr->getFieldKey(), oldFieldAttr->getLayout());
    newStructTypeLayoutBuilder.addField(coverageBuffer, coverageVarLayout);
    auto newStructTypeLayout = newStructTypeLayoutBuilder.build();

    // Rewrap: if the old layout had a parameter-group wrapper, build
    // a new wrapper around the new struct; otherwise the new struct
    // *is* the new outer type layout.
    IRTypeLayout* newOuterTypeLayout = newStructTypeLayout;
    auto oldOuterTypeLayout = oldScopeVarLayout->getTypeLayout();
    if (auto oldGroupLayout = as<IRParameterGroupTypeLayout>(oldOuterTypeLayout))
    {
        auto oldElementVarLayout = oldGroupLayout->getElementVarLayout();
        IRVarLayout::Builder newElementVarLayoutBuilder(&builder, newStructTypeLayout);
        newElementVarLayoutBuilder.cloneEverythingButOffsetsFrom(oldElementVarLayout);
        for (auto oldResInfo : oldElementVarLayout->getOffsetAttrs())
        {
            auto newResInfo =
                newElementVarLayoutBuilder.findOrAddResourceInfo(oldResInfo->getResourceKind());
            newResInfo->offset = oldResInfo->getOffset();
            newResInfo->space = oldResInfo->getSpace();
        }
        auto newElementVarLayout = newElementVarLayoutBuilder.build();

        IRParameterGroupTypeLayout::Builder newGroupBuilder(&builder);
        newGroupBuilder.setContainerVarLayout(oldGroupLayout->getContainerVarLayout());
        newGroupBuilder.setElementVarLayout(newElementVarLayout);
        newGroupBuilder.setOffsetElementTypeLayout(newStructTypeLayout);
        newOuterTypeLayout = newGroupBuilder.build();
    }

    // Rebuild the outer var layout pointing at the new outer type
    // layout, preserving offsets/semantics from the old.
    IRVarLayout::Builder newScopeVarLayoutBuilder(&builder, newOuterTypeLayout);
    newScopeVarLayoutBuilder.cloneEverythingButOffsetsFrom(oldScopeVarLayout);
    for (auto oldResInfo : oldScopeVarLayout->getOffsetAttrs())
    {
        auto newResInfo =
            newScopeVarLayoutBuilder.findOrAddResourceInfo(oldResInfo->getResourceKind());
        newResInfo->offset = oldResInfo->getOffset();
        newResInfo->space = oldResInfo->getSpace();
    }
    return newScopeVarLayoutBuilder.build();
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
            // `findInnerMostGenericReturnVal` peels off all nested
            // `IRGeneric` layers (multi-parameter generics lower to
            // `Generic<T> { Generic<U> { Func } }`); a single layer
            // would miss the function in those cases.
            if (auto inner = as<IRFunc>(findInnerMostGenericReturnVal(generic)))
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

        // The counter op has void return type and, by construction, no uses.
        // Catch a future IR transform that takes a use of it before we reach
        // this removal — the op would otherwise be silently dropped here.
        SLANG_ASSERT(!counterOp->hasUses());
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
    int explicitBinding,
    int explicitSpace,
    TargetRequest* targetRequest,
    IRVarLayout*& globalScopeVarLayout,
    ArtifactPostEmitMetadata& outMetadata)
{
    // Collect any counter ops so stale ones from cached modules can't
    // leak into the backend when the flag is off.
    List<IRInst*> counterOps;
    collectCoverageCounterOps(module, counterOps);

    if (!enabled)
    {
        // Flag off: drop any counter ops without emitting atomics or
        // synthesizing a buffer.
        for (auto op : counterOps)
            op->removeAndDeallocate();
        return;
    }

    if (counterOps.getCount() == 0)
        return;

    // WGSL requires the buffer's element type to be `atomic<u32>` for
    // atomic ops; the IR coverage pass synthesizes a plain
    // `RWStructuredBuffer<uint>` which lowers to `array<u32>` on WGSL
    // and fails WGSL validation at the `atomicAdd` call. Until the
    // synthesized type is wrapped in `Atomic<...>` for WGSL targets,
    // skip instrumentation with a clear warning rather than emitting
    // invalid WGSL. Other backends are unaffected. (For WebGPU
    // workflows that need coverage today, `-target spirv` works via
    // the SPIR-V → WebGPU path.)
    if (isWGPUTarget(targetRequest))
    {
        if (sink)
            sink->diagnose(Diagnostics::CoverageTargetNotSupported{});
        for (auto op : counterOps)
            op->removeAndDeallocate();
        return;
    }

    // Surface a warning if the user has declared a global parameter
    // named `__slang_coverage`. The IR coverage pass synthesizes its
    // own buffer with that name and the user declaration is silently
    // shadowed (no counter writes ever target it). Reserving the name
    // explicitly avoids a class of confusing wrong-coverage outcomes.
    if (sink)
    {
        if (auto userBuffer = findUserDeclaredCoverageBuffer(module))
            sink->diagnose(
                Diagnostics::CoverageBufferReservedName{.location = userBuffer->sourceLoc});
    }

    // When the user explicitly pins the coverage buffer with
    // `-trace-coverage-binding`, bail with an actionable diagnostic
    // if the requested slot is already in use. The auto-allocation
    // path already avoids collisions; the explicit path used to take
    // the user's value at face value, leaving DXC / spirv-val to
    // reject the resulting two-params-at-one-slot program with a
    // cryptic downstream error.
    if (sink && explicitSpace >= 0 && explicitBinding >= 0)
    {
        auto kind = selectCoverageResourceKind(targetRequest);
        if (auto colliding = findCollidingParam(module, kind, explicitSpace, explicitBinding))
        {
            sink->diagnose(Diagnostics::CoverageBindingCollision{.location = colliding->sourceLoc});
            return;
        }
    }

    int chosenSpace = -1;
    int chosenBinding = -1;
    auto buffer = synthesizeCoverageBuffer(
        module,
        targetRequest,
        explicitBinding,
        explicitSpace,
        chosenSpace,
        chosenBinding);

    // `synthesizeCoverageBuffer` returns nullptr when the
    // auto-allocator can't find a free binding (existing globals
    // occupy slots up to INT_MAX in space 0 — pathological but
    // possible). Diagnose loudly and drop the queued counter ops so
    // the rest of the compile still runs without coverage.
    if (!buffer)
    {
        if (sink)
            sink->diagnose(Diagnostics::CoverageBindingExhausted{});
        for (auto op : counterOps)
            op->removeAndDeallocate();
        return;
    }

    // Extend the program-scope layout so the buffer participates in
    // global-uniform packaging on targets that pack (CPU, CUDA). On
    // graphics targets the scope layout typically has no struct to
    // extend; the helper is a no-op there and the buffer remains a
    // standalone `IRGlobalParam`.
    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto newScopeVarLayout =
        extendScopeLayoutWithCoverageBuffer(builder, globalScopeVarLayout, buffer);
    if (newScopeVarLayout != globalScopeVarLayout)
    {
        globalScopeVarLayout = newScopeVarLayout;
        // The module inst also carries the scope layout as a layout
        // decoration; refresh that so subsequent passes that read
        // from the module rather than the linked-IR struct see the
        // new layout. Replacing the decoration's payload requires
        // removing the old and adding the new.
        if (auto oldDecor = module->getModuleInst()->findDecoration<IRLayoutDecoration>())
            oldDecor->removeAndDeallocate();
        builder.addLayoutDecoration(module->getModuleInst(), newScopeVarLayout);
    }

    outMetadata.m_coverageBufferSpace = chosenSpace;
    outMetadata.m_coverageBufferBinding = chosenBinding;

    CoverageInstrumenter instrumenter(
        module,
        buffer,
        sink ? sink->getSourceManager() : nullptr,
        outMetadata);
    instrumenter.run(counterOps);
}

} // namespace Slang
