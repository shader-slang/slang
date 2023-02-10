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
    IRDumpOptions options = {};
    options.flags = IRDumpOptions::Flag::DumpDebugIds;
    dumpIR(root, options, nullptr, &writer);
    return sb.ToString();
}

void copyNameHintDecoration(IRInst* dest, IRInst* src)
{
    auto decor = src->findDecoration<IRNameHintDecoration>();
    if (decor)
    {
        cloneDecoration(decor, dest);
    }
}

void getTypeNameHint(StringBuilder& sb, IRInst* type)
{
    if (!type)
        return;

    switch (type->getOp())
    {
    case kIROp_FloatType:
        sb << "float";
        break;
    case kIROp_HalfType:
        sb << "half";
        break;
    case kIROp_DoubleType:
        sb << "double";
        break;
    case kIROp_IntType:
        sb << "int";
        break;
    case kIROp_Int8Type:
        sb << "int8";
        break;
    case kIROp_Int16Type:
        sb << "int16";
        break;
    case kIROp_Int64Type:
        sb << "int64";
        break;
    case kIROp_IntPtrType:
        sb << "intptr";
        break;
    case kIROp_UIntType:
        sb << "uint";
        break;
    case kIROp_UInt8Type:
        sb << "uint8";
        break;
    case kIROp_UInt16Type:
        sb << "uint16";
        break;
    case kIROp_UInt64Type:
        sb << "uint64";
        break;
    case kIROp_UIntPtrType:
        sb << "uintptr";
        break;
    case kIROp_CharType:
        sb << "char";
        break;
    case kIROp_StringType:
        sb << "string";
        break;
    case kIROp_ArrayType:
        sb << "array_";
        getTypeNameHint(sb, type->getOperand(0));
        break;
    case kIROp_VectorType:
        getTypeNameHint(sb, type->getOperand(0));
        getTypeNameHint(sb, as<IRVectorType>(type)->getElementCount());
        break;
    case kIROp_MatrixType:
        getTypeNameHint(sb, type->getOperand(0));
        getTypeNameHint(sb, as<IRMatrixType>(type)->getRowCount());
        sb << "x";
        getTypeNameHint(sb, as<IRMatrixType>(type)->getColumnCount());
        break;
    case kIROp_IntLit:
        sb << as<IRIntLit>(type)->getValue();
        break;
    default:
        if (auto decor = type->findDecoration<IRNameHintDecoration>())
            sb << decor->getName();
        break;
    }
}

static IRInst* _getRootAddr(IRInst* addr)
{
    for (;;)
    {
        switch (addr->getOp())
        {
        case kIROp_GetElementPtr:
        case kIROp_FieldAddress:
            addr = addr->getOperand(0);
            continue;
        default:
            break;
        }
        break;
    }
    return addr;
}

// A simple and conservative address aliasing check.
bool canAddressesPotentiallyAlias(IRGlobalValueWithCode* func, IRInst* addr1, IRInst* addr2)
{
    if (addr1 == addr2)
        return true;

    // Two variables can never alias.
    addr1 = _getRootAddr(addr1);
    addr2 = _getRootAddr(addr2);

    // Global addresses can alias with anything.
    if (!isChildInstOf(addr1, func))
        return true;

    if (!isChildInstOf(addr2, func))
        return true;

    if (addr1->getOp() == kIROp_Var && addr2->getOp() == kIROp_Var
        && addr1 != addr2)
        return false;

    // A param and a var can never alias.
    if (addr1->getOp() == kIROp_Param && addr1->getParent() == func->getFirstBlock() &&
        addr2->getOp() == kIROp_Var ||
        addr1->getOp() == kIROp_Var && addr2->getOp() == kIROp_Param &&
        addr2->getParent() == func->getFirstBlock())
        return false;
    return true;
}

bool isPtrLikeOrHandleType(IRInst* type)
{
    switch (type->getOp())
    {
    case kIROp_ComPtrType:
    case kIROp_RawPointerType:
    case kIROp_RTTIPointerType:
    case kIROp_PseudoPtrType:
    case kIROp_OutType:
    case kIROp_InOutType:
    case kIROp_PtrType:
    case kIROp_RefType:
        return true;
    }
    return false;
}

bool canInstHaveSideEffectAtAddress(IRGlobalValueWithCode* func, IRInst* inst, IRInst* addr)
{
    switch (inst->getOp())
    {
    case kIROp_Store:
        // If the target of the store inst may overlap addr, return true.
        if (canAddressesPotentiallyAlias(func, as<IRStore>(inst)->getPtr(), addr))
            return true;
        break;
    case kIROp_Call:
        {
            auto call = as<IRCall>(inst);

            // If addr is a global variable, calling a function may change its value.
            // So we need to return true here to be conservative.
            if (!isChildInstOf(_getRootAddr(addr), func))
            {
                auto callee = call->getCallee();
                if (callee &&
                    callee->findDecoration<IRReadNoneDecoration>() &&
                    callee->findDecoration<IRNoSideEffectDecoration>())
                {
                    // An exception is if the callee is side-effect free and is not reading from
                    // memory.
                }
                else
                {
                    return true;
                }
            }

            // If any pointer typed argument of the call inst may overlap addr, return true.
            for (UInt i = 1; i < call->getArgCount(); i++)
            {
                if (isPtrLikeOrHandleType(call->getArg(i)->getDataType()))
                {
                    if (canAddressesPotentiallyAlias(func, call->getArg(i), addr))
                        return true;
                }
            }
        }
        break;
    case kIROp_CastPtrToInt:
    case kIROp_Reinterpret:
    case kIROp_BitCast:
        {
            // If we are trying to cast an address to something else, return true.
            if (isPtrLikeOrHandleType(inst->getOperand(0)->getDataType()) &&
                canAddressesPotentiallyAlias(func, inst->getOperand(0), addr))
                return true;
        }
        break;
    default:
        // Default behavior is that any insts that have side effect may affect `addr`.
        if (inst->mightHaveSideEffects())
            return true;
        break;
    }
    return false;
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
