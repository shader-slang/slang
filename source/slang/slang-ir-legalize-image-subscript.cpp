#include "slang-ir-legalize-image-subscript.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-parameter-binding.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
// A write to some of the components of a texel becomes a read-modify-write of the whole texel,
// which is not atomic: a concurrent write by another thread to other components of the same
// texel can be lost. On CUDA the read is also undefined if the texel was already written in the
// same kernel launch (CUDA C++ Programming Guide, "Read/Write Coherency"). We warn about this on
// CUDA.
static void diagnoseTexelReadModifyWrite(
    TargetRequest* target,
    IRInst* storeInst,
    DiagnosticSink* sink)
{
    if (!isCUDATarget(target))
        return;
    sink->diagnose(Diagnostics::TexturePartialWriteIsReadModifyWrite{
        .target = target->getTarget(),
        .location = storeInst->sourceLoc,
    });
}

// Return `texel` with the component at `index` replaced by `value`. A constant index is a
// one-element swizzle. A dynamic index, as in `tex[i][k] = v`, cannot be a swizzle, so we pick each
// component `c` of the new texel as `k == c ? v : texel[c]`.
static IRInst* emitSetTexelComponent(
    IRBuilder& builder,
    IRType* texelType,
    IRInst* texel,
    IRInst* index,
    IRInst* value)
{
    if (as<IRIntLit>(index))
        return builder.emitSwizzleSet(texelType, texel, value, 1, &index);

    auto vectorType = as<IRVectorType>(texelType);
    SLANG_RELEASE_ASSERT(vectorType);
    IRIntegerValue componentCount = getIntVal(vectorType->getElementCount());
    ShortList<IRInst*> components;
    for (IRIntegerValue c = 0; c < componentCount; c++)
    {
        IRInst* selectArgs[] = {
            builder.emitEql(index, builder.getIntValue(index->getDataType(), c)),
            value,
            builder.emitElementExtract(texel, c),
        };
        components.add(builder.emitIntrinsicInst(
            vectorType->getElementType(),
            kIROp_Select,
            3,
            selectArgs));
    }
    return builder.emitMakeVector(
        texelType,
        components.getCount(),
        components.getArrayView().getBuffer());
}

void legalizeStore(
    TargetRequest* target,
    IRBuilder& builder,
    IRInst* storeInst,
    DiagnosticSink* sink)
{
    SLANG_ASSERT(storeInst);

    builder.setInsertBefore(storeInst);
    IRBuilderSourceLocRAII sourceLocationScope(&builder, storeInst->sourceLoc);
    auto getElementPtr = as<IRGetElementPtr>(storeInst->getOperand(0));
    IRImageSubscript* imageSubscript = as<IRImageSubscript>(getRootAddr(storeInst->getOperand(0)));
    SLANG_ASSERT(imageSubscript);
    SLANG_ASSERT(imageSubscript->getImage());
    IRTextureType* textureType = as<IRTextureType>(imageSubscript->getImage()->getFullType());
    SLANG_ASSERT(textureType);
    auto imageElementType = cast<IRPtrTypeBase>(imageSubscript->getDataType())->getValueType();
    // Metal, GLSL and SPIR-V image loads and stores always operate on 4-component texels. A CUDA
    // `surf*read<T>`/`surf*write<T>` accesses exactly `sizeof(T)` bytes, so on CUDA the texel keeps
    // the texture's own element type.
    IRType* texelType = isCUDATarget(target)
                            ? imageElementType
                            : builder.getVectorType(getIRVectorBaseType(imageElementType), 4);
    IRType* coordType = imageSubscript->getCoord()->getDataType();
    int coordVectorSize = getIRVectorElementSize(coordType);

    bool seperateArrayCoord =
        (isMetalTarget(target) && textureType->isArray());     // seperate array param
    bool seperateSampleCoord = (textureType->isMultisample()); // seperate sample param

    if (seperateSampleCoord && isMetalTarget(target))
    {
        sink->diagnose(Diagnostics::MultiSampledTextureDoesNotAllowWrites{
            .target = target->getTarget(),
            .location = imageSubscript->getImage()->sourceLoc,
        });
    }

    IRType* indexingType = builder.getIntType();
    if (isMetalTarget(target))
        indexingType = builder.getUIntType();

    if (coordVectorSize != 1)
    {
        coordType = builder.getVectorType(
            indexingType,
            builder.getIntValue(builder.getIntType(), coordVectorSize));
    }
    else
    {
        coordType = indexingType;
    }

    auto legalizedCoord = imageSubscript->getCoord();
    if (coordType != imageSubscript->getCoord()->getDataType())
    {
        legalizedCoord = builder.emitCast(coordType, legalizedCoord);
    }

    const Index kCoordParamIndex = 1;
    const Index kValueParamIndex = 2;

    ShortList<IRInst*> loadParams;
    loadParams.reserveOverflowBuffer(4);
    loadParams.add(imageSubscript->getImage()); // image
    loadParams.add(legalizedCoord);             // coord

    ShortList<IRInst*> storeParams;
    storeParams.reserveOverflowBuffer(5);
    storeParams.add(imageSubscript->getImage()); // image
    storeParams.add(legalizedCoord);             // coord
    storeParams.add(nullptr);                    // value

    if (seperateArrayCoord)
    {

        UInt paramIndexToFetch = coordVectorSize - 1;

        auto seperatedParam =
            builder.emitSwizzle(indexingType, legalizedCoord, 1, &paramIndexToFetch);
        loadParams.add(seperatedParam);
        storeParams.add(seperatedParam);

        coordVectorSize -= 1;
        ShortList<UInt> paramToFetch;
        paramToFetch.reserveOverflowBuffer(coordVectorSize);
        for (int i = 0; i < coordVectorSize; i++)
        {
            paramToFetch.add(i);
        }
        auto newCoord = builder.emitSwizzle(
            builder.getVectorType(
                indexingType,
                builder.getIntValue(builder.getIntType(), coordVectorSize)),
            legalizedCoord,
            coordVectorSize,
            paramToFetch.getArrayView().getBuffer());
        storeParams[kCoordParamIndex] = newCoord;
        loadParams[kCoordParamIndex] = newCoord;
    }
    if (seperateSampleCoord)
    {
        loadParams.add(imageSubscript->getSampleCoord());
        storeParams.add(imageSubscript->getSampleCoord());
    }

    IRInst* legalizedStore = storeInst->getOperand(1);
    switch (storeInst->getOp())
    {
    case kIROp_Store:
        {
            IRInst* newValue = nullptr;
            if (getElementPtr)
            {
                diagnoseTexelReadModifyWrite(target, storeInst, sink);
                auto originalValue = builder.emitImageLoad(texelType, loadParams);
                newValue = emitSetTexelComponent(
                    builder,
                    texelType,
                    originalValue,
                    getElementPtr->getIndex(),
                    legalizedStore);
            }
            else
            {
                newValue = legalizedStore;
                if (getIRVectorElementSize(imageElementType) != getIRVectorElementSize(texelType))
                {
                    newValue = builder.emitVectorReshape(texelType, newValue);
                }
            }

            storeParams[kValueParamIndex] = newValue;
            auto imageStore = builder.emitImageStore(builder.getVoidType(), storeParams);
            storeInst->replaceUsesWith(imageStore);
            storeInst->removeAndDeallocate();
            if (!imageSubscript->hasUses())
            {
                imageSubscript->removeAndDeallocate();
            }
        }
        break;
    case kIROp_SwizzledStore:
        {
            auto swizzledStore = cast<IRSwizzledStore>(storeInst);
            // Here we assume the imageElementType is already lowered into float4/uint4 types from
            // any user-defined type.
            SLANG_ASSERT(imageElementType->getOp() == kIROp_VectorType);
            diagnoseTexelReadModifyWrite(target, storeInst, sink);
            auto originalValue = builder.emitImageLoad(texelType, loadParams);
            Array<IRInst*, 4> indices;
            for (UInt i = 0; i < swizzledStore->getElementCount(); i++)
            {
                indices.add(swizzledStore->getElementIndex(i));
            }
            auto newValue = builder.emitSwizzleSet(
                texelType,
                originalValue,
                swizzledStore->getSource(),
                swizzledStore->getElementCount(),
                indices.getBuffer());
            storeParams[kValueParamIndex] = newValue;
            auto imageStore = builder.emitImageStore(builder.getVoidType(), storeParams);
            storeInst->replaceUsesWith(imageStore);
            storeInst->removeAndDeallocate();
            if (!imageSubscript->hasUses())
            {
                imageSubscript->removeAndDeallocate();
            }
        }
        break;
    default:
        break;
    }
}
void legalizeImageSubscript(IRModule* module, TargetRequest* target, DiagnosticSink* sink)
{
    IRBuilder builder(module);
    for (auto globalInst : module->getModuleInst()->getChildren())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;
        for (auto block : func->getBlocks())
        {
            auto inst = block->getFirstInst();
            IRInst* next;
            for (; inst; inst = next)
            {
                next = inst->getNextInst();
                switch (inst->getOp())
                {
                case kIROp_Store:
                case kIROp_SwizzledStore:
                    if (as<IRImageSubscript>(getRootAddr(inst->getOperand(0))))
                        legalizeStore(target, builder, inst, sink);
                    continue;
                }
            }
        }
    }
}
} // namespace Slang
