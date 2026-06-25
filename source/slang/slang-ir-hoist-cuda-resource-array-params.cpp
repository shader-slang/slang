// slang-ir-hoist-cuda-resource-array-params.cpp
//
// Note: the per-entry-point iteration and use-replacement logic here is closely modeled on
// slang-ir-entry-point-uniforms.cpp / slang-ir-optix-entry-point-uniforms.cpp; the global-param
// creation mirrors slang-ir-collect-global-uniforms.cpp.

#include "slang-ir-hoist-cuda-resource-array-params.h"

#include "slang-ir-entry-point-pass.h"
#include "slang-ir-insts.h"
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

        IRBuilder builderStorage(m_module);
        auto builder = &builderStorage;

        // Build a `GlobalParams` struct and a module-scope `ConstantBuffer<GlobalParams>` global
        // parameter. Attaching the entry point's params layout makes the reflected/bound shape
        // match what an explicit parameter-group wrapper produces.
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
        builder->addLayoutDecoration(collectedParam, entryPointParamsLayout);
        builder->addNameHintDecoration(
            collectedParam,
            UnownedTerminatedStringSlice("globalParams"));

        // Move every uniform parameter into the struct and rematerialize its value at each use site
        // as a load from the constant buffer.
        IRParam* nextParam = nullptr;
        UInt paramCounter = 0;
        for (IRParam* param = entryPointFunc->getFirstParam(); param; param = nextParam)
        {
            nextParam = param->getNextParam();
            UInt paramIndex = paramCounter++;

            auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
            SLANG_ASSERT(layoutDecoration);
            if (!layoutDecoration)
                continue;
            auto paramLayout = as<IRVarLayout>(layoutDecoration->getLayout());
            SLANG_ASSERT(paramLayout);
            if (!paramLayout)
                continue;

            // Leave varying parameters (system values, stage I/O) on the entry point.
            if (isVaryingParameter(paramLayout))
                continue;

            auto paramType = param->getFullType();

            builder->setInsertBefore(paramStructType);
            auto paramFieldKey = cast<IRStructKey>(
                entryPointParamsStructLayout->getFieldLayoutAttrs()[paramIndex]->getFieldKey());
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

        fixUpFuncType(entryPointFunc);
    }
};

void hoistCUDAResourceArrayParamsToParameterGroup(IRModule* module)
{
    HoistCUDAResourceArrayParams context;
    context.processModule(module);
}

} // namespace Slang
