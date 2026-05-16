// slang-ir-byte-granularity-usage.cpp
//
// This pass produces a coarse, conservative answer to the question
// "for each global shader parameter that holds byte addressable uniform
// storage, which byte ranges does any reachable code actually read?"
//
// The output feeds into spIsParameterLocationUsed for the Uniform
// parameter category, which engines use to skip uploads or binds for
// storage that the shader never touches.
//
// Scope. We only walk parameters whose layout is wrapped in a parameter
// group (constant buffers, parameter blocks, and the implicit parameter
// group Slang synthesizes around loose top level uniforms). Other byte
// addressable globals like RT payloads or byte address buffer contents
// are out of scope because they do not have a useful per leaf access
// model in this representation. Unbounded byte regions are not tracked
// here either; instead we emit an isUntracked marker so the caller
// returns SLANG_E_NOT_AVAILABLE rather than a misleading false answer.
//
// Strategy. For each in scope param we walk forward through use chains
// from the global param. We peel FieldAddress and FieldExtract,
// GetElement and GetElementPtr against array or vector layouts, Swizzle,
// and Load. Any other op, and any non literal index, falls into the
// default branch which records the entire current sub region and stops
// descending. The recorded ranges are then coalesced (sort by offset,
// merge overlapping or adjacent) so that a shader with N reads of the
// same field does not produce N entries.
//
// Invariants. Field keys on FieldAddress and FieldExtract resolve to a
// layout entry; legalization passes that mint new keys also rewrite
// uses, so a miss is a real bug and we abort via SLANG_UNEXPECTED. For a
// parameter whose layout reports a finite uniform byte size, every
// nested field with uniform storage also has a finite size.
//
// Conservative on purpose. Runtime array indices widen to the whole
// array. Aggregate values flowing into a function call widen to the
// whole sub region (no interprocedural analysis). Matrix element access
// widens to the whole matrix; column major storage makes per element
// byte ranges discontiguous and that case is not yet handled. The
// function name carries Approximate to flag this. Engines should treat
// used true as authoritative and SLANG_E_NOT_AVAILABLE as may be used
// and bind it.
//
// Alternatives considered. (1) Reusing UsedRanges from
// slang-parameter-binding.cpp. That type is parameter binding specific
// (carries VarLayout per range for overlap diagnostics). We want
// overlapping reads to merge silently, so a thinner local representation
// is a better fit. (2) Tracking unbounded byte regions explicitly with
// a new range kind. The metadata coordinate space (spaceIndex,
// byteOffset) has no per param identity, so an unbounded entry would
// taint sibling CBs in the same space. The isUntracked marker is the
// same idea pushed down to a single per param decision.
// (3) Coalescing on every insert during the walk. Per call merging is
// O(N) per insert and burns time during the walk; deferring to a single
// sort and sweep at the end is O(N log N) total per param.
#include "slang-ir-byte-granularity-usage.h"

#include "slang-ir.h"

namespace Slang
{

// Reports the finite uniform byte size of typeLayout via outSize. A
// finite size of zero is a legitimate answer (the type has no uniform
// storage, e.g. a texture or sampler). Returns false for unbounded or
// invalid sizes; the caller decides what to do.
static bool _tryGetUniformByteSize(IRTypeLayout* typeLayout, UInt& outSize)
{
    // Every caller passes a layout obtained from another layout (an
    // element layout, field layout, or parameter group element). Those
    // accessors never legitimately return null in well formed IR. If
    // this fires, something upstream produced a malformed layout and
    // we want the loud failure rather than a silent zero.
    SLANG_RELEASE_ASSERT(typeLayout);
    outSize = 0;
    auto sizeAttr = typeLayout->findSizeAttr(LayoutResourceKind::Uniform);
    if (!sizeAttr)
        return true;
    auto size = sizeAttr->getSize();
    if (!size.isFinite())
        return false;
    outSize = UInt(size.getFiniteValue().getValidValue());
    return true;
}

// Looks up the layout for the named struct field. The IR maintains the
// invariant that any field key on a FieldAddress or FieldExtract
// resolves to a layout entry, so a miss here means a real bug upstream
// (a pass minted a new key without rewriting uses) and we abort.
static IRVarLayout* _getFieldVarLayout(IRStructTypeLayout* structLayout, IRInst* fieldKey)
{
    for (auto attr : structLayout->getFieldLayoutAttrs())
    {
        if (attr->getFieldKey() == fieldKey)
            return attr->getLayout();
    }
    SLANG_UNEXPECTED("field key not present in struct layout");
}

// Walks forward from inst through every use chain that stays inside the
// byte addressable region described by currentTypeLayout starting at
// currentOffset, recording byte intervals into outRanges as we go.
//
// Recognizes FieldAddress, FieldExtract, GetElement and GetElementPtr
// against array or vector layouts, Swizzle, and Load. Anything else,
// plus any non literal index, falls into the default branch which
// records the entire current sub region and stops descending. That
// conservative widening is bounded to the sub region we are looking at
// right now, so sibling fields are not affected.
//
// Each recognized case also has its own conservative fallbacks for
// situations where the access shape and the available layout shape do
// not line up (e.g. a FieldAddress against a region whose layout is
// not a struct, or a vector subscript whose base IR type is not a
// vector). In those cases we record the entire current region and
// stop descending. The pattern is always the same. Try to narrow.
// If we cannot narrow safely, widen to current and stop.
static void _collectByteRangesReadFromValue(
    IRInst* inst,
    IRTypeLayout* currentTypeLayout,
    UInt currentOffset,
    List<ByteGranularityUsageRange>& outRanges)
{
    // Bail when the region described by currentTypeLayout has no uniform
    // byte storage we can track. Two distinct cases. First, the type
    // genuinely has no uniform bytes (e.g. a texture or sampler embedded
    // in a struct we are recursing into) and there is nothing to record.
    // Second, the size is unbounded or invalid, which the param loop
    // already filters upstream, but bail defensively if we ever descend
    // into one through a struct field whose type slipped through.
    UInt currentSize = 0;
    if (!_tryGetUniformByteSize(currentTypeLayout, currentSize) || currentSize == 0)
        return;

    // For each use of inst, decide how that use narrows or widens the
    // region we are tracking. Recognized peelers (FieldAddress,
    // FieldExtract, GetElement, GetElementPtr, Swizzle, Load) recurse
    // into a sub region or record a more precise byte range. Anything
    // else hits the default branch which conservatively records the
    // entire current region and stops descending that chain.
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        IRInst* user = use->getUser();
        switch (user->getOp())
        {
        // Field access. Look up the field's layout in the current
        // struct, shift the running offset by the field's uniform
        // offset, and recurse into the field's sub region.
        case kIROp_FieldAddress:
        case kIROp_FieldExtract:
            {
                auto structLayout = as<IRStructTypeLayout>(currentTypeLayout);
                // Fallback. The IR is doing field access but the layout
                // we have for the current region is not a struct. Cannot
                // resolve a per field offset, so widen to current.
                if (!structLayout)
                {
                    outRanges.add({currentOffset, currentSize});
                    break;
                }
                auto fieldVar = _getFieldVarLayout(structLayout, user->getOperand(1));
                // If the field has unbounded uniform storage, we cannot
                // narrow further. Record the parent region and stop here.
                UInt fieldSize = 0;
                if (!_tryGetUniformByteSize(fieldVar->getTypeLayout(), fieldSize))
                {
                    outRanges.add({currentOffset, currentSize});
                    break;
                }
                UInt fieldByteOffset = 0;
                if (auto offsetAttr = fieldVar->findOffsetAttr(LayoutResourceKind::Uniform))
                    fieldByteOffset = offsetAttr->getOffset();
                _collectByteRangesReadFromValue(
                    user,
                    fieldVar->getTypeLayout(),
                    currentOffset + fieldByteOffset,
                    outRanges);
            }
            break;

        // Indexed access against an array or vector. Constant indices
        // narrow to one element. Runtime indices fall through to the
        // conservative whole region record. Vectors do not have a
        // dedicated IRTypeLayout subtype, so we detect them via the
        // base operand's IR type and compute element size from the
        // vector layout's total size.
        case kIROp_GetElementPtr:
        case kIROp_GetElement:
            {
                auto indexLit = as<IRIntLit>(user->getOperand(1));
                if (auto arrayLayout = as<IRArrayTypeLayout>(currentTypeLayout))
                {
                    auto elemLayout = arrayLayout->getElementTypeLayout();
                    UInt elemSize = 0;
                    bool elemFinite = _tryGetUniformByteSize(elemLayout, elemSize);
                    // Fallback. Runtime index, unbounded element type,
                    // or zero size element. Cannot resolve a single
                    // element to narrow to.
                    if (!indexLit || !elemFinite || elemSize == 0)
                    {
                        outRanges.add({currentOffset, currentSize});
                        break;
                    }
                    _collectByteRangesReadFromValue(
                        user,
                        elemLayout,
                        currentOffset + UInt(getIntVal(indexLit)) * elemSize,
                        outRanges);
                    break;
                }

                IRType* baseType = inst->getDataType();
                if (auto ptrType = as<IRPtrTypeBase>(baseType))
                    baseType = ptrType->getValueType();
                auto vectorType = as<IRVectorType>(baseType);
                auto elementCountLit =
                    vectorType ? as<IRIntLit>(vectorType->getElementCount()) : nullptr;
                // Fallback. Base is not a vector, element count is not
                // statically known, or the index is a runtime value.
                if (!vectorType || !elementCountLit || !indexLit)
                {
                    outRanges.add({currentOffset, currentSize});
                    break;
                }
                UInt elementCount = UInt(getIntVal(elementCountLit));
                // Fallback. Element count was a literal but came out as
                // zero (degenerate or malformed layout).
                if (elementCount == 0)
                {
                    outRanges.add({currentOffset, currentSize});
                    break;
                }
                UInt elementSize = currentSize / elementCount;
                outRanges.add(
                    {currentOffset + UInt(getIntVal(indexLit)) * elementSize, elementSize});
            }
            break;

        // Swizzle. The vector value is being read at one or more
        // component positions. Compute element size from the vector
        // layout, then record one byte range per swizzle component.
        case kIROp_Swizzle:
            {
                auto swizzle = static_cast<IRSwizzle*>(user);
                auto vectorType = as<IRVectorType>(inst->getDataType());
                auto elementCountLit =
                    vectorType ? as<IRIntLit>(vectorType->getElementCount()) : nullptr;
                // Fallback. The value being swizzled is not a vector, or
                // the vector's element count is not statically known.
                if (!vectorType || !elementCountLit)
                {
                    outRanges.add({currentOffset, currentSize});
                    break;
                }
                UInt elementCount = UInt(getIntVal(elementCountLit));
                // Fallback. Degenerate or malformed vector layout.
                if (elementCount == 0)
                {
                    outRanges.add({currentOffset, currentSize});
                    break;
                }
                UInt elementSize = currentSize / elementCount;
                bool anyDynamic = false;
                UInt swizzleCount = swizzle->getElementCount();
                for (UInt i = 0; i < swizzleCount; ++i)
                {
                    auto idxLit = as<IRIntLit>(swizzle->getElementIndex(i));
                    if (!idxLit)
                    {
                        anyDynamic = true;
                        break;
                    }
                }
                // Fallback. At least one swizzle index is a runtime
                // value, so we cannot tell which components were read.
                if (anyDynamic)
                {
                    outRanges.add({currentOffset, currentSize});
                    break;
                }
                for (UInt i = 0; i < swizzleCount; ++i)
                {
                    UInt idx = UInt(getIntVal(as<IRIntLit>(swizzle->getElementIndex(i))));
                    outRanges.add({currentOffset + idx * elementSize, elementSize});
                }
            }
            break;

        // Load. The loaded value still represents the same uniform
        // region, so recurse on its uses to see what gets read out of
        // it (typically a FieldExtract or Swizzle chain).
        case kIROp_Load:
            _collectByteRangesReadFromValue(user, currentTypeLayout, currentOffset, outRanges);
            break;

        // Anything else. The value or pointer escapes through an op we
        // do not peel (function call, store, cast, and so on). We
        // cannot know which bytes are read by a downstream consumer,
        // so conservatively widen to the entire current region.
        default:
            outRanges.add({currentOffset, currentSize});
            break;
        }
    }
}

List<ByteGranularityParameterUsageInfo>
collectApproximateByteGranularityUsageInformationForParameterGroups(const IRModule* module)
{
    List<ByteGranularityParameterUsageInfo> result;
    // This loop assumes shader parameters are encoded as individual
    // IRGlobalParams. That holds for every production target today.
    // A hypothetical target that aggregates parameters into a single
    // global (or some other representation) would need different
    // discovery logic here.
    for (auto inst : module->getGlobalInsts())
    {
        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;

        auto layoutDeco = param->findDecoration<IRLayoutDecoration>();
        // Not every IRGlobalParam carries a layout decoration. Generic
        // instantiation leftovers and intrinsic resource handles
        // without explicit binding info legitimately do not, so skip.
        if (!layoutDeco)
            continue;

        // Invariant. A layout decoration on an IRGlobalParam is always
        // an IRVarLayout. If this ever fires, the upstream pass that
        // attached the decoration is broken and we want to find out
        // immediately rather than silently dropping the param.
        auto varLayout = as<IRVarLayout>(layoutDeco->getLayout());
        SLANG_RELEASE_ASSERT(varLayout);

        // Only CBs and parameter blocks get tracked here (including the
        // implicit parameter group Slang wraps around loose top level
        // uniforms). Other byte addressable globals like RT payloads or
        // byte address buffer contents are skipped. The Uniform parameter
        // category is just a byte storage label, but per leaf tracking
        // only makes sense for parameter group contents.
        auto groupLayout = as<IRParameterGroupTypeLayout>(varLayout->getTypeLayout());
        if (!groupLayout)
            continue;
        auto elementVar = groupLayout->getElementVarLayout();
        IRTypeLayout* typeLayout = elementVar->getTypeLayout();
        UInt baseOffset = 0;
        if (auto offsetAttr = varLayout->findOffsetAttr(LayoutResourceKind::Uniform))
            baseOffset += offsetAttr->getOffset();
        if (auto offsetAttr = elementVar->findOffsetAttr(LayoutResourceKind::Uniform))
            baseOffset += offsetAttr->getOffset();

        ByteGranularityParameterUsageInfo info;
        info.param = param;
        info.parentSpace = 0;
        if (auto spaceAttr = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
            info.parentSpace = spaceAttr->getOffset();
        // Capture the parent CB or parameter block binding index. For most
        // targets a parameter group has a ConstantBuffer offset (D3D b
        // register) or a DescriptorTableSlot offset (Vulkan/SPIR-V
        // descriptor binding). Some target configurations (cooperative
        // type metadata shaders, certain Metal/WGSL paths) yield a group
        // with neither attr. In that case we have no parent identity to
        // scope per byte ranges against, so emit an untracked entry in
        // the param's space. Queries in that space then return
        // SLANG_E_NOT_AVAILABLE, which is safe over reporting rather
        // than a silent collision with binding zero.
        auto cbAttr = varLayout->findOffsetAttr(LayoutResourceKind::ConstantBuffer);
        auto dtAttr = varLayout->findOffsetAttr(LayoutResourceKind::DescriptorTableSlot);
        if (!cbAttr && !dtAttr)
        {
            info.parentBindingIndex = 0;
            info.isUntracked = true;
            result.add(_Move(info));
            continue;
        }
        info.parentBindingIndex = cbAttr ? cbAttr->getOffset() : dtAttr->getOffset();
        info.isUntracked = false;

        UInt elementByteSize = 0;
        if (!_tryGetUniformByteSize(typeLayout, elementByteSize))
        {
            // Unbounded uniform element type. Emit an untracked marker so
            // queries against this region return SLANG_E_NOT_AVAILABLE
            // rather than a misleading false answer. Engines reading the
            // reflection should treat this as may be used and bind.
            info.isUntracked = true;
            result.add(_Move(info));
            continue;
        }
        // Parameter group whose element type has zero uniform byte size
        // (e.g. an empty cbuffer, or a parameter block whose contents
        // are all resources with no byte addressable storage). Nothing
        // to track, so skip.
        if (elementByteSize == 0)
            continue;

        _collectByteRangesReadFromValue(param, typeLayout, baseOffset, info.ranges);
        if (info.ranges.getCount() > 1)
        {
            info.ranges.sort(
                [](const ByteGranularityUsageRange& a, const ByteGranularityUsageRange& b)
                { return a.offset < b.offset; });
            Index out = 0;
            for (Index i = 1; i < info.ranges.getCount(); ++i)
            {
                auto& last = info.ranges[out];
                auto& cur = info.ranges[i];
                if (cur.offset <= last.offset + last.size)
                {
                    UInt end = Math::Max(last.offset + last.size, cur.offset + cur.size);
                    last.size = end - last.offset;
                }
                else
                {
                    info.ranges[++out] = cur;
                }
            }
            info.ranges.setCount(out + 1);
        }
        result.add(_Move(info));
    }
    return result;
}

} // namespace Slang
