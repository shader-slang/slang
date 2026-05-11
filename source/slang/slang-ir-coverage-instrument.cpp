#include "slang-ir-coverage-instrument.h"

#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
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
static const char kGlobalParamsName[] = "globalParams";
// Synthetic resource ids are stable, non-zero feature-local
// constants. Additional synthetic instrumentation resources should
// claim their own ids alongside this one.
static const uint32_t kCoverageSyntheticResourceID = 1;

static bool hasNameHint(IRInst* inst, UnownedTerminatedStringSlice expectedName)
{
    if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
        return nameHint->getName() == expectedName;
    return false;
}

static SyntheticResourceRecord* findSyntheticResourceRecordById(
    ArtifactPostEmitMetadata& metadata,
    uint32_t id)
{
    for (auto& record : metadata.m_syntheticResources)
    {
        if (record.id == id)
            return &record;
    }
    return nullptr;
}

static SyntheticResourceRecord& getOrAddCoverageSyntheticResourceRecord(
    ArtifactPostEmitMetadata& metadata)
{
    SLANG_RELEASE_ASSERT(!metadata.m_syntheticResourcesPublished);
    if (auto existing = findSyntheticResourceRecordById(metadata, kCoverageSyntheticResourceID))
        return *existing;

    SyntheticResourceRecord record;
    record.id = kCoverageSyntheticResourceID;
    record.bindingType = slang::BindingType::MutableRawBuffer;
    record.arraySize = 1;
    record.scope = slang::SyntheticResourceScope::Global;
    record.access = slang::SyntheticResourceAccess::ReadWrite;
    record.entryPointIndex = -1;
    record.debugName = kCoverageBufferName;
    metadata.m_syntheticResources.add(record);
    return metadata.m_syntheticResources.getLast();
}

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
        if (hasNameHint(param, UnownedTerminatedStringSlice(kCoverageBufferName)))
            return param;
    }
    return nullptr;
}

static IRGlobalParam* findLastGlobalParamByName(
    IRModule* module,
    UnownedTerminatedStringSlice expectedName)
{
    IRGlobalParam* result = nullptr;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto param = as<IRGlobalParam>(inst))
        {
            if (hasNameHint(param, expectedName))
                result = param;
        }
    }
    return result;
}

static IRStructField* findLastStructFieldByName(
    IRType* type,
    UnownedTerminatedStringSlice expectedName)
{
    if (auto structType = as<IRStructType>(type))
    {
        IRStructField* result = nullptr;
        for (auto field : structType->getFields())
        {
            if (hasNameHint(field->getKey(), expectedName))
                result = field;
        }
        return result;
    }

    if (auto specialize = as<IRSpecialize>(type))
    {
        if (auto generic = as<IRGeneric>(specialize->getBase()))
        {
            if (auto genericStructType = as<IRStructType>(findInnerMostGenericReturnVal(generic)))
                return findLastStructFieldByName(genericStructType, expectedName);
        }
    }

    return nullptr;
}

struct UsedBindingRange
{
    UInt space = 0;
    UInt binding = 0;
    UInt count = 0;
    IRInst* source = nullptr;
};

static UInt getResourceCount(IRTypeLayout* typeLayout, LayoutResourceKind kind)
{
    if (!typeLayout)
        return 0;
    if (auto sizeAttr = typeLayout->findSizeAttr(kind))
        return (UInt)sizeAttr->getSize().getFiniteValueOr(0);
    return 0;
}

static IRInst* selectBindingSource(IRInst* candidate, IRInst* fallback)
{
    if (candidate && candidate->sourceLoc.isValid())
        return candidate;
    return fallback ? fallback : candidate;
}

static void collectUsedBindingsFromTypeLayout(
    IRTypeLayout* typeLayout,
    LayoutResourceKind kind,
    UInt baseBinding,
    UInt baseSpace,
    IRInst* source,
    List<UsedBindingRange>& outRanges);

static void addUsedBindingRange(
    List<UsedBindingRange>& outRanges,
    UInt space,
    UInt binding,
    UInt count,
    IRInst* source)
{
    if (count == 0)
        return;

    UsedBindingRange range;
    range.space = space;
    range.binding = binding;
    range.count = count;
    range.source = source;
    outRanges.add(range);
}

static void collectUsedBindingsFromVarLayout(
    IRVarLayout* varLayout,
    LayoutResourceKind kind,
    UInt baseBinding,
    UInt baseSpace,
    IRInst* source,
    List<UsedBindingRange>& outRanges)
{
    if (!varLayout)
        return;

    auto typeLayout = varLayout->getTypeLayout();

    UInt localBinding = baseBinding;
    UInt localSpace = baseSpace;
    if (auto registerSpaceAttr = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
        localSpace += registerSpaceAttr->getOffset();

    if (auto offsetAttr = varLayout->findOffsetAttr(kind))
    {
        localBinding += offsetAttr->getOffset();
        localSpace += offsetAttr->getSpace();

        UInt count = getResourceCount(typeLayout, kind);
        // If a layout carries an explicit resource-kind offset but
        // its type layout does not expose a finite size, reserve the
        // base slot rather than pretending the offset is unused.
        if (count == 0)
            count = 1;
        addUsedBindingRange(outRanges, localSpace, localBinding, count, source);
    }

    if (auto groupLayout = as<IRParameterGroupTypeLayout>(typeLayout))
    {
        UInt elementSpace = localSpace;
        if (auto subElementSpaceAttr =
                varLayout->findOffsetAttr(LayoutResourceKind::SubElementRegisterSpace))
        {
            elementSpace += subElementSpaceAttr->getOffset();
        }

        collectUsedBindingsFromVarLayout(
            groupLayout->getContainerVarLayout(),
            kind,
            localBinding,
            elementSpace,
            source,
            outRanges);
        collectUsedBindingsFromVarLayout(
            groupLayout->getElementVarLayout(),
            kind,
            localBinding,
            elementSpace,
            source,
            outRanges);
        return;
    }

    collectUsedBindingsFromTypeLayout(
        typeLayout,
        kind,
        localBinding,
        localSpace,
        source,
        outRanges);
}

static void collectUsedBindingsFromTypeLayout(
    IRTypeLayout* typeLayout,
    LayoutResourceKind kind,
    UInt baseBinding,
    UInt baseSpace,
    IRInst* source,
    List<UsedBindingRange>& outRanges)
{
    if (!typeLayout)
        return;

    if (auto structTypeLayout = as<IRStructTypeLayout>(typeLayout))
    {
        for (auto fieldAttr : structTypeLayout->getFieldLayoutAttrs())
        {
            auto fieldSource = selectBindingSource(fieldAttr->getFieldKey(), source);
            collectUsedBindingsFromVarLayout(
                fieldAttr->getLayout(),
                kind,
                baseBinding,
                baseSpace,
                fieldSource,
                outRanges);
        }
        return;
    }

    if (auto arrayTypeLayout = as<IRArrayTypeLayout>(typeLayout))
    {
        collectUsedBindingsFromTypeLayout(
            arrayTypeLayout->getElementTypeLayout(),
            kind,
            baseBinding,
            baseSpace,
            source,
            outRanges);
    }
}

static List<UsedBindingRange> collectUsedBindings(
    IRModule* module,
    IRVarLayout* globalScopeVarLayout,
    LayoutResourceKind kind)
{
    List<UsedBindingRange> ranges;

    collectUsedBindingsFromVarLayout(globalScopeVarLayout, kind, 0, 0, nullptr, ranges);

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
        collectUsedBindingsFromVarLayout(varLayout, kind, 0, 0, param, ranges);
    }

    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (!func)
            continue;
        auto layoutDecor = func->findDecoration<IRLayoutDecoration>();
        if (!layoutDecor)
            continue;
        auto entryPointLayout = as<IREntryPointLayout>(layoutDecor->getLayout());
        if (!entryPointLayout)
            continue;

        // Entry-point uniform parameters are not `IRGlobalParam`s yet
        // when coverage instrumentation runs. Later parameter-collection
        // passes flatten them into globals using this entry-point layout,
        // so the auto allocator must reserve their eventual slots here.
        collectUsedBindingsFromVarLayout(
            entryPointLayout->getParamsLayout(),
            kind,
            0,
            0,
            func,
            ranges);
        collectUsedBindingsFromVarLayout(
            entryPointLayout->getResultLayout(),
            kind,
            0,
            0,
            func,
            ranges);
    }

    return ranges;
}

static bool bindingRangeContains(const UsedBindingRange& range, int space, int binding)
{
    if (space < 0 || binding < 0)
        return false;
    if (range.space != (UInt)space || binding < (int)range.binding)
        return false;
    return (UInt)(binding - (int)range.binding) < range.count;
}

// Locate a global parameter whose layout already occupies the given
// `(space, binding)` pair for resource kind `kind`. Returns null if
// no collision. Used by the explicit-binding path of
// `instrumentCoverage` to reject `-trace-coverage-binding` values
// that would emit two parameters at the same slot — DXC and
// spirv-val both reject the resulting program with cryptic errors,
// so a Slang-level diagnostic anchored at the colliding declaration
// is more debuggable.
static IRInst* findCollidingParam(
    IRModule* module,
    IRVarLayout* globalScopeVarLayout,
    LayoutResourceKind kind,
    int space,
    int binding)
{
    auto ranges = collectUsedBindings(module, globalScopeVarLayout, kind);
    for (auto& range : ranges)
    {
        if (bindingRangeContains(range, space, binding))
            return range.source ? range.source : module->getModuleInst();
    }
    return nullptr;
}

// Pick a binding offset in space 0 that doesn't collide with any
// existing layout range for `kind`. Walks both the program-scope
// layout and the module globals, so nested parameter-group resources
// such as `ParameterBlock<T>` members are counted before later
// target legalization flattens them into concrete descriptor slots.
// Returns max-occupied + 1, or 0 when nothing else claims a slot of
// that kind in space 0.
static int pickFreeBindingForCoverage(
    IRModule* module,
    IRVarLayout* globalScopeVarLayout,
    LayoutResourceKind kind)
{
    int maxOccupied = -1;
    auto ranges = collectUsedBindings(module, globalScopeVarLayout, kind);
    for (auto& range : ranges)
    {
        if (range.space != 0 || range.count == 0)
            continue;
        if (range.binding > std::numeric_limits<UInt>::max() - (range.count - 1))
            return -1;
        UInt lastBinding = range.binding + range.count - 1;
        if (lastBinding > (UInt)std::numeric_limits<int>::max())
            return -1;
        if ((int)lastBinding > maxOccupied)
            maxOccupied = (int)lastBinding;
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
    IRVarLayout* globalScopeVarLayout,
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
        binding = pickFreeBindingForCoverage(module, globalScopeVarLayout, kind);
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

    // Metadata reports `space` as -1 on targets that have no
    // space/set dimension (Metal uses `[[buffer(N)]]` only). The IR
    // varLayout above still carries `space = 0` for Metal because the
    // emitter walks the layout's RegisterSpace offset; the public
    // `CoverageBufferInfo.space` is documented as -1 in those cases
    // and hosts shouldn't see a bogus 0.
    outSpace = isMetalTarget(targetRequest) ? -1 : space;
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

static bool tryNarrowToInt32(IRIntegerValue value, int32_t& outValue)
{
    if (value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max())
    {
        return false;
    }
    outValue = (int32_t)value;
    return true;
}

static bool tryFillUniformInfoFromField(
    TargetRequest* targetRequest,
    IRStructField* field,
    int32_t& outUniformOffset,
    int32_t& outUniformStride)
{
    if (!field || !targetRequest)
        return false;

    IRIntegerValue naturalOffset = 0;
    if (SLANG_FAILED(getNaturalOffset(targetRequest, field, &naturalOffset)))
        return false;

    IRSizeAndAlignment fieldSizeAlignment;
    if (SLANG_FAILED(
            getNaturalSizeAndAlignment(targetRequest, field->getFieldType(), &fieldSizeAlignment)))
    {
        return false;
    }

    if (fieldSizeAlignment.size == IRSizeAndAlignment::kIndeterminateSize)
        return false;

    int32_t narrowOffset = -1;
    int32_t narrowStride = 0;
    if (!tryNarrowToInt32(naturalOffset, narrowOffset) ||
        !tryNarrowToInt32(fieldSizeAlignment.getStride(), narrowStride))
    {
        return false;
    }

    outUniformOffset = narrowOffset;
    outUniformStride = narrowStride;
    return true;
}

static bool tryFillUniformInfoFromDirectBuffer(
    TargetRequest* targetRequest,
    IRGlobalParam* directBuffer,
    int32_t& outUniformOffset,
    int32_t& outUniformStride)
{
    if (!directBuffer || !targetRequest)
        return false;

    IRSizeAndAlignment bufferSizeAlignment;
    if (SLANG_FAILED(getNaturalSizeAndAlignment(
            targetRequest,
            directBuffer->getDataType(),
            &bufferSizeAlignment)))
    {
        return false;
    }

    if (bufferSizeAlignment.size == IRSizeAndAlignment::kIndeterminateSize)
        return false;

    int32_t narrowStride = 0;
    if (!tryNarrowToInt32(bufferSizeAlignment.getStride(), narrowStride))
        return false;

    outUniformOffset = 0;
    outUniformStride = narrowStride;
    return true;
}

static IRType* getParameterGroupElementType(IRType* type)
{
    if (auto parameterGroupType = as<IRParameterGroupType>(type))
        return cast<IRType>(parameterGroupType->getOperand(0));
    return type;
}

static bool tryGetCoverageUniformBindingInfo(
    IRModule* module,
    TargetRequest* targetRequest,
    int32_t& outUniformOffset,
    int32_t& outUniformStride)
{
    outUniformOffset = -1;
    outUniformStride = 0;

    if (!targetRequest || !(isCPUTarget(targetRequest) || isCUDATarget(targetRequest)))
        return false;

    if (auto wrapperParam =
            findLastGlobalParamByName(module, UnownedTerminatedStringSlice(kGlobalParamsName)))
    {
        auto wrapperValueType = getParameterGroupElementType(wrapperParam->getDataType());
        if (auto field = findLastStructFieldByName(
                wrapperValueType,
                UnownedTerminatedStringSlice(kCoverageBufferName)))
        {
            if (tryFillUniformInfoFromField(
                    targetRequest,
                    field,
                    outUniformOffset,
                    outUniformStride))
            {
                return true;
            }
        }
    }

    if (auto directBuffer =
            findLastGlobalParamByName(module, UnownedTerminatedStringSlice(kCoverageBufferName)))
    {
        return tryFillUniformInfoFromDirectBuffer(
            targetRequest,
            directBuffer,
            outUniformOffset,
            outUniformStride);
    }

    return false;
}

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
    // path uses the same layout-derived occupancy query; the
    // explicit path used to take the user's value at face value,
    // leaving DXC / spirv-val to reject the resulting
    // two-params-at-one-slot program with a cryptic downstream error.
    //
    // The collision check itself runs regardless of whether `sink` is
    // available — correctness must not depend on diagnostic plumbing.
    // The diagnostic emission is gated on `sink`, but the IR cleanup
    // (drop queued counter ops, return without synthesizing) happens
    // unconditionally so the resulting program is well-formed.
    if (explicitSpace >= 0 && explicitBinding >= 0)
    {
        auto kind = selectCoverageResourceKind(targetRequest);
        if (auto colliding = findCollidingParam(
                module,
                globalScopeVarLayout,
                kind,
                explicitSpace,
                explicitBinding))
        {
            if (sink)
                sink->diagnose(
                    Diagnostics::CoverageBindingCollision{.location = colliding->sourceLoc});
            for (auto op : counterOps)
                op->removeAndDeallocate();
            return;
        }
    }

    int chosenSpace = -1;
    int chosenBinding = -1;
    auto buffer = synthesizeCoverageBuffer(
        module,
        targetRequest,
        globalScopeVarLayout,
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

    // Extend the program-scope layout only for targets that pack
    // global parameters into a host-visible aggregate. Graphics
    // targets should keep the coverage buffer as a standalone global
    // parameter; extending their scope layout can leave stale layout
    // keys behind when later resource legalization replaces the
    // synthesized parameter.
    if (isCPUTarget(targetRequest) || isCUDATarget(targetRequest))
    {
        IRBuilder builder(module);
        builder.setInsertInto(module->getModuleInst());
        auto newScopeVarLayout =
            extendScopeLayoutWithCoverageBuffer(builder, globalScopeVarLayout, buffer);
        if (newScopeVarLayout != globalScopeVarLayout)
        {
            globalScopeVarLayout = newScopeVarLayout;
            // The module inst also carries the scope layout as a
            // layout decoration; refresh that so subsequent passes
            // that read from the module rather than the linked-IR
            // struct see the new layout. Replacing the decoration's
            // payload requires removing the old and adding the new.
            if (auto oldDecor = module->getModuleInst()->findDecoration<IRLayoutDecoration>())
                oldDecor->removeAndDeallocate();
            builder.addLayoutDecoration(module->getModuleInst(), newScopeVarLayout);
        }
    }

    outMetadata.m_coverageBufferSpace = chosenSpace;
    outMetadata.m_coverageBufferBinding = chosenBinding;
    auto& syntheticResource = getOrAddCoverageSyntheticResourceRecord(outMetadata);
    syntheticResource.space = chosenSpace;
    syntheticResource.binding = chosenBinding;
    syntheticResource.uniformOffset = -1;
    syntheticResource.uniformStride = 0;

    CoverageInstrumenter instrumenter(
        module,
        buffer,
        sink ? sink->getSourceManager() : nullptr,
        outMetadata);
    instrumenter.run(counterOps);
}

void finalizeCoverageInstrumentationMetadata(
    IRModule* module,
    bool enabled,
    TargetRequest* targetRequest,
    ArtifactPostEmitMetadata& outMetadata)
{
    if (!enabled)
        return;

    auto record = findSyntheticResourceRecordById(outMetadata, kCoverageSyntheticResourceID);
    if (!record)
        return;

    int32_t uniformOffset = -1;
    int32_t uniformStride = 0;
    if (tryGetCoverageUniformBindingInfo(module, targetRequest, uniformOffset, uniformStride))
    {
        record->uniformOffset = uniformOffset;
        record->uniformStride = uniformStride;
    }
}

} // namespace Slang
