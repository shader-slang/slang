#include "slang-ir-util.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"

namespace Slang
{

bool isPointerOfType(IRInst* type, IROp opCode)
{
    if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        return ptrType->getValueType() && ptrType->getValueType()->getOp() == opCode;
    }
    return false;
}

Dictionary<IRInst*, IRInst*> buildInterfaceRequirementDict(IRInterfaceType* interfaceType)
{
    Dictionary<IRInst*, IRInst*> result;
    for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
    {
        auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i));
        if (!entry) continue;
        result[entry->getRequirementKey()] = entry->getRequirementVal();
    }
    return result;
}

bool isPointerOfType(IRInst* type, IRInst* elementType)
{
    if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        return ptrType->getValueType() && isTypeEqual(ptrType->getValueType(), (IRType*)elementType);
    }
    return false;
}

bool isPtrToClassType(IRInst* type)
{
    return isPointerOfType(type, kIROp_ClassType);
}

bool isPtrToArrayType(IRInst* type)
{
    return isPointerOfType(type, kIROp_ArrayType) || isPointerOfType(type, kIROp_UnsizedArrayType);
}


bool isComInterfaceType(IRType* type)
{
    if (!type) return false;
    if (type->findDecoration<IRComInterfaceDecoration>() ||
        type->getOp() == kIROp_ComPtrType)
    {
        return true;
    }
    if (auto witnessTableType = as<IRWitnessTableTypeBase>(type))
    {
        return isComInterfaceType((IRType*)witnessTableType->getConformanceType());
    }
    if (auto ptrType = as<IRNativePtrType>(type))
    {
        auto valueType = ptrType->getValueType();
        return valueType->findDecoration<IRComInterfaceDecoration>() != nullptr;
    }

    return false;
}

IROp getTypeStyle(IROp op)
{
    switch (op)
    {
    case kIROp_VoidType:
    case kIROp_BoolType:
    {
        return op;
    }
    case kIROp_Int8Type:
    case kIROp_Int16Type:
    case kIROp_IntType:
    case kIROp_UInt8Type:
    case kIROp_UInt16Type:
    case kIROp_UIntType:
    case kIROp_Int64Type:
    case kIROp_UInt64Type:
    case kIROp_IntPtrType:
    case kIROp_UIntPtrType:
    {
        // All int like 
        return kIROp_IntType;
    }
    case kIROp_HalfType:
    case kIROp_FloatType:
    case kIROp_DoubleType:
    {
        // All float like
        return kIROp_FloatType;
    }
    default: return kIROp_Invalid;
    }
}

IROp getTypeStyle(BaseType op)
{
    switch (op)
    {
    case BaseType::Void:
        return kIROp_VoidType;
    case BaseType::Bool:
        return kIROp_BoolType;
    case BaseType::Char:
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
    case BaseType::IntPtr:
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
    case BaseType::UIntPtr:
        return kIROp_IntType;
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
        return kIROp_FloatType;
    default:
        return kIROp_Invalid;
    }
}

IRInst* specializeWithGeneric(IRBuilder& builder, IRInst* genericToSpecialize, IRGeneric* userGeneric)
{
    List<IRInst*> genArgs;
    for (auto param : userGeneric->getFirstBlock()->getParams())
    {
        genArgs.add(param);
    }
    return builder.emitSpecializeInst(
        builder.getTypeKind(),
        genericToSpecialize,
        (UInt)genArgs.getCount(),
        genArgs.getBuffer());
}

IRInst* maybeSpecializeWithGeneric(IRBuilder& builder, IRInst* genericToSpecailize, IRInst* userGeneric)
{
    if (auto gen = as<IRGeneric>(userGeneric))
    {
        if (auto toSpecialize = as<IRGeneric>(genericToSpecailize))
        {
            return specializeWithGeneric(builder, toSpecialize, gen);
        }
    }
    return genericToSpecailize;
}

IRInst* hoistValueFromGeneric(IRBuilder& inBuilder, IRInst* value, IRInst*& outSpecializedVal, bool replaceExistingValue)
{
    auto outerGeneric = as<IRGeneric>(findOuterGeneric(value));
    if (!outerGeneric) return value;
    IRBuilder builder = inBuilder;
    builder.setInsertBefore(outerGeneric);
    auto newGeneric = builder.emitGeneric();
    builder.setInsertInto(newGeneric);
    builder.emitBlock();
    IRInst* newResultVal = nullptr;

    // Clone insts in outerGeneric up until `value`.
    IRCloneEnv cloneEnv;
    for (auto inst : outerGeneric->getFirstBlock()->getChildren())
    {
        auto newInst = cloneInst(&cloneEnv, &builder, inst);
        if (inst == value)
        {
            builder.emitReturn(newInst);
            newResultVal = newInst;
            break;
        }
    }
    SLANG_RELEASE_ASSERT(newResultVal);
    if (newResultVal->getOp() == kIROp_Func)
    {
        IRBuilder subBuilder = builder;
        IRInst* subOutSpecialized = nullptr;
        auto genericFuncType = hoistValueFromGeneric(subBuilder, newResultVal->getFullType(), subOutSpecialized, false);
        newGeneric->setFullType((IRType*)genericFuncType);
    }
    else
    {
        newGeneric->setFullType(builder.getTypeKind());
    }
    if (replaceExistingValue)
    {
        builder.setInsertBefore(value);
        outSpecializedVal = specializeWithGeneric(builder, newGeneric, outerGeneric);
        value->replaceUsesWith(outSpecializedVal);
        value->removeAndDeallocate();
    }
    eliminateDeadCode(newGeneric);
    return newGeneric;
}

void moveInstChildren(IRInst* dest, IRInst* src)
{
    for (auto child = dest->getFirstDecorationOrChild(); child; )
    {
        auto next = child->getNextInst();
        child->removeAndDeallocate();
        child = next;
    }
    for (auto child = src->getFirstDecorationOrChild(); child; )
    {
        auto next = child->getNextInst();
        child->insertAtEnd(dest);
        child = next;
    }
}

String dumpIRToString(IRInst* root)
{
    StringBuilder sb;
    StringWriter writer(&sb, Slang::WriterFlag::AutoFlush);
    dumpIR(root, IRDumpOptions(), nullptr, &writer);
    return sb.ToString();
}

bool isPureFunctionalCall(IRCall* call)
{
    auto callee = getResolvedInstForDecorations(call->getCallee());
    if (callee->findDecoration<IRReadNoneDecoration>())
    {
        return true;
    }
    if (callee->findDecoration<IRNoSideEffectDecoration>())
    {
        // If the function has no side effect and is not writing to any outputs,
        // we can safely treat the call as a normal inst.
        bool hasOutArg = false;
        for (UInt i = 0; i < call->getArgCount(); i++)
        {
            if (as<IRPtrTypeBase>(call->getArg(i)->getDataType()))
            {
                hasOutArg = true;
                break;
            }
        }
        return !hasOutArg;
    }
    return false;
}

struct GenericChildrenMigrationContextImpl
{
    IRCloneEnv cloneEnv;
    IRGeneric* srcGeneric;
    IRGeneric* dstGeneric;
    DeduplicateContext deduplicateContext;

    void init(IRGeneric* genericSrc, IRGeneric* genericDst, IRInst* insertBefore)
    {
        srcGeneric = genericSrc;
        dstGeneric = genericDst;

        if (!genericSrc)
            return;
        auto srcParam = genericSrc->getFirstBlock()->getFirstParam();
        auto dstParam = genericDst->getFirstBlock()->getFirstParam();
        while (srcParam && dstParam)
        {
            cloneEnv.mapOldValToNew[srcParam] = dstParam;
            srcParam = srcParam->getNextParam();
            dstParam = dstParam->getNextParam();
        }
        cloneEnv.mapOldValToNew[genericSrc] = genericDst;
        cloneEnv.mapOldValToNew[genericSrc->getFirstBlock()] = genericDst->getFirstBlock();

        if (insertBefore)
        {
            for (auto inst = genericDst->getFirstBlock()->getFirstOrdinaryInst();
                inst && inst != insertBefore;
                inst = inst->getNextInst())
            {
                IRInstKey key = { inst };
                deduplicateContext.deduplicateMap.AddIfNotExists(key, inst);
            }
        }
    }

    IRInst* deduplicate(IRInst* value)
    {
        return deduplicateContext.deduplicate(value, [this](IRInst* inst)
            {
                if (inst->getParent() != dstGeneric->getFirstBlock())
                    return false;
                switch (inst->getOp())
                {
                case kIROp_Param:
                case kIROp_StructType:
                case kIROp_StructKey:
                case kIROp_InterfaceType:
                case kIROp_ClassType:
                case kIROp_Func:
                case kIROp_Generic:
                    return false;
                default:
                    break;
                }
                if (as<IRConstant>(inst))
                    return false;
                return true;
            });
    }

    IRInst* cloneInst(IRBuilder* builder, IRInst* src)
    {
        if (!srcGeneric)
            return src;
        if (findOuterGeneric(src) == srcGeneric)
        {
            auto cloned = Slang::cloneInst(&cloneEnv, builder, src);
            auto deduplicated = deduplicate(cloned);
            if (deduplicated != cloned)
                cloneEnv.mapOldValToNew[src] = deduplicated;
            return deduplicated;
        }
        return src;
    }
};

GenericChildrenMigrationContext::GenericChildrenMigrationContext()
{
    impl = new GenericChildrenMigrationContextImpl();
}

GenericChildrenMigrationContext::~GenericChildrenMigrationContext()
{
    delete impl;
}

IRCloneEnv* GenericChildrenMigrationContext::getCloneEnv()
{
    return &impl->cloneEnv;
}

void GenericChildrenMigrationContext::init(IRGeneric* genericSrc, IRGeneric* genericDst, IRInst* insertBefore)
{
    impl->init(genericSrc, genericDst, insertBefore);
}

IRInst* GenericChildrenMigrationContext::deduplicate(IRInst* value)
{
    return impl->deduplicate(value);
}

IRInst* GenericChildrenMigrationContext::cloneInst(IRBuilder* builder, IRInst* src)
{
    return impl->cloneInst(builder, src);
}

}
