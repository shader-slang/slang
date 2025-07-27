#include "slang-ir-defer-buffer-load.h"

#include "slang-ir-clone.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-redundancy-removal.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct DeferBufferLoadContext
{
    // Map an original SSA value to a pointer that can be used to load the value.
    Dictionary<IRInst*, IRInst*> mapValueToPtr;

    // Map an ptr to its loaded value.
    Dictionary<IRInst*, IRInst*> mapPtrToValue;

    IRFunc* currentFunc = nullptr;

    // Ensure that for an original SSA value, we have formed a pointer that can be used to load the
    // value.
    IRInst* ensurePtr(IRInst* valueInst)
    {
        IRInst* result = nullptr;
        if (mapValueToPtr.tryGetValue(valueInst, result))
            return result;

        IRBuilder b(valueInst);
        b.setInsertBefore(valueInst);

        switch (valueInst->getOp())
        {
        case kIROp_StructuredBufferLoad:
        case kIROp_StructuredBufferLoadStatus:
            {
                result = b.emitRWStructuredBufferGetElementPtr(
                    valueInst->getOperand(0),
                    valueInst->getOperand(1));
                break;
            }
        case kIROp_GetElement:
            {
                auto ptr = ensurePtr(valueInst->getOperand(0));
                if (!ptr)
                    return nullptr;
                result = b.emitElementAddress(ptr, valueInst->getOperand(1));
                break;
            }
        case kIROp_FieldExtract:
            {
                auto ptr = ensurePtr(valueInst->getOperand(0));
                if (!ptr)
                    return nullptr;
                result = b.emitFieldAddress(ptr, valueInst->getOperand(1));
                break;
            }
        case kIROp_Load:
            result = valueInst->getOperand(0);
            break;
        }
        if (result)
        {
            mapValueToPtr[valueInst] = result;
        }
        return result;
    }

    static bool isImmutableLocation(IRInst* loc)
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

    static bool isImmutableBufferLoad(IRInst* inst)
    {
        // Note: we cannot defer loads from RWStructuredBuffer because there can be other
        // instructions that modify the buffer.
        switch (inst->getOp())
        {
        case kIROp_StructuredBufferLoad:
        case kIROp_StructuredBufferLoadStatus:
            return true;
        case kIROp_Load:
            {
                auto rootAddr = getRootAddr(inst->getOperand(0));
                return isImmutableLocation(rootAddr);
            }
        default:
            return false;
        }
    }

    // Ensure that for a pointer value, we have created a load instruction to materialize the value.
    IRInst* materializePointer(IRBuilder& builder, IRInst* loadInst)
    {
        auto ptr = ensurePtr(loadInst);
        if (!ptr)
            return nullptr;
        IRInst* result = nullptr;
        if (mapPtrToValue.tryGetValue(ptr, result))
            return result;
        IRAlignedAttr* align = nullptr;
        if (auto load = as<IRLoad>(loadInst))
            align = load->findAttr<IRAlignedAttr>();
        if (!as<IRModuleInst>(ptr->getParent()))
        {
            builder.setInsertAfter(ptr);
            IRType* valueType = tryGetPointedToType(&builder, ptr->getFullType());
            result = builder.emitLoad(valueType, ptr, align);
            mapPtrToValue[ptr] = result;
        }
        else
        {
            builder.setInsertBefore(loadInst);
            IRType* valueType = tryGetPointedToType(&builder, ptr->getFullType());
            result = builder.emitLoad(valueType, ptr, align);
            // Since we are inserting the load in a local scope, we can't register
            // the mapping to the pointer, since the global pointer needs to be
            // loaded once per function.
        }
        return result;
    }

    static bool isSimpleType(IRInst* type)
    {
        if (auto modType = as<IRRateQualifiedType>(type))
            type = modType->getValueType();
        if (as<IRStructType>(type))
            return false;
        if (as<IRTupleType>(type))
            return false;
        if (as<IRArrayTypeBase>(type))
            return false;
        return true;
    }

    void deferBufferLoadInst(IRBuilder& builder, List<IRInst*>& workList, IRInst* loadInst)
    {
        // Don't defer the load anymore if the type is simple.
        if (isSimpleType(loadInst->getDataType()) || loadInst->findAttr<IRAlignedAttr>())
        {
            auto materializedVal = materializePointer(builder, loadInst);
            loadInst->transferDecorationsTo(materializedVal);
            loadInst->replaceUsesWith(materializedVal);
            return;
        }

        // Otherwise, look for all uses and try to defer the load before actual use of the value.
        ShortList<IRInst*> pendingWorkList;
        bool needMaterialize = false;
        traverseUses(
            loadInst,
            [&](IRUse* use)
            {
                if (needMaterialize)
                    return;

                auto user = use->getUser();
                switch (user->getOp())
                {
                case kIROp_GetElement:
                case kIROp_FieldExtract:
                    {
                        auto basePtr = ensurePtr(loadInst);
                        if (!basePtr)
                            return;
                        pendingWorkList.add(user);
                    }
                    break;
                default:
                    needMaterialize = true;
                    return;
                }
            });

        if (needMaterialize)
        {
            auto val = materializePointer(builder, loadInst);
            loadInst->transferDecorationsTo(val);
            loadInst->replaceUsesWith(val);
            loadInst->removeAndDeallocate();
        }
        else
        {
            // Append to worklist in reverse order so we process the uses in natural appearance
            // order.
            for (Index i = pendingWorkList.getCount() - 1; i >= 0; i--)
                workList.add(pendingWorkList[i]);
        }
    }

    void deferBufferLoadInFunc(IRFunc* func)
    {
        removeRedundancyInFunc(func, false);

        currentFunc = func;

        List<IRInst*> workList;

        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (isImmutableBufferLoad(inst))
                {
                    workList.add(inst);
                }
            }
        }

        IRBuilder builder(func);
        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto inst = workList[i];
            deferBufferLoadInst(builder, workList, inst);
        }
    }

    void deferBufferLoad(IRGlobalValueWithCode* inst)
    {
        if (auto func = as<IRFunc>(inst))
        {
            deferBufferLoadInFunc(func);
        }
        else if (auto generic = as<IRGeneric>(inst))
        {
            auto inner = findGenericReturnVal(generic);
            if (auto innerFunc = as<IRFunc>(inner))
                deferBufferLoadInFunc(innerFunc);
        }
    }
};

void deferBufferLoad(IRModule* module)
{
    DeferBufferLoadContext context;
    for (auto childInst : module->getGlobalInsts())
    {
        if (auto code = as<IRGlobalValueWithCode>(childInst))
        {
            context.deferBufferLoad(code);
        }
    }
}

} // namespace Slang
