#include "slang-ir-legalize-image-subscript.h"

#include "slang-intrinsic-expand.h"
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
// Return true if `swizzledStore` assigns every component of a texel of type `texelType` exactly
// once, as `t[i].xyzw = v` does, or `t[i].yx = v` on a two-component texel. Such a store replaces
// the whole texel, so it needs no read of the current texel.
static bool doesSwizzleWriteWholeTexel(IRSwizzledStore* swizzledStore, IRType* texelType)
{
    const UInt componentCount = UInt(getIRVectorElementSize(texelType));
    if (swizzledStore->getElementCount() != componentCount)
        return false;
    UInt writtenComponentMask = 0;
    for (UInt i = 0; i < componentCount; i++)
    {
        auto index = cast<IRIntLit>(swizzledStore->getElementIndex(i));
        writtenComponentMask |= UInt(1) << UInt(index->getValue());
    }
    return writtenComponentMask == (UInt(1) << componentCount) - 1;
}

// Return the texel that a store satisfying `doesSwizzleWriteWholeTexel` writes: component `c` of
// the texel is the source component that the swizzle assigns to `c`.
static IRInst* emitWholeTexelFromSwizzle(
    IRBuilder& builder,
    IRSwizzledStore* swizzledStore,
    IRType* texelType)
{
    const UInt componentCount = swizzledStore->getElementCount();
    UInt sourceIndexOfComponent[4];
    bool isIdentity = true;
    for (UInt i = 0; i < componentCount; i++)
    {
        auto component = UInt(cast<IRIntLit>(swizzledStore->getElementIndex(i))->getValue());
        sourceIndexOfComponent[component] = i;
        isIdentity = isIdentity && component == i;
    }
    if (isIdentity)
        return swizzledStore->getSource();
    return builder
        .emitSwizzle(texelType, swizzledStore->getSource(), componentCount, sourceIndexOfComponent);
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
        components.add(
            builder.emitIntrinsicInst(vectorType->getElementType(), kIROp_Select, 3, selectArgs));
    }
    return builder.emitMakeVector(
        texelType,
        components.getCount(),
        components.getArrayView().getBuffer());
}

// Report a texel access that CUDA cannot express. CUDA reaches a surface only through its
// prelude's `surf{1D,2D,3D}[Layered]{read,write}[_convert]` functions, and only some of the
// `_convert` variants exist (see `CUDASurfaceAccessInfo::isConversionAvailable`). We check here,
// where the surface access is introduced, so that the CUDA emitter only receives accesses it can
// spell.
static void diagnoseUnavailableCUDASurfaceAccess(
    IRInst* image,
    IRTextureType* textureType,
    bool readsTexel,
    IRInst* storeInst,
    DiagnosticSink* sink)
{
    switch (textureType->GetBaseShape())
    {
    case SLANG_TEXTURE_1D:
    case SLANG_TEXTURE_2D:
    case SLANG_TEXTURE_3D:
        break;
    default:
        sink->diagnose(Diagnostics::CudaSurfaceShapeUnsupported{.location = storeInst->sourceLoc});
        return;
    }

    if (readsTexel && !getCUDASurfaceAccessInfo(image, false).isConversionAvailable)
    {
        sink->diagnose(Diagnostics::CudaSurfaceFormatConversionUnavailable{
            .access = "read",
            .location = storeInst->sourceLoc,
        });
    }
    else if (!getCUDASurfaceAccessInfo(image, true).isConversionAvailable)
    {
        sink->diagnose(Diagnostics::CudaSurfaceFormatConversionUnavailable{
            .access = "write",
            .location = storeInst->sourceLoc,
        });
    }
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

    if (seperateSampleCoord && (isMetalTarget(target) || isCUDATarget(target)))
    {
        sink->diagnose(Diagnostics::MultiSampledTextureDoesNotAllowWrites{
            .target = target->getTarget(),
            .location = imageSubscript->getImage()->sourceLoc,
        });
    }

    auto swizzledStore = as<IRSwizzledStore>(storeInst);
    const bool readsTexel = swizzledStore
                                ? !doesSwizzleWriteWholeTexel(swizzledStore, imageElementType)
                                : getElementPtr != nullptr;

    if (isCUDATarget(target))
    {
        diagnoseUnavailableCUDASurfaceAccess(
            imageSubscript->getImage(),
            textureType,
            readsTexel,
            storeInst,
            sink);
        // We warn only on CUDA. Elsewhere, losing a concurrent write to another component of the
        // texel is the same data race as two threads writing the texel. On CUDA the read is also
        // undefined after an earlier write to the texel in the same kernel launch (CUDA C++
        // Programming Guide, "Read/Write Coherency"), so even one thread can lose a write.
        if (readsTexel)
        {
            sink->diagnose(Diagnostics::TexturePartialWriteIsReadModifyWrite{
                .location = storeInst->sourceLoc});
        }
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

    IRInst* newTexel = nullptr;
    if (readsTexel)
    {
        auto originalTexel = builder.emitImageLoad(texelType, loadParams);
        if (swizzledStore)
        {
            // Here we assume the imageElementType is already lowered into a vector from any
            // user-defined type.
            SLANG_ASSERT(imageElementType->getOp() == kIROp_VectorType);
            Array<IRInst*, 4> indices;
            for (UInt i = 0; i < swizzledStore->getElementCount(); i++)
            {
                indices.add(swizzledStore->getElementIndex(i));
            }
            newTexel = builder.emitSwizzleSet(
                texelType,
                originalTexel,
                swizzledStore->getSource(),
                swizzledStore->getElementCount(),
                indices.getBuffer());
        }
        else
        {
            newTexel = emitSetTexelComponent(
                builder,
                texelType,
                originalTexel,
                getElementPtr->getIndex(),
                storeInst->getOperand(1));
        }
    }
    else
    {
        newTexel = swizzledStore
                       ? emitWholeTexelFromSwizzle(builder, swizzledStore, imageElementType)
                       : storeInst->getOperand(1);
        if (getIRVectorElementSize(imageElementType) != getIRVectorElementSize(texelType))
        {
            newTexel = builder.emitVectorReshape(texelType, newTexel);
        }
    }

    storeParams[kValueParamIndex] = newTexel;
    auto imageStore = builder.emitImageStore(builder.getVoidType(), storeParams);
    storeInst->replaceUsesWith(imageStore);
    storeInst->removeAndDeallocate();
    if (!imageSubscript->hasUses())
    {
        imageSubscript->removeAndDeallocate();
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
