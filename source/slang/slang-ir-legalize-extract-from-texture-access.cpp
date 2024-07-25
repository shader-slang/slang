#include "slang-ir-legalize-extract-from-texture-access.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-clone.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-parameter-binding.h"
#include "slang-ir-legalize-image-subscript.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-simplify-cfg.h"

namespace Slang
{
    void legalizeExtractTextureFromTextureAccess(IRBuilder& builder, IRInst* inst)
    {
        SLANG_ASSERT(inst);

        builder.setInsertBefore(inst);
        IRImageSubscript* imageSubscript = as<IRImageSubscript>(getRootAddr(inst->getOperand(0)));
        SLANG_ASSERT(imageSubscript);
        SLANG_ASSERT(imageSubscript->getImage());
        inst->replaceUsesWith(imageSubscript->getImage());
        inst->removeAndDeallocate();
        // Ensure we are done processing the imageSubscript before we remove it
        if (!imageSubscript->hasUses())
            imageSubscript->removeAndDeallocate();
    }

    void legalizeExtractArrayCoordFromTextureAccess(IRBuilder& builder, IRInst* inst)
    {
        SLANG_ASSERT(inst);

        builder.setInsertBefore(inst);
        IRImageSubscript* imageSubscript = as<IRImageSubscript>(getRootAddr(inst->getOperand(0)));
        SLANG_ASSERT(imageSubscript);
        SLANG_ASSERT(imageSubscript->getImage());
        
        auto image = as<IRTextureType>(imageSubscript->getImage()->getDataType());
        IRInst* coord = imageSubscript->getCoord();
        if(image->isArray())
        {
            // Extract final element which is 'ArrayCoord'
            IRVectorType* coordType = as<IRVectorType>(imageSubscript->getCoord()->getDataType());
            SLANG_ASSERT(coordType);
            auto coordSize = getIRVectorElementSize(coordType);

            IRType* newArrayCoordType = coordType->getElementType();
            auto arrayCoordLocation = coordSize - 1;
            List<UInt> swizzleIndicies = { (UInt)arrayCoordLocation };
            
            coord = builder.emitSwizzle(newArrayCoordType, coord, 1, swizzleIndicies.getBuffer());
        }
        else
            coord = builder.getIntValue(builder.getUIntType(), 0);


        inst->replaceUsesWith(coord);
        inst->removeAndDeallocate();
        // Ensure we are done processing the imageSubscript completly before we remove it
        if (!imageSubscript->hasUses())
            imageSubscript->removeAndDeallocate();
    }

    void legalizeExtractCoordFromTextureAccess(IRBuilder& builder, IRInst* inst)
    {
        SLANG_ASSERT(inst);

        builder.setInsertBefore(inst);
        IRImageSubscript* imageSubscript = as<IRImageSubscript>(getRootAddr(inst->getOperand(0)));
        SLANG_ASSERT(imageSubscript);
        SLANG_ASSERT(imageSubscript->getImage());
        
        auto image = as<IRTextureType>(imageSubscript->getImage()->getDataType());
        IRInst* coord = imageSubscript->getCoord();
        if(image->isArray())
        {
            // Extract all but final element which is 'ArrayCoord'
            IRVectorType* coordType = as<IRVectorType>(imageSubscript->getCoord()->getDataType());
            auto coordSize = getIRVectorElementSize(coordType);
            SLANG_ASSERT(coordType);
            
            IRType* newCoordType = nullptr;
            auto newCoordSize = coordSize - 1;
            if(newCoordSize != 1)
                newCoordType = builder.getVectorType(coordType->getElementType(), newCoordSize);
            else
                newCoordType = coordType->getElementType();
            List<UInt> swizzleIndicies = {1, 2, 3, 4};
            
            coord = builder.emitSwizzle(newCoordType, coord, newCoordSize, swizzleIndicies.getBuffer());
        }

        inst->replaceUsesWith(coord);
        inst->removeAndDeallocate();
        // Ensure we are done processing the imageSubscript completly before we remove it
        if (!imageSubscript->hasUses())
            imageSubscript->removeAndDeallocate();
    }

    void legalizeExtractFromTextureAccess(IRModule* module)
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
                for ( ; inst; inst = next)
                {
                    next = inst->getNextInst();
                    switch (inst->getOp())
                    {
                    case kIROp_ExtractArrayCoordFromTextureAccess:
                        if (as<IRImageSubscript>(getRootAddr(inst->getOperand(0))))
                            legalizeExtractArrayCoordFromTextureAccess(builder, inst);
                        continue;
                    case kIROp_ExtractCoordFromTextureAccess:
                        if (as<IRImageSubscript>(getRootAddr(inst->getOperand(0))))
                            legalizeExtractCoordFromTextureAccess(builder, inst);
                        continue;
                    case kIROp_ExtractTextureFromTextureAccess:
                        if (as<IRImageSubscript>(getRootAddr(inst->getOperand(0))))
                            legalizeExtractTextureFromTextureAccess(builder, inst);
                        continue;
                    }
                }   
            }
        }
    }
}

