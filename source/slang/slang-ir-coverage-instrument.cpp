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
static const uint32_t kCoverageSyntheticResourceID = uint32_t(SyntheticResourceKnownID::Coverage);

static bool hasNameHint(IRInst* inst, UnownedTerminatedStringSlice expectedName)
{
    if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
        return nameHint->getName() == expectedName;
    return false;
}

static Index findSyntheticResourceRecordIndexById(ArtifactPostEmitMetadata& metadata, uint32_t id)
{
    for (Index i = 0; i < metadata.m_syntheticResources.getCount(); ++i)
    {
        if (metadata.m_syntheticResources[i].id == id)
            return i;
    }
    return -1;
}

static Index getOrAddCoverageSyntheticResourceRecordIndex(ArtifactPostEmitMetadata& metadata)
{
    Index existingIndex =
        findSyntheticResourceRecordIndexById(metadata, kCoverageSyntheticResourceID);
    if (existingIndex >= 0)
        return existingIndex;

    SyntheticResourceRecord record;
    record.id = kCoverageSyntheticResourceID;
    SLANG_RELEASE_ASSERT(record.id != uint32_t(SyntheticResourceKnownID::None));
    record.bindingType = slang::BindingType::MutableRawBuffer;
    record.arraySize = 1;
    record.scope = slang::SyntheticResourceScope::Global;
    record.access = slang::SyntheticResourceAccess::ReadWrite;
    record.entryPointIndex = -1;
    record.debugName = kCoverageBufferName;
    metadata.m_syntheticResources.add(record);
    return metadata.m_syntheticResources.getCount() - 1;
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

static IRStructField* findStructFieldByKey(IRType* type, IRInst* expectedKey)
{
    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (field->getKey() == expectedKey)
                return field;
        }
        return nullptr;
    }

    if (auto specialize = as<IRSpecialize>(type))
    {
        if (auto generic = as<IRGeneric>(specialize->getBase()))
        {
            if (auto genericStructType = as<IRStructType>(findInnerMostGenericReturnVal(generic)))
                return findStructFieldByKey(genericStructType, expectedKey);
        }
    }

    return nullptr;
}

struct UsedBindingRange
{
    UInt space = 0;
    UInt binding = 0;
    UInt count = 0;
    bool isUnbounded = false;
    IRInst* source = nullptr;
};

static UInt getResourceCount(
    IRTypeLayout* typeLayout,
    LayoutResourceKind kind,
    bool& outIsUnbounded)
{
    outIsUnbounded = false;
    if (!typeLayout)
        return 0;
    if (auto sizeAttr = typeLayout->findSizeAttr(kind))
    {
        auto size = sizeAttr->getSize();
        if (size.isInfinite())
        {
            outIsUnbounded = true;
            return 1;
        }
        return (UInt)size.getFiniteValueOr(0);
    }
    return 0;
}

static IRInst* selectBindingSource(IRInst* candidate, IRInst* fallback)
{
    if (candidate && candidate->sourceLoc.isValid())
        return candidate;
    return fallback ? fallback : candidate;
}

static bool isUnsizedArrayResourceSource(IRInst* source)
{
    if (!source)
        return false;

    // Layout lowering currently records an unsized descriptor array as
    // an array type layout with a scalar resource count. Preserve the
    // source variable's IR type so explicit coverage bindings do not
    // get placed after a Vulkan variable-descriptor-count binding in
    // the same descriptor set.
    return as<IRUnsizedArrayType>(source->getDataType()) != nullptr;
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
    bool isUnbounded,
    IRInst* source)
{
    if (count == 0)
        return;

    UsedBindingRange range;
    range.space = space;
    range.binding = binding;
    range.count = count;
    range.isUnbounded = isUnbounded;
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

        bool isUnbounded = false;
        UInt count = getResourceCount(typeLayout, kind, isUnbounded);
        // If a layout carries an explicit resource-kind offset but
        // its type layout does not expose a finite size, reserve the
        // base slot rather than pretending the offset is unused.
        if (count == 0)
            count = 1;
        if (isUnsizedArrayResourceSource(source))
            isUnbounded = true;
        addUsedBindingRange(outRanges, localSpace, localBinding, count, isUnbounded, source);
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

static void collectUsedBindingsFromEntryPointParamsLayout(
    IRFunc* func,
    IRVarLayout* paramsLayout,
    LayoutResourceKind kind,
    List<UsedBindingRange>& outRanges)
{
    if (!paramsLayout)
        return;

    auto typeLayout = paramsLayout->getTypeLayout();
    auto structTypeLayout = as<IRStructTypeLayout>(typeLayout);
    if (!structTypeLayout)
    {
        collectUsedBindingsFromVarLayout(paramsLayout, kind, 0, 0, func, outRanges);
        return;
    }

    UInt baseBinding = 0;
    UInt baseSpace = 0;
    if (auto registerSpaceAttr = paramsLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
        baseSpace += registerSpaceAttr->getOffset();
    if (auto offsetAttr = paramsLayout->findOffsetAttr(kind))
    {
        baseBinding += offsetAttr->getOffset();
        baseSpace += offsetAttr->getSpace();
    }

    auto param = func->getFirstParam();
    for (auto fieldAttr : structTypeLayout->getFieldLayoutAttrs())
    {
        IRInst* source = param ? (IRInst*)param : (IRInst*)func;
        collectUsedBindingsFromVarLayout(
            fieldAttr->getLayout(),
            kind,
            baseBinding,
            baseSpace,
            source,
            outRanges);

        if (param)
            param = param->getNextParam();
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
        collectUsedBindingsFromEntryPointParamsLayout(
            func,
            entryPointLayout->getParamsLayout(),
            kind,
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
    if (range.isUnbounded)
        return true;
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
    IRInst* fallback = nullptr;
    bool foundCollision = false;
    for (auto& range : ranges)
    {
        if (bindingRangeContains(range, space, binding))
        {
            foundCollision = true;
            if (range.source && range.source->sourceLoc.isValid())
                return range.source;
            if (!fallback)
                fallback = range.source;
        }
    }
    if (!foundCollision)
        return nullptr;
    return fallback ? fallback : module->getModuleInst();
}

static int pickFreeBindingInSpace(List<UsedBindingRange>& ranges, UInt space)
{
    int maxOccupied = -1;
    for (auto& range : ranges)
    {
        if (range.space != space || range.count == 0)
            continue;
        if (range.isUnbounded)
            return -1;
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

static int pickSpaceAfterHighestUsedSpace(List<UsedBindingRange>& ranges)
{
    UInt maxSpace = 0;
    bool hasAnyRange = false;
    for (auto& range : ranges)
    {
        hasAnyRange = true;
        if (range.space > (UInt)std::numeric_limits<int>::max())
            return -1;
        if (range.space > maxSpace)
            maxSpace = range.space;
    }

    if (!hasAnyRange)
        return 0;
    if (maxSpace == (UInt)std::numeric_limits<int>::max())
        return -1;
    return int(maxSpace + 1);
}

static bool shouldHonorReservedCoverageSpaces(TargetRequest* targetRequest)
{
    return isKhronosTarget(targetRequest);
}

static void addReservedCoverageSpaces(
    List<UsedBindingRange>& ranges,
    const int* reservedSpaces,
    int reservedSpaceCount,
    IRInst* source)
{
    if (!reservedSpaces || reservedSpaceCount <= 0)
        return;

    List<int> uniqueSpaces;
    for (int i = 0; i < reservedSpaceCount; ++i)
    {
        const int space = reservedSpaces[i];
        if (space < 0)
            continue;
        bool alreadyAdded = false;
        for (auto uniqueSpace : uniqueSpaces)
        {
            if (uniqueSpace == space)
            {
                alreadyAdded = true;
                break;
            }
        }
        if (alreadyAdded)
            continue;
        uniqueSpaces.add(space);

        // Model a host-reserved descriptor/register space as an
        // unbounded range starting at binding 0. The allocator then
        // cannot place coverage into that space even if the current
        // shader IR does not reference any resource from the host's
        // pipeline layout.
        addUsedBindingRange(ranges, (UInt)space, 0, 1, true, source);
    }
}

// Pick a `(space, binding)` pair that doesn't collide with any
// existing layout range for `kind`. Walks program-scope, global, and
// entry-point layouts, so hidden resources such as `ParameterBlock<T>`
// members are counted before later target legalization flattens them
// into concrete descriptor slots.
static bool pickFreeBindingForCoverage(
    IRModule* module,
    TargetRequest* targetRequest,
    IRVarLayout* globalScopeVarLayout,
    LayoutResourceKind kind,
    const int* reservedSpaces,
    int reservedSpaceCount,
    int& outSpace,
    int& outBinding)
{
    auto ranges = collectUsedBindings(module, globalScopeVarLayout, kind);
    if (shouldHonorReservedCoverageSpaces(targetRequest))
    {
        addReservedCoverageSpaces(
            ranges,
            reservedSpaces,
            reservedSpaceCount,
            module->getModuleInst());
    }

    // For Vulkan/SPIR-V-style descriptor sets, do not extend an
    // existing set with a new binding, and do not fill holes between
    // existing sets. Many hosts build descriptor set layouts directly
    // from parameter blocks or other reflection data, so coverage is
    // appended as a new synthetic-resource set after all shader-visible
    // sets. If the shader has no visible sets, set 0 is the fresh set;
    // hosts with externally reserved set 0 should pass
    // `-trace-coverage-reserved-space 0` so allocation moves to set 1.
    // The complete location is reported through metadata.
    if (kind == LayoutResourceKind::DescriptorTableSlot && isKhronosTarget(targetRequest))
    {
        outSpace = pickSpaceAfterHighestUsedSpace(ranges);
        if (outSpace < 0)
            return false;
        outBinding = 0;
        return true;
    }

    outSpace = 0;
    outBinding = pickFreeBindingInSpace(ranges, 0);
    if (outBinding >= 0)
        return true;

    outSpace = pickSpaceAfterHighestUsedSpace(ranges);
    if (outSpace < 0)
        return false;
    outBinding = 0;
    return true;
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
    const int* reservedSpaces,
    int reservedSpaceCount,
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
        if (!pickFreeBindingForCoverage(
                module,
                targetRequest,
                globalScopeVarLayout,
                kind,
                reservedSpaces,
                reservedSpaceCount,
                space,
                binding))
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
    // synthetic resource metadata should not expose a bogus 0.
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

static IRType* getParameterGroupElementType(IRType* type)
{
    if (auto parameterGroupType = as<IRParameterGroupType>(type))
        return cast<IRType>(parameterGroupType->getOperand(0));
    return type;
}

static IRTypeLayout* getParameterGroupElementTypeLayout(IRTypeLayout* typeLayout)
{
    if (auto parameterGroupTypeLayout = as<IRParameterGroupTypeLayout>(typeLayout))
        return parameterGroupTypeLayout->getElementVarLayout()->getTypeLayout();
    return typeLayout;
}

static IRInst* findUniqueCoverageFieldKey(IRVarLayout* globalScopeVarLayout)
{
    if (!globalScopeVarLayout)
        return nullptr;

    auto globalScopeTypeLayout =
        getParameterGroupElementTypeLayout(globalScopeVarLayout->getTypeLayout());
    auto globalScopeStructLayout = as<IRStructTypeLayout>(globalScopeTypeLayout);
    if (!globalScopeStructLayout)
        return nullptr;

    IRInst* result = nullptr;
    for (auto fieldAttr : globalScopeStructLayout->getFieldLayoutAttrs())
    {
        auto key = fieldAttr->getFieldKey();
        if (!hasNameHint(key, UnownedTerminatedStringSlice(kCoverageBufferName)))
            continue;
        if (result)
            return nullptr;
        result = key;
    }
    return result;
}

static IRStructField* findUniqueStructFieldByKeyInGlobalParams(IRModule* module, IRInst* key)
{
    if (!key)
        return nullptr;

    IRStructField* result = nullptr;
    for (auto inst : module->getGlobalInsts())
    {
        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;

        auto wrapperValueType = getParameterGroupElementType(param->getDataType());
        auto field = findStructFieldByKey(wrapperValueType, key);
        if (!field)
            continue;
        if (result && result != field)
            return nullptr;
        result = field;
    }
    return result;
}

static bool tryGetCoverageUniformBindingInfo(
    IRModule* module,
    IRVarLayout* globalScopeVarLayout,
    TargetRequest* targetRequest,
    int32_t& outUniformOffset,
    int32_t& outUniformStride)
{
    outUniformOffset = -1;
    outUniformStride = 0;

    if (!targetRequest || !(isCPUTarget(targetRequest) || isCUDATarget(targetRequest)))
        return false;

    if (auto fieldKey = findUniqueCoverageFieldKey(globalScopeVarLayout))
    {
        if (auto field = findUniqueStructFieldByKeyInGlobalParams(module, fieldKey))
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

    return false;
}

} // anonymous namespace

void instrumentCoverage(
    IRModule* module,
    DiagnosticSink* sink,
    bool enabled,
    int explicitBinding,
    int explicitSpace,
    const int* reservedSpaces,
    int reservedSpaceCount,
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

    if (reservedSpaceCount > 0 && !shouldHonorReservedCoverageSpaces(targetRequest))
    {
        if (sink)
            sink->diagnose(Diagnostics::CoverageReservedSpaceIgnored{});
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
        reservedSpaces,
        reservedSpaceCount,
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

    Index syntheticResourceIndex = getOrAddCoverageSyntheticResourceRecordIndex(outMetadata);
    auto& syntheticResource = outMetadata.m_syntheticResources[syntheticResourceIndex];
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
    DiagnosticSink* sink,
    bool enabled,
    IRVarLayout* globalScopeVarLayout,
    TargetRequest* targetRequest,
    ArtifactPostEmitMetadata& outMetadata)
{
    if (!enabled)
        return;

    Index recordIndex =
        findSyntheticResourceRecordIndexById(outMetadata, kCoverageSyntheticResourceID);
    if (recordIndex < 0)
        return;

    auto& record = outMetadata.m_syntheticResources[recordIndex];
    int32_t uniformOffset = -1;
    int32_t uniformStride = 0;
    if (tryGetCoverageUniformBindingInfo(
            module,
            globalScopeVarLayout,
            targetRequest,
            uniformOffset,
            uniformStride))
    {
        record.uniformOffset = uniformOffset;
        record.uniformStride = uniformStride;
    }
    else if (targetRequest && (isCPUTarget(targetRequest) || isCUDATarget(targetRequest)))
    {
        if (sink)
            sink->diagnose(Diagnostics::CoverageUniformLayoutUnavailable{});
    }
}

} // namespace Slang
