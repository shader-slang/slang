#include "slang-ir-address-analysis.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-pairs.h"
#include "slang-ir-autodiff-rev.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-single-return.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-validate.h"

namespace Slang
{
bool isDifferentiableType(DifferentiableTypeConformanceContext& context, IRInst* typeInst);

struct AddressInstEliminationContext
{
    OrderedDictionary<IRInst*, IRInst*> mapAddrInstToTempVar;

    IRInst* _reconstructStruct(
        IRBuilder& builder, IRStructType* type, IRInst* tempVar, List<AddressInfo*>& childAddrs)
    {
        List<IRInst*> args;
        IRInst* loadedTempVar = nullptr;
        for (auto child : type->getChildren())
        {
            if (auto field = as<IRStructField>(child))
            {
                IRInst* childVar = nullptr;
                for (auto subAddr : childAddrs)
                {
                    auto fieldAddrInst = cast<IRFieldAddress>(subAddr->addrInst);
                    if (fieldAddrInst->getField() == field->getKey())
                    {
                        mapAddrInstToTempVar.TryGetValue(subAddr->addrInst, childVar);
                        break;
                    }
                }
                if (childVar)
                {
                    args.add(builder.emitLoad(childVar));
                }
                else
                {
                    if (!loadedTempVar)
                        loadedTempVar = builder.emitLoad(tempVar);
                    args.add(builder.emitFieldExtract(
                        field->getFieldType(), loadedTempVar, field->getKey()));
                }
            }
        }
        return builder.emitMakeStruct(type, args);
    }

    IRInst* _reconstructArray(
        IRBuilder& builder,
        IRArrayType* type,
        IRIntegerValue arraySize,
        IRInst* tempVar,
        List<AddressInfo*>& childAddrs)
    {
        IRInst* loadedTempVar = nullptr;
        List<IRInst*> args;
        for (IRIntegerValue index = 0; index < arraySize; index++)
        {
            IRInst* childVar = nullptr;
            for (auto subAddr : childAddrs)
            {
                auto elementPtrInst = cast<IRGetElementPtr>(subAddr->addrInst);
                auto elementIndex = as<IRIntLit>(elementPtrInst->getIndex());
                if (elementIndex && elementIndex->getValue() == index)
                {
                    mapAddrInstToTempVar.TryGetValue(subAddr->addrInst, childVar);
                    break;
                }
            }
            if (childVar)
            {
                args.add(builder.emitLoad(childVar));
            }
            else
            {
                if (!loadedTempVar)
                    loadedTempVar = builder.emitLoad(tempVar);
                args.add(builder.emitElementExtract(
                    type->getElementType(),
                    loadedTempVar,
                    builder.getIntValue(builder.getIntType(), index)));
            }
        }
        return builder.emitMakeArray(type, args.getCount(), args.getBuffer());
    }

    void updateChildTempVarRecursive(
        IRBuilder& builder,
        AddressInfo* addr,
        IRInst* val)
    {
        for (auto child : addr->children)
        {
            IRInst* childVar = nullptr;
            if (mapAddrInstToTempVar.TryGetValue(child->addrInst, childVar))
            {
                switch (child->addrInst->getOp())
                {
                case kIROp_FieldAddress:
                    {
                        auto subVal = builder.emitFieldExtract(
                            cast<IRPtrTypeBase>(child->addrInst->getDataType())->getValueType(),
                            val,
                            child->addrInst->getOperand(1));
                        builder.emitStore(childVar, subVal);
                        updateChildTempVarRecursive(builder, child, subVal);
                    }
                    break;
                case kIROp_GetElementPtr:
                    {
                        auto subVal = builder.emitElementExtract(
                            cast<IRPtrTypeBase>(child->addrInst->getDataType())->getValueType(),
                            val,
                            child->addrInst->getOperand(1));
                        builder.emitStore(childVar, subVal);
                        updateChildTempVarRecursive(builder, child, subVal);
                    }
                    break;
                default:
                    {
                    }
                    break;
                }
            }
        }
    }

    IRInst* getLoadedValue(
        IRBuilder& builder,
        AddressInfo* addr,
        IRInst* tempVar)
    {
        if (addr->children.getCount())
        {
            // Reconstruct val.
            auto type =
                cast<IRPtrTypeBase>(unwrapAttributedType(tempVar->getFullType()))->getValueType();
            switch (type->getOp())
            {
            case kIROp_StructType:
                return _reconstructStruct(
                    builder, as<IRStructType>(type), tempVar, addr->children);
            case kIROp_ArrayType:
                {
                    auto arrayType = as<IRArrayType>(type);
                    auto size = as<IRIntLit>(arrayType->getElementCount());
                    if (!size || size->getValue() < 0)
                    {
                        // Unsupported array type.
                    }
                    else
                    {
                        return _reconstructArray(
                            builder,
                            arrayType,
                            size->getValue(),
                            tempVar,
                            addr->children);
                    }
                }
                break;
            default:
                // Unsupported address type.
                break;
            }
        }
        return builder.emitLoad(tempVar);
    };

    void updateParentTempVarRecursive(
        IRBuilder& builder,
        AddressInfo* addr)
    {
        for (auto parent = addr->parentAddress; parent; parent = parent->parentAddress)
        {
            IRInst* parentVar = nullptr;
            if (mapAddrInstToTempVar.TryGetValue(parent->addrInst, parentVar))
            {
                auto val = getLoadedValue(builder, parent, parentVar);
                builder.emitStore(parentVar, val);
            }
        }
    }

    String getAddrName(IRInst* addrInst)
    {
        StringBuilder sb;
        List<IRInst*> bases;
        bases.add(addrInst);
        for (; addrInst;)
        {
            if (auto fieldAddr = as<IRFieldAddress>(addrInst))
                bases.add(fieldAddr->getBase());
            else if (auto index = as<IRGetElementPtr>(addrInst))
                bases.add(index->getBase());
            else
                break;
        }
        for (Index i = bases.getCount() - 1; i >= 0; i--)
        {
            if (bases[i]->getOp() == kIROp_FieldAddress)
            {
                sb << ".";
                auto field = bases[i]->getOperand(1);
                auto nameDecor = field->findDecoration<IRNameHintDecoration>();
                sb << (nameDecor ? nameDecor->getName() : UnownedStringSlice("<unknown>"));
            }
            else if (bases[i]->getOp() == kIROp_FieldAddress)
            {
                sb << "[";
                auto index = bases[i]->getOperand(1);
                auto nameDecor = index->findDecoration<IRNameHintDecoration>();
                if (nameDecor)
                {
                    sb << nameDecor->getName();
                }
                else if (auto intLit = as<IRIntLit>(index))
                {
                    sb << intLit->getValue();
                }
                else
                {
                    sb << "...";
                }
                sb << "]";
            }
            else
            {
                auto nameDecor = bases[i]->findDecoration<IRNameHintDecoration>();
                sb << (nameDecor ? nameDecor->getName() : UnownedStringSlice("<unknown>"));
            }
        }
        return sb.ProduceString();
    }

    SlangResult eliminateAddressInstsImpl(
        SharedIRBuilder* sharedBuilder,
        DifferentiableTypeConformanceContext& diffContext,
        IRFunc* func,
        DiagnosticSink* sink)
    {
        bool hasError = false;

        if (!isSingleReturnFunc(func))
        {
            convertFuncToSingleReturnForm(func->getModule(), func);
        }

        IRBuilder builder(sharedBuilder);

        auto dom = computeDominatorTree(func);
        auto addrUse = analyzeAddressUse(dom, func);
        List<AddressInfo*> workList;
        HashSet<AddressInfo*> workListSet;

        // Process leaf addresses first.
        for (auto addr : addrUse.addressInfos)
        {
            if (addr.Value->children.getCount() == 0)
                workList.add(addr.Value);
        }

        auto createTempVarForAddr = [&](IRInst* addrInst)
        {
            if (as<IRParam>(addrInst))
                builder.setInsertAfter(as<IRBlock>(addrInst->getParent())->getLastParam());
            else
                builder.setInsertAfter(addrInst);
            auto ptrType = as<IRPtrTypeBase>(addrInst->getFullType());
            SLANG_RELEASE_ASSERT(ptrType);
            auto tempVar = builder.emitVar(ptrType->getValueType());
            mapAddrInstToTempVar[addrInst] = tempVar;
        };

        // In the first pass, we create temp vars for addresses with non-trivial access pattern.
        for (Index workListIndex = 0; workListIndex < workList.getCount(); workListIndex++)
        {
            auto addr = workList[workListIndex];

            if (!isDifferentiableType(diffContext, addr->addrInst->getDataType()))
                continue;

            List<IRUse*> readUses, writeUses, callUses, subAddrUses, unknownUses;

            for (auto node = addr; node; node = node->parentAddress)
            {
                auto addrInst = node->addrInst;

                for (auto use = addrInst->firstUse; use; use = use->nextUse)
                {
                    if (as<IRDecoration>(use->getUser()))
                        continue;
                    switch (use->getUser()->getOp())
                    {
                    case kIROp_Load:
                        readUses.add(use);
                        break;
                    case kIROp_Store:
                        writeUses.add(use);
                        break;
                    case kIROp_Call:
                        callUses.add(use);
                        break;
                    case kIROp_GetElementPtr:
                    case kIROp_FieldAddress:
                        if (node == addr)
                            subAddrUses.add(use);
                        break;
                    default:
                        unknownUses.add(use);
                        break;
                    }
                }
            }

            if (unknownUses.getCount() != 0)
            {
                // Diagnose about unknown use.
                sink->diagnose(
                    unknownUses.getFirst()->getUser(),
                    Diagnostics::unsupportedUseOfLValueForAutoDiff);
                hasError = true;
                continue;
            }

            if (addr->isConstant)
            {
                // Otherwise, the address must be a constant, and we need to create a temp var for
                // it. The exception is when the variable is a temp var for a call.
                if (callUses.getCount() == 1 && writeUses.getCount() <= 1 &&
                    readUses.getCount() <= 1)
                {
                    if (writeUses.getCount() == 0)
                        continue;

                    // The uses must be in write->call->read order.
                    auto callUse = callUses.getFirst();
                    auto writeUse = writeUses.getFirst();
                    auto readUse = readUses.getCount() ? readUses.getFirst() : writeUse;
                    if (dom->dominates(writeUse->getUser(), callUse->getUser()) &&
                        dom->dominates(callUse->getUser(), readUse->getUser()))
                    {
                        continue;
                    }
                }

                // Create a temp var for the address and replace all uses of the address to the temp
                // var.
                createTempVarForAddr(addr->addrInst);
            }
            else
            {
                // This is a dynamic address. We can only allow at most one write access to it.
                bool hasNonTrivialAccess = false;
                if (readUses.getCount() + callUses.getCount() != 0 &&
                    writeUses.getCount() + callUses.getCount() > 1)
                    hasNonTrivialAccess = true;

                if (hasNonTrivialAccess)
                {
                    // Mixed use of a non-constant address is unsupported right now.
                    sink->diagnose(
                        addr->addrInst,
                        Diagnostics::cannotDifferentiateDynamicallyIndexedData,
                        getAddrName(addr->addrInst));
                }
            }
            if (addr->parentAddress && workListSet.Add(addr->parentAddress))
                workList.add(addr->parentAddress);
        }

        if (hasError)
            return SLANG_FAIL;

        // Actually replace addresses with temp vars.
        for (auto addr : workList)
        {
            IRInst* tempVar = nullptr;
            if (!mapAddrInstToTempVar.TryGetValue(addr->addrInst, tempVar))
                continue;
            for (auto use = addr->addrInst->firstUse; use;)
            {
                auto nextUse = use->nextUse;
                auto user = use->getUser();

                builder.setInsertBefore(user);
                switch (user->getOp())
                {
                case kIROp_Load:
                    use->set(tempVar);
                    break;
                case kIROp_Store:
                    use->set(tempVar);
                    updateChildTempVarRecursive(
                        builder, addr, as<IRStore>(user)->getVal());
                    updateParentTempVarRecursive(builder, addr);
                case kIROp_Call:
                    {
                        use->set(tempVar);
                        builder.setInsertAfter(user);
                        auto newVal = builder.emitLoad(tempVar);
                        updateChildTempVarRecursive(builder, addr, newVal);
                        updateParentTempVarRecursive(builder, addr);
                    }
                    break;
                default:
                    use->set(tempVar);
                    break;
                }
                use = nextUse;
            }
        }

        // Assign initial values to tempVar.
        for (auto tempVar : mapAddrInstToTempVar)
        {
            builder.setInsertAfter(tempVar.Value);
            IRInst* initVal = nullptr;
            if (tempVar.Key->getOp() == kIROp_Var ||
                tempVar.Key->getOp() == kIROp_Param && as<IROutType>(tempVar.Key->getFullType()))
            {
                initVal = builder.emitDefaultConstruct(
                    cast<IRPtrTypeBase>(tempVar.Key->getFullType())->getValueType());
            }
            else
            {
                initVal = builder.emitLoad(tempVar.Key);
            }
            builder.emitStore(tempVar.Value, initVal);
        }

        // Store final values to out parameters before exiting function.
        IRInst* returnInst = nullptr;
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (inst->getOp() == kIROp_Return)
                {
                    returnInst = inst;
                    break;
                }
            }
        }
        SLANG_RELEASE_ASSERT(returnInst);
        builder.setInsertBefore(returnInst);
        for (auto param : func->getParams())
        {
            IRInst* tempVar = nullptr;
            if (mapAddrInstToTempVar.TryGetValue(param, tempVar))
            {
                auto val = builder.emitLoad(tempVar);
                builder.emitStore(param, val);
            }
        }
        if (hasError)
            return SLANG_FAIL;
        return SLANG_OK;
    }
};

SlangResult eliminateAddressInsts(
    SharedIRBuilder* sharedBuilder,
    DifferentiableTypeConformanceContext& diffContext,
    IRFunc* func,
    DiagnosticSink* sink)
{
    AddressInstEliminationContext ctx;
    return ctx.eliminateAddressInstsImpl(sharedBuilder, diffContext, func, sink);
}
} // namespace Slang
