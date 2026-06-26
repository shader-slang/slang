#include "slang-ir-vk-invert-y.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

static IRInst* _invertYOfVector(IRBuilder& builder, IRInst* originalVector)
{
    auto vectorType = as<IRVectorType>(originalVector->getDataType());
    SLANG_ASSERT(vectorType);
    UInt elementIndexY = 1;
    auto originalY =
        builder.emitSwizzle(vectorType->getElementType(), originalVector, 1, &elementIndexY);
    auto negY = builder.emitNeg(originalY->getDataType(), originalY);
    auto newVal =
        builder
            .emitSwizzleSet(originalVector->getDataType(), originalVector, negY, 1, &elementIndexY);
    return newVal;
}
// Find outputs to SV_Position and invert the y coordinates of it right before the write.
void invertYOfPositionOutput(IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->findDecoration<IRGLPositionOutputDecoration>())
        {
            // Find all loads and stores to it.
            IRBuilder builder(module);
            List<IRUse*> useWorkList;
            auto processUse = [&](IRUse* use)
            {
                if (auto store = as<IRStore>(use->getUser()))
                {
                    if (getRootAddr(store->getPtr()) != globalInst)
                        return;

                    builder.setInsertBefore(store);
                    auto originalVal = store->getVal();
                    auto invertedVal = _invertYOfVector(builder, originalVal);
                    builder.replaceOperand(&store->val, invertedVal);
                }
                else if (auto load = as<IRLoad>(use->getUser()))
                {
                    // Since we negate the y coordinate before writing
                    // to gl_Position, we also need to negate the value after reading from it.
                    builder.setInsertAfter(load);
                    // Store existing uses of the load that we are going to replace with
                    // inverted val later.
                    List<IRUse*> oldUses;
                    for (auto loadUse = load->firstUse; loadUse; loadUse = loadUse->nextUse)
                        oldUses.add(loadUse);
                    // Get the inverted vector.
                    auto invertedVal = _invertYOfVector(builder, load);
                    // Replace original uses with the invertex vector.
                    for (auto loadUse : oldUses)
                        builder.replaceOperand(loadUse, invertedVal);
                }
                else if (auto getElementPtr = as<IRGetElementPtr>(use->getUser()))
                {
                    traverseUses(getElementPtr, [&](IRUse* use) { useWorkList.add(use); });
                }
            };
            traverseUses(globalInst, processUse);
            for (Index i = 0; i < useWorkList.getCount(); i++)
            {
                processUse(useWorkList[i]);
            }
        }
    }
}

// Return `originalVector` with its z (index 2) component remapped from the OpenGL clip-space
// depth range [-w, w] to the standard [0, w] range (NDC z in [-1, 1] -> [0, 1] after the
// perspective divide), computing z' = (z + w) / 2. Unlike `_invertYOfVector`, the map is
// affine and reads both z and w, so it must operate on the full position vector.
static IRInst* _remapZOfVector(IRBuilder& builder, IRInst* originalVector)
{
    auto vectorType = as<IRVectorType>(originalVector->getDataType());
    SLANG_ASSERT(vectorType);
    auto elementType = vectorType->getElementType();
    UInt elementIndexZ = 2;
    UInt elementIndexW = 3;
    auto originalZ = builder.emitSwizzle(elementType, originalVector, 1, &elementIndexZ);
    auto originalW = builder.emitSwizzle(elementType, originalVector, 1, &elementIndexW);
    auto sum = builder.emitAdd(elementType, originalZ, originalW);
    auto remappedZ = builder.emitDiv(elementType, sum, builder.getFloatValue(elementType, 2.0));
    auto newVal = builder.emitSwizzleSet(
        originalVector->getDataType(),
        originalVector,
        remappedZ,
        1,
        &elementIndexZ);
    return newVal;
}

// Find outputs to SV_Position and remap their clip-space z right before the write, so a shader
// authored against the OpenGL [-1, 1] NDC depth convention emits the standard [0, 1] depth.
// This is the output/store path only; the affine remap needs the full float4 to read w, so
// partial single-component writes to the position are intentionally left untouched. The pass is
// scheduled exclusively for the GLSL target on vertex entry points (see linkAndOptimizeIR in
// slang-emit.cpp), so it performs no target/stage checks of its own.
void remapZOfPositionOutput(IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->findDecoration<IRGLPositionOutputDecoration>())
        {
            // Find all stores to it (including those reached through a getElementPtr).
            IRBuilder builder(module);
            List<IRUse*> useWorkList;
            auto processUse = [&](IRUse* use)
            {
                if (auto store = as<IRStore>(use->getUser()))
                {
                    if (getRootAddr(store->getPtr()) != globalInst)
                        return;

                    // Only a full-vector write can be remapped, since z' depends on w; skip
                    // partial single-component stores (e.g. a write to just `.z`).
                    auto originalVal = store->getVal();
                    if (!as<IRVectorType>(originalVal->getDataType()))
                        return;

                    builder.setInsertBefore(store);
                    auto remappedVal = _remapZOfVector(builder, originalVal);
                    builder.replaceOperand(&store->val, remappedVal);
                }
                else if (auto getElementPtr = as<IRGetElementPtr>(use->getUser()))
                {
                    traverseUses(getElementPtr, [&](IRUse* use) { useWorkList.add(use); });
                }
            };
            traverseUses(globalInst, processUse);
            for (Index i = 0; i < useWorkList.getCount(); i++)
            {
                processUse(useWorkList[i]);
            }
        }
    }
}


static IRInst* _invertWOfVector(IRBuilder& builder, IRInst* originalVector)
{
    auto vectorType = as<IRVectorType>(originalVector->getDataType());
    SLANG_ASSERT(vectorType);
    UInt elementIndexW = 3;
    auto originalW =
        builder.emitSwizzle(vectorType->getElementType(), originalVector, 1, &elementIndexW);
    auto rcpW = builder.emitDiv(
        originalW->getDataType(),
        builder.getFloatValue(originalW->getDataType(), 1.0),
        originalW);
    auto newVal =
        builder
            .emitSwizzleSet(originalVector->getDataType(), originalVector, rcpW, 1, &elementIndexW);
    return newVal;
}

// Find inputs of SV_Position and rcp the w coordinates of it right after the read.
void rcpWOfPositionInput(IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->findDecoration<IRGLPositionInputDecoration>())
        {
            // Find all loads and replace them with reciprocals.
            IRBuilder builder(module);
            traverseUses(
                globalInst,
                [&](IRUse* use)
                {
                    // Get the inverted vector.
                    auto user = use->getUser();
                    if (user->getOp() == kIROp_Load)
                    {
                        builder.setInsertBefore(user);
                        auto val = builder.emitLoad(globalInst);
                        auto invertedVal = _invertWOfVector(builder, val);
                        user->replaceUsesWith(invertedVal);
                        user->removeAndDeallocate();
                    }
                });
        }
    }
}
} // namespace Slang
