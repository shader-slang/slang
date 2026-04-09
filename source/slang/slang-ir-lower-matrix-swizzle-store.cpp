// slang-ir-lower-matrix-swizzle-store.cpp
#include "slang-ir-lower-matrix-swizzle-store.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

static void lowerMatrixSwizzleStore(IRBuilder& builder, IRMatrixSwizzleStore* inst)
{
    const UInt elementCount = inst->getElementCount();
    IRInst* dest = inst->getDest();
    IRInst* source = inst->getSource();

    const UInt maxRowIndex = 4;
    const UInt maxCols = 4;

    // Group elements by row.
    UInt rowSizes[maxRowIndex] = {};
    uint32_t rowCols[maxRowIndex][maxCols];
    UInt rowSourceIndices[maxRowIndex][maxCols];

    for (UInt i = 0; i < elementCount; ++i)
    {
        auto rowConst = as<IRIntLit>(inst->getElementRow(i));
        auto colConst = as<IRIntLit>(inst->getElementCol(i));
        SLANG_ASSERT(rowConst && colConst);

        UInt row = (UInt)rowConst->getValue();
        auto& rowSize = rowSizes[row];
        rowCols[row][rowSize] = (uint32_t)colConst->getValue();
        rowSourceIndices[row][rowSize] = i;
        ++rowSize;
    }

    // Determine element type of source (null if source is a scalar).
    IRType* sourceElemType = nullptr;
    if (auto vecType = as<IRVectorType>(source->getDataType()))
        sourceElemType = vecType->getElementType();

    builder.setInsertBefore(inst);

    for (UInt r = 0; r < maxRowIndex; ++r)
    {
        if (rowSizes[r] == 0)
            continue;

        auto rowAddr = builder.emitElementAddress(dest, r);

        if (rowSizes[r] == 1)
        {
            // Single element — use a direct store instead of SwizzledStore,
            // which expects a vector-typed source.
            auto elemAddr = builder.emitElementAddress(rowAddr, (IRIntegerValue)rowCols[r][0]);
            IRInst* elemVal;
            if (sourceElemType)
                elemVal =
                    builder.emitElementExtract(source, (IRIntegerValue)rowSourceIndices[r][0]);
            else
                elemVal = source;
            builder.emitStore(elemAddr, elemVal);
        }
        else
        {
            IRInst* rSwizzled;
            if (sourceElemType)
            {
                // Source is a vector — select the elements for this row.
                rSwizzled = builder.emitSwizzle(
                    builder.getVectorType(sourceElemType, rowSizes[r]),
                    source,
                    rowSizes[r],
                    rowSourceIndices[r]);
            }
            else
            {
                // Source is a scalar.
                rSwizzled = source;
            }
            builder.emitSwizzledStore(rowAddr, rSwizzled, rowSizes[r], rowCols[r]);
        }
    }

    inst->removeAndDeallocate();
}

void lowerMatrixSwizzleStores(IRModule* module)
{
    List<IRMatrixSwizzleStore*> instsToLower;

    auto collectFromFunc = [&](IRFunc* func)
    {
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (auto matSwizzleStore = as<IRMatrixSwizzleStore>(inst))
                    instsToLower.add(matSwizzleStore);
            }
        }
    };
    for (auto globalInst : module->getFuncs())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;
        collectFromFunc(func);
    }
    for (auto globalInst : module->getGenerics())
    {
        auto gen = as<IRGeneric>(globalInst);
        if (!gen)
            continue;
        auto func = as<IRFunc>(findGenericReturnVal(gen));
        if (!func)
            continue;
        collectFromFunc(func);
    }

    if (instsToLower.getCount() == 0)
        return;

    IRBuilder builder(module);
    for (auto inst : instsToLower)
    {
        lowerMatrixSwizzleStore(builder, inst);
    }
}

} // namespace Slang
