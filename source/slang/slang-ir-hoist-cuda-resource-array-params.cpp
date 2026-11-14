// slang-ir-hoist-cuda-resource-array-params.cpp
//
// Note: the per-entry-point iteration and use-replacement logic here is closely modeled on
// slang-ir-entry-point-uniforms.cpp / slang-ir-optix-entry-point-uniforms.cpp; the global-param
// creation mirrors slang-ir-collect-global-uniforms.cpp.

#include "slang-ir-hoist-cuda-resource-array-params.h"

#include "slang-ir-entry-point-pass.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Returns true if `type` is a resource, or transitively contains one through struct fields or
// array elements. Used to decide whether a fixed-size array's element carries a resource (the
// element may be a struct wrapping a resource, e.g. a tensor type, not a bare resource).
//
// `isResourceType` (slang-legalize-types.cpp) is the single source of truth for the resource-leaf
// test: it already unwraps `IRArrayTypeBase` and recognizes the whole resource family (textures,
// samplers, structured/byte-address buffers, pointer-like types). The extra array/struct recursion
// below exists only to thread the `IRPtrType` carve-out and the struct-wrapping case through
// nesting that `isResourceType`'s leaf check does not itself reach (e.g. a `Ptr<float>[N]` field).
static bool _typeIsOrContainsResource(IRType* type)
{
    if (!type)
        return false;
    if (isResourceType(type))
        return true;
    // Some hosts (e.g. SlangPy) specialize tensor/buffer storage to a plain data pointer on CUDA
    // rather than a structured-buffer resource; a fixed-size array of such pointer-backed elements
    // packed into the `.param` bank exhibits the same dynamic-addressing slowdown as a resource
    // array. Match only the first-class data pointer `IRPtrType` here, not the `IRPtrTypeBase`
    // umbrella, which also covers the out/inout/ref parameter-direction wrappers (those are not
    // resource-backing storage and must not divert ordinary kernels onto this path).
    if (as<IRPtrType>(type))
        return true;
    if (auto arrayType = as<IRArrayTypeBase>(type))
        return _typeIsOrContainsResource(arrayType->getElementType());
    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (_typeIsOrContainsResource(field->getFieldType()))
                return true;
        }
    }
    return false;
}

// Returns true if `type` is, or transitively contains, a fixed-size (constant-length) array whose
// element type is or contains a resource. Unsized arrays (`IRUnsizedArrayType`) are intentionally
// ignored: they are not packed into the `.param` bank and are out of scope here.
//
// This is purely a type check and does not inspect access patterns. A uniform whose resource array
// is only statically indexed (e.g. `tensors[0][tid]`) therefore also qualifies — the hoist is still
// correct in that case, just not a perf win (the `.param` slowdown the pass targets only arises
// with a runtime index). Narrowing to genuinely-runtime-indexed accesses would require walking the
// parameter's uses for a non-literal `IRGetElement`; the conservative type-based proxy is used for
// now.
static bool _typeContainsFixedSizeResourceArray(IRType* type)
{
    if (!type)
        return false;

    // `IRArrayType` is the fixed-size array (as opposed to `IRUnsizedArrayType`); its extent need
    // not be a literal `IRIntLit` after specialization, so detect on the array node itself.
    if (auto arrayType = as<IRArrayType>(type))
    {
        if (_typeIsOrContainsResource(arrayType->getElementType()))
            return true;
        // An array of structs/arrays may itself hold a qualifying fixed-size resource array.
        return _typeContainsFixedSizeResourceArray(arrayType->getElementType());
    }

    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (_typeContainsFixedSizeResourceArray(field->getFieldType()))
                return true;
        }
        return false;
    }

    return false;
}

// Returns `param`'s variable layout if it carries an `IRLayoutDecoration` whose payload is an
// `IRVarLayout`, or null otherwise. A parameter can legitimately lack such a layout (e.g. one
// synthesized by an earlier pass), so the qualifier and the rewrite both consult this single
// predicate and skip a null result identically — without it the two loops diverged on which
// parameters they considered hoistable (the qualifier `continue`d, the rewrite release-asserted),
// which could abort the compiler on a qualifying kernel that happened to carry a layout-less
// sibling parameter.
static IRVarLayout* _getParamVarLayout(IRParam* param)
{
    auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
    if (!layoutDecoration)
        return nullptr;
    return as<IRVarLayout>(layoutDecoration->getLayout());
}

struct HoistCUDAResourceArrayParams : PerEntryPointPass
{
    // Returns true if `func` is a compute entry point whose uniform parameters transitively contain
    // a fixed-size resource array — i.e. one this pass would hoist. Used both to gate the per-entry
    // point transform and to count qualifying entry points up front.
    static bool _funcQualifiesForHoist(IRFunc* func)
    {
        auto entryPointDecoration = func->findDecoration<IREntryPointDecoration>();
        if (!entryPointDecoration)
            return false;
        // CUDA launch parameters only matter for ordinary compute kernels; ray-tracing entry points
        // route their uniforms through the SBT and are handled elsewhere.
        if (entryPointDecoration->getProfile().getStage() != Stage::Compute)
            return false;
        // We synthesize a `ConstantBuffer<GlobalParams>` global, so the entry point's params layout
        // must itself be a parameter group for us to build a matching parameter-group layout. The
        // layout pipeline may instead produce a bare `IRStructTypeLayout` (no constant-buffer
        // wrapper) — see the `needConstantBuffer` discrimination in
        // slang-ir-entry-point-uniforms.cpp — and such an entry point is left on the default path
        // rather than hoisted.
        auto funcLayoutDecoration = func->findDecoration<IRLayoutDecoration>();
        if (!funcLayoutDecoration)
            return false;
        auto entryPointLayout = as<IREntryPointLayout>(funcLayoutDecoration->getLayout());
        if (!entryPointLayout)
            return false;
        if (!as<IRParameterGroupTypeLayout>(entryPointLayout->getParamsLayout()->getTypeLayout()))
            return false;
        for (IRParam* param = func->getFirstParam(); param; param = param->getNextParam())
        {
            auto paramLayout = _getParamVarLayout(param);
            if (!paramLayout)
                continue;
            if (isVaryingParameter(paramLayout))
                continue;
            if (_typeContainsFixedSizeResourceArray(param->getFullType()))
                return true;
        }
        return false;
    }

    // Returns true if the module already contains any module-scope uniform parameter group global.
    // `CUDASourceEmitter::emitParameterGroupImpl` emits *every* such global as the hardcoded
    // `extern "C" __constant__ ... SLANG_globalParams` symbol (the variable's name only drives a
    // `#define` alias), so a second one — whatever its name — would be a duplicate-symbol error in
    // NVRTC. We therefore refuse to hoist when any parameter group global already exists (e.g. one
    // `collectGlobalUniformParameters` created); merging into it is future work.
    static bool _moduleHasUniformParameterGroupGlobal(IRModule* module)
    {
        for (auto inst : module->getGlobalInsts())
        {
            auto globalParam = as<IRGlobalParam>(inst);
            if (!globalParam)
                continue;
            if (as<IRUniformParameterGroupType>(globalParam->getDataType()))
                return true;
        }
        return false;
    }

    void processEntryPointImpl(EntryPointInfo const& info) SLANG_OVERRIDE
    {
        auto entryPointFunc = info.func;

        // The module-level driver guarantees exactly one entry point qualifies and that hoisting
        // will not collide with an existing `globalParams`, so any qualifying entry point we see
        // here is the one to hoist.
        if (!_funcQualifiesForHoist(entryPointFunc))
            return;

        // We need explicit layout to know the field keys and to attach a matching layout to the
        // synthesized global parameter; these hold for a qualifying compute entry point.
        auto funcLayoutDecoration = entryPointFunc->findDecoration<IRLayoutDecoration>();
        SLANG_RELEASE_ASSERT(funcLayoutDecoration);
        auto entryPointLayout = as<IREntryPointLayout>(funcLayoutDecoration->getLayout());
        SLANG_RELEASE_ASSERT(entryPointLayout);
        auto entryPointParamsLayout = entryPointLayout->getParamsLayout();
        auto entryPointParamsStructLayout = getScopeStructLayout(entryPointLayout);

        IRBuilder builderStorage(m_module);
        auto builder = &builderStorage;

        // Build a `GlobalParams` struct and a module-scope `ConstantBuffer<GlobalParams>` global
        // parameter to hold the hoisted uniforms. Its layout is attached after the field loop.
        builder->setInsertBefore(entryPointFunc);
        auto paramStructType = builder->createStructType();
        builder->addNameHintDecoration(
            paramStructType,
            UnownedTerminatedStringSlice("GlobalParams"));
        builder->addBinaryInterfaceTypeDecoration(paramStructType);

        auto constantBufferType = builder->getConstantBufferType(
            paramStructType,
            builder->getType(kIROp_DefaultBufferLayoutType));
        auto collectedParam = builder->createGlobalParam(constantBufferType);
        builder->addNameHintDecoration(
            collectedParam,
            UnownedTerminatedStringSlice("globalParams"));

        // Unlike the reference pass `moveEntryPointUniformParamsToGlobalScope`
        // (slang-ir-entry-point-uniforms.cpp:652), we intentionally do NOT add an
        // `IREntryPointParamDecoration` back-link to `entryPointFunc`. CUDA source emit consumes
        // this global directly via `CUDASourceEmitter::emitParameterGroupImpl` and never reads that
        // decoration; its only consumers do not act on it for CUDA —
        // `introduceExplicitGlobalContext` runs but skips over `IRGlobalParam`s on CUDA, leaving
        // them as `__constant__` (slang-ir-explicit-global-context.cpp:245-247), and the Metal/CPU
        // layout-legalization paths do not run on the CUDA source path. Add it if this pass is ever
        // reused for a target that re-associates globals with their entry point.

        // Move every uniform parameter into the struct and rematerialize its value at each use site
        // as a load from the constant buffer. A fresh `IRStructTypeLayout` is built in lock-step
        // with the field insertion so the synthesized global carries a layout shape that matches
        // its `ConstantBuffer` type (see the parameter-group layout construction below).
        IRStructTypeLayout::Builder structLayoutBuilder(builder);
        HashSet<LayoutResourceKind> resourceKinds;
        IRParam* nextParam = nullptr;
        UInt paramCounter = 0;
        for (IRParam* param = entryPointFunc->getFirstParam(); param; param = nextParam)
        {
            nextParam = param->getNextParam();
            UInt paramIndex = paramCounter++;

            // Skip exactly the parameters the qualifier skips. A parameter without an `IRVarLayout`
            // is not a uniform we can place into the layout-bearing `GlobalParams` struct, so leave
            // it on the entry point (matching the reference pass
            // `moveEntryPointUniformParamsToGlobalScope`, which also `continue`s) rather than
            // aborting. `paramCounter` is incremented above for every parameter, so skipping here
            // does not disturb the positional field-key indexing used below.
            auto paramLayout = _getParamVarLayout(param);
            if (!paramLayout)
                continue;

            // Leave varying parameters (system values, stage I/O) on the entry point.
            if (isVaryingParameter(paramLayout))
                continue;

            for (auto offsetAttr : paramLayout->getOffsetAttrs())
                resourceKinds.add(offsetAttr->getResourceKind());

            auto paramType = param->getFullType();

            builder->setInsertBefore(paramStructType);
            SLANG_RELEASE_ASSERT(entryPointParamsStructLayout);
            auto fieldLayoutAttrs = entryPointParamsStructLayout->getFieldLayoutAttrs();
            SLANG_RELEASE_ASSERT(paramIndex < (UInt)fieldLayoutAttrs.getCount());
            auto paramFieldKey = cast<IRStructKey>(fieldLayoutAttrs[paramIndex]->getFieldKey());
            structLayoutBuilder.addField(
                paramFieldKey,
                entryPointParamsStructLayout->getFieldLayout(paramIndex));
            builder->createStructField(paramStructType, paramFieldKey, paramType);

            // Move decorations (name hint, etc.) onto the field key for downstream emit.
            param->transferDecorationsTo(paramFieldKey);

            // `paramFieldKey` is the *same* `IRStructKey` the original entry-point params struct
            // layout keys on, and we deliberately retain that layout (it preserves CUDA reflection,
            // see the header). `transferDecorationsTo` also moved the parameter's
            // `IRLayoutDecoration` — whose offsets are relative to the *entry-point* layout — onto
            // that shared key, so a consumer reading field-level layout off the key would see
            // entry-point-relative offsets rather than the constant-buffer-relative ones the
            // synthesized `structTypeLayout` represents. The `GlobalParams` field's layout already
            // lives in `structLayoutBuilder`, so strip the stray decoration off the key to keep the
            // two layout pictures from aliasing.
            if (auto staleParamLayout = paramFieldKey->findDecoration<IRLayoutDecoration>())
                staleParamLayout->removeAndDeallocate();

            while (auto use = param->firstUse)
            {
                builder->setInsertBefore(use->getUser());
                auto fieldAddress = builder->emitFieldAddress(
                    builder->getPtrType(paramType),
                    collectedParam,
                    paramFieldKey);
                auto fieldVal = builder->emitLoad(fieldAddress);
                builder->replaceOperand(use, fieldVal);
            }

            param->removeAndDeallocate();
        }

        // Construct and attach the global parameter's layout. The synthesized global is always a
        // `ConstantBuffer<GlobalParams>`, so its layout must be a parameter-group layout. This is
        // guaranteed for any entry point reaching here: `_funcQualifiesForHoist` requires the
        // params layout to be an `IRParameterGroupTypeLayout` (entry points with a bare struct
        // layout are not hoisted). We mirror the original group's container/element split with
        // unrelated (e.g. varying) offsets filtered out, modeled on
        // `moveEntryPointUniformParamsToGlobalScope`.
        auto originalParamGroupLayout =
            as<IRParameterGroupTypeLayout>(entryPointParamsLayout->getTypeLayout());
        SLANG_RELEASE_ASSERT(originalParamGroupLayout);

        for (auto offsetAttr : originalParamGroupLayout->getContainerVarLayout()->getOffsetAttrs())
            resourceKinds.add(offsetAttr->getResourceKind());

        auto structTypeLayout = structLayoutBuilder.build();
        auto originalElementVarLayout = originalParamGroupLayout->getElementVarLayout();
        IRVarLayout::Builder elementVarLayoutBuilder(builder, structTypeLayout);
        elementVarLayoutBuilder.cloneEverythingButOffsetsFrom(originalElementVarLayout);
        for (auto resKind : resourceKinds)
        {
            auto originalOffset = originalElementVarLayout->findOffsetAttr(resKind);
            if (!originalOffset)
                continue;
            auto resInfo = elementVarLayoutBuilder.findOrAddResourceInfo(resKind);
            resInfo->offset = originalOffset->getOffset();
            resInfo->space = originalOffset->getSpace();
        }
        auto newElementVarLayout = elementVarLayoutBuilder.build();

        IRParameterGroupTypeLayout::Builder paramGroupTypeLayoutBuilder(builder);
        for (auto resKind : resourceKinds)
        {
            if (auto sizeAttr = originalParamGroupLayout->findSizeAttr(resKind))
                paramGroupTypeLayoutBuilder.addResourceUsage(sizeAttr);
        }
        paramGroupTypeLayoutBuilder.setContainerVarLayout(
            originalParamGroupLayout->getContainerVarLayout());
        paramGroupTypeLayoutBuilder.setElementVarLayout(newElementVarLayout);
        paramGroupTypeLayoutBuilder.setOffsetElementTypeLayout(
            applyOffsetToTypeLayout(builder, structTypeLayout, newElementVarLayout));
        IRTypeLayout* collectedTypeLayout = paramGroupTypeLayoutBuilder.build();

        IRVarLayout::Builder varLayoutBuilder(builder, collectedTypeLayout);
        varLayoutBuilder.cloneEverythingButOffsetsFrom(entryPointParamsLayout);
        for (auto offsetAttr : entryPointParamsLayout->getOffsetAttrs())
        {
            if (!resourceKinds.contains(offsetAttr->getResourceKind()))
                continue;
            auto resInfo = varLayoutBuilder.findOrAddResourceInfo(offsetAttr->getResourceKind());
            resInfo->offset = offsetAttr->getOffset();
            resInfo->space = offsetAttr->getSpace();
        }
        builder->addLayoutDecoration(collectedParam, varLayoutBuilder.build());

        fixUpFuncType(entryPointFunc);
    }
};

void hoistCUDAResourceArrayParamsToParameterGroup(IRModule* module)
{
    // CUDA emits a single hardcoded `SLANG_globalParams` symbol per module
    // (`CUDASourceEmitter::emitParameterGroupImpl`), so at most one parameter-group global can
    // exist. If more than one compute entry point would qualify, hoisting just one and leaving the
    // rest on the `.param` path would be non-uniform and risks mutating layout/field-key state
    // shared with the un-hoisted kernels, so we hoist none and leave every kernel on the default
    // path. Merging multiple kernels into one shared `GlobalParams` is future work.
    int qualifyingCount = 0;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            if (HoistCUDAResourceArrayParams::_funcQualifiesForHoist(func))
            {
                if (++qualifyingCount > 1)
                    return;
            }
        }
    }
    if (qualifyingCount != 1)
        return;

    // Never hoist when a module-scope uniform parameter group global already exists (e.g. one
    // `collectGlobalUniformParameters` synthesizes for any global-scope uniform or user `cbuffer`):
    // every such global emits as the same hardcoded `__constant__ SLANG_globalParams`, so adding a
    // second would collide in NVRTC. A consequence is that the optimization does not fire for a
    // kernel that also has global-scope uniforms; the runtime-indexed resource array stays on the
    // `.param` path in that case.
    // TODO: merge the hoisted fields into the existing GlobalParams instead of bailing, so the
    // optimization also applies when a global parameter group is already present.
    if (HoistCUDAResourceArrayParams::_moduleHasUniformParameterGroupGlobal(module))
        return;

    HoistCUDAResourceArrayParams context;
    context.processModule(module);
}

} // namespace Slang
