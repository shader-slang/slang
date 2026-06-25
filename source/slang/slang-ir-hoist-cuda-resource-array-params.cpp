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
static bool _typeContainsFixedSizeResourceArray(IRType* type)
{
    if (!type)
        return false;

    // `IRArrayType` is the fixed-size array (as opposed to `IRUnsizedArrayType`).
    if (auto arrayType = as<IRArrayType>(type))
    {
        if (as<IRIntLit>(arrayType->getElementCount()) &&
            _typeIsOrContainsResource(arrayType->getElementType()))
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

struct HoistCUDAResourceArrayParams : PerEntryPointPass
{
    // CUDA emits a single hardcoded `SLANG_globalParams` symbol, so a module can hold only one
    // hoisted global parameter group; this tracks whether we have already created it.
    bool m_moduleAlreadyHoisted = false;

    // Returns true if the module already contains a module-scope uniform parameter group global
    // (e.g. one created by `collectGlobalUniformParameters`, or our own from a prior entry point).
    // Emitting a second would collide on the hardcoded `SLANG_globalParams` symbol in NVRTC.
    static bool _moduleHasUniformParameterGroupGlobal(IRModule* module)
    {
        for (auto inst : module->getGlobalInsts())
        {
            if (auto globalParam = as<IRGlobalParam>(inst))
            {
                if (as<IRUniformParameterGroupType>(globalParam->getDataType()))
                    return true;
            }
        }
        return false;
    }

    void processEntryPointImpl(EntryPointInfo const& info) SLANG_OVERRIDE
    {
        auto entryPointFunc = info.func;
        auto entryPointDecoration = info.decoration;

        // CUDA launch parameters only matter for ordinary compute kernels; ray-tracing entry
        // points route their uniforms through the SBT and are handled elsewhere.
        if (entryPointDecoration->getProfile().getStage() != Stage::Compute)
            return;

        // We need explicit layout to know the field keys and to attach a matching layout to the
        // synthesized global parameter. Be defensive in release builds.
        auto funcLayoutDecoration = entryPointFunc->findDecoration<IRLayoutDecoration>();
        SLANG_ASSERT(funcLayoutDecoration);
        if (!funcLayoutDecoration)
            return;
        auto entryPointLayout = as<IREntryPointLayout>(funcLayoutDecoration->getLayout());
        SLANG_ASSERT(entryPointLayout);
        if (!entryPointLayout)
            return;
        auto entryPointParamsLayout = entryPointLayout->getParamsLayout();
        auto entryPointParamsStructLayout = getScopeStructLayout(entryPointLayout);

        // Decide whether this entry point is one we care about: it must have at least one uniform
        // parameter that transitively contains a fixed-size resource array. If not, leave it
        // completely untouched so we never perturb the common fast path.
        bool shouldHoist = false;
        for (IRParam* param = entryPointFunc->getFirstParam(); param; param = param->getNextParam())
        {
            auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
            if (!layoutDecoration)
                continue;
            auto paramLayout = as<IRVarLayout>(layoutDecoration->getLayout());
            if (!paramLayout)
                continue;
            if (isVaryingParameter(paramLayout))
                continue;
            if (_typeContainsFixedSizeResourceArray(param->getFullType()))
            {
                shouldHoist = true;
                break;
            }
        }
        if (!shouldHoist)
            return;

        // CUDA emits a single hardcoded `SLANG_globalParams` symbol per module
        // (`CUDASourceEmitter::emitParameterGroupImpl`), so a module can have only one global
        // parameter group. Hoist at most one qualifying compute entry point per module, and never
        // when a module-scope uniform parameter group already exists (e.g. one that
        // `collectGlobalUniformParameters` created): a second `__constant__ SLANG_globalParams`
        // would be a duplicate-symbol error in NVRTC. Any other qualifying kernels keep the default
        // (correct, if slower) entry-point `.param` path. A full multi-kernel merge is future work.
        if (m_moduleAlreadyHoisted || _moduleHasUniformParameterGroupGlobal(m_module))
            return;

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

            // Once hoisting, every uniform parameter must carry layout information; a missing
            // layout would leave a half-hoisted ABI, so fail loudly rather than silently continue.
            auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
            SLANG_RELEASE_ASSERT(layoutDecoration);
            auto paramLayout = as<IRVarLayout>(layoutDecoration->getLayout());
            SLANG_RELEASE_ASSERT(paramLayout);

            // Leave varying parameters (system values, stage I/O) on the entry point.
            if (isVaryingParameter(paramLayout))
                continue;

            for (auto offsetAttr : paramLayout->getOffsetAttrs())
                resourceKinds.add(offsetAttr->getResourceKind());

            auto paramType = param->getFullType();

            builder->setInsertBefore(paramStructType);
            auto paramFieldKey = cast<IRStructKey>(
                entryPointParamsStructLayout->getFieldLayoutAttrs()[paramIndex]->getFieldKey());
            structLayoutBuilder.addField(
                paramFieldKey,
                entryPointParamsStructLayout->getFieldLayout(paramIndex));
            builder->createStructField(paramStructType, paramFieldKey, paramType);

            // Move decorations (name hint, etc.) onto the field key for downstream emit.
            param->transferDecorationsTo(paramFieldKey);

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

        // Construct and attach the global parameter's layout. When the entry point's params layout
        // is itself a parameter group (the common case), mirror its container/element split with
        // unrelated (e.g. varying) offsets filtered out so the result is a parameter-group layout
        // whose type matches the `ConstantBuffer`-typed global; otherwise fall back to the bare
        // struct layout. Modeled on `moveEntryPointUniformParamsToGlobalScope`.
        IRTypeLayout* collectedTypeLayout = nullptr;
        if (auto originalParamGroupLayout =
                as<IRParameterGroupTypeLayout>(entryPointParamsLayout->getTypeLayout()))
        {
            for (auto offsetAttr :
                 originalParamGroupLayout->getContainerVarLayout()->getOffsetAttrs())
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
            collectedTypeLayout = paramGroupTypeLayoutBuilder.build();
        }
        else
        {
            collectedTypeLayout = structLayoutBuilder.build();
        }

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
        m_moduleAlreadyHoisted = true;
    }
};

void hoistCUDAResourceArrayParamsToParameterGroup(IRModule* module)
{
    HoistCUDAResourceArrayParams context;
    context.processModule(module);
}

} // namespace Slang
