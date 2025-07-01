#include "slang-defer-immutable-buffer-load.h"

#include "slang-ir-util.h"

namespace Slang
{
struct DeferImmutableBufferLoadContext
{
    IRModule* module;

    bool isImmutableLocation(IRInst* loc)
    {
        switch (loc->getOp())
        {
        case kIROp_GetStructuredBufferPtr:
        case kIROp_ImageSubscript:
            return isImmutableLocation(loc->getOperand(0));
        default:
            break;
        }

        auto type = loc->getDataType();
        if (!type)
            return false;

        switch (type->getOp())
        {
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLByteAddressBufferType:
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
            return true;
        default:
            break;
        }

        if (auto textureType = as<IRTextureType>(type))
            return textureType->getAccess() == SLANG_RESOURCE_ACCESS_READ;

        return false;
    }

    // Defer loads of immutable buffer elements to as late as possible inside the function.
    // If we see `load(parameterBlock).field1.array2[3]`, we should push the load to the far end of
    // the leaf element, so we only load the relevant element that is actually used.
    // In this case, we will replace the instruction stream with
    // `load(parameterBlock.field1.array2[3])`.
    //
    void processFunc(IRGlobalValueWithCode* func)
    {
        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;

        auto addToWorkList = [&](IRInst* inst)
        {
            if (workListSet.add(inst))
            {
                workList.add(inst);
            }
        };

        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (auto loadInst = as<IRLoad>(inst))
                {
                    auto addrRoot = getRootAddr(loadInst->getPtr());
                    if (!isImmutableLocation(addrRoot))
                        continue;

                    addToWorkList(loadInst);
                }
            }
        }

        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto& loadInst = workList[i];
            auto baseAddr = loadInst->getOperand(0);
            traverseUses(
                loadInst,
                [&](IRUse* use)
                {
                    auto user = use->getUser();
                    switch (user->getOp())
                    {
                    case kIROp_GetElement:
                        {
                            IRBuilder builder(user);
                            builder.setInsertBefore(user);
                            auto newAddr = builder.emitElementAddress(
                                baseAddr,
                                as<IRGetElement>(user)->getIndex());
                            auto newLoad = builder.emitLoad(newAddr);
                            user->replaceUsesWith(newLoad);
                            addToWorkList(newLoad);
                        }
                        break;
                    case kIROp_FieldExtract:
                        {
                            IRBuilder builder(user);
                            builder.setInsertBefore(user);
                            auto newAddr = builder.emitFieldAddress(
                                baseAddr,
                                as<IRFieldExtract>(user)->getField());
                            auto newLoad = builder.emitLoad(newAddr);
                            user->replaceUsesWith(newLoad);
                            addToWorkList(newLoad);
                        }
                        break;
                    }
                });
            if (!loadInst->hasUses())
                loadInst->removeAndDeallocate();
        }
    }

    void processModule()
    {
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRGlobalValueWithCode>(inst))
                processFunc(func);
        }
    };
};

void deferImmutableBufferLoad(IRModule* module)
{
    DeferImmutableBufferLoadContext context;
    context.module = module;
    context.processModule();
}

} // namespace Slang
