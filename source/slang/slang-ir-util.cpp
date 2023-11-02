#include "slang-ir-util.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-dominators.h"

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

// Returns true if is not possible to produce side-effect from a value of `dataType`.
bool isValueType(IRInst* dataType)
{
    dataType = getResolvedInstForDecorations(unwrapAttributedType(dataType));
    if (as<IRBasicType>(dataType))
        return true;
    switch (dataType->getOp())
    {
    case kIROp_StructType:
    case kIROp_InterfaceType:
    case kIROp_ClassType:
    case kIROp_VectorType:
    case kIROp_MatrixType:
    case kIROp_TupleType:
    case kIROp_ResultType:
    case kIROp_OptionalType:
    case kIROp_DifferentialPairType:
    case kIROp_DifferentialPairUserCodeType:
    case kIROp_DynamicType:
    case kIROp_AnyValueType:
    case kIROp_ArrayType:
    case kIROp_FuncType:
    case kIROp_RaytracingAccelerationStructureType:
        return true;
    default:
        // Read-only resource handles are considered as Value type.
        if (auto resType = as<IRResourceTypeBase>(dataType))
            return (resType->getAccess() == SLANG_RESOURCE_ACCESS_READ);
        else if (as<IRSamplerStateTypeBase>(dataType))
            return true;
        else if (as<IRHLSLByteAddressBufferType>(dataType))
            return true;
        else if (as<IRHLSLStructuredBufferType>(dataType))
            return true;
        return false;
    }
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

String dumpIRToString(IRInst* root, IRDumpOptions options)
{
    StringBuilder sb;
    StringWriter writer(&sb, Slang::WriterFlag::AutoFlush);
    dumpIR(root, options, nullptr, &writer);
    return sb.toString();
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

IRInst* getRootAddr(IRInst* addr)
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
    addr1 = getRootAddr(addr1);
    addr2 = getRootAddr(addr2);

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
    if (!type)
        return false;
    if (as<IRPointerLikeType>(type))
        return true;
    if (as<IRPseudoPtrType>(type))
        return true;
    if (as<IRHLSLStructuredBufferTypeBase>(type))
        return true;
    switch (type->getOp())
    {
    case kIROp_ComPtrType:
    case kIROp_RawPointerType:
    case kIROp_RTTIPointerType:
    case kIROp_OutType:
    case kIROp_InOutType:
    case kIROp_PtrType:
    case kIROp_RefType:
    case kIROp_ConstRefType:
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
    case kIROp_SwizzledStore:
        // If the target of the swizzled store inst may overlap addr, return true.
        if (canAddressesPotentiallyAlias(func, as<IRSwizzledStore>(inst)->getDest(), addr))
            return true;
        break;
    case kIROp_Call:
        {
            auto call = as<IRCall>(inst);

            // If addr is a global variable, calling a function may change its value.
            // So we need to return true here to be conservative.
            if (!isChildInstOf(getRootAddr(addr), func))
            {
                auto callee = call->getCallee();
                if (callee &&
                    !doesCalleeHaveSideEffect(callee))
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
            for (UInt i = 0; i < call->getArgCount(); i++)
            {
                SLANG_RELEASE_ASSERT(call->getArg(i)->getDataType());
                if (isPtrLikeOrHandleType(call->getArg(i)->getDataType()))
                {
                    if (canAddressesPotentiallyAlias(func, call->getArg(i), addr))
                        return true;
                }
                else if (!isValueType(call->getArg(i)->getDataType()))
                {
                    // This is some unknown handle type, we assume it can have any side effects.
                    return true;
                }
            }
        }
        break;
    case kIROp_unconditionalBranch:
    case kIROp_loop:
        {
            auto branch = as<IRUnconditionalBranch>(inst);
            // If any pointer typed argument of the branch inst may overlap addr, return true.
            for (UInt i = 0; i < branch->getArgCount(); i++)
            {
                SLANG_RELEASE_ASSERT(branch->getArg(i)->getDataType());
                if (isPtrLikeOrHandleType(branch->getArg(i)->getDataType()))
                {
                    if (canAddressesPotentiallyAlias(func, branch->getArg(i), addr))
                        return true;
                }
                else if (!isValueType(branch->getArg(i)->getDataType()))
                {
                    // This is some unknown handle type, we assume it can have any side effects.
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
            else if (!isValueType(inst->getOperand(0)->getDataType()))
            {
                // This is some unknown handle type, we assume it can have any side effects.
                return true;
            }
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

IRInst* getUndefInst(IRBuilder builder, IRModule* module)
{
    IRInst* undefInst = nullptr;

    for (auto inst : module->getModuleInst()->getChildren())
    {
        if (inst->getOp() == kIROp_undefined && inst->getDataType() && inst->getDataType()->getOp() == kIROp_VoidType)
        {
            undefInst = inst;
            break;
        }
    }
    if (!undefInst)
    {
        auto voidType = builder.getVoidType();
        builder.setInsertAfter(voidType);
        undefInst = builder.emitUndefined(voidType);
    }
    return undefInst;
}

IROp getSwapSideComparisonOp(IROp op)
{
    switch (op)
    {
    case kIROp_Eql:
        return kIROp_Eql;
    case kIROp_Neq:
        return kIROp_Neq;
    case kIROp_Leq:
        return kIROp_Geq;
    case kIROp_Geq:
        return kIROp_Leq;
    case kIROp_Less:
        return kIROp_Greater;
    case kIROp_Greater:
        return kIROp_Less;
    default:
        return kIROp_Nop;
    }
}

IRInst* emitLoopBlocks(IRBuilder* builder, IRInst* initVal, IRInst* finalVal, IRBlock*& loopBodyBlock, IRBlock*& loopBreakBlock)
{
    IRBuilder loopBuilder = *builder;
    auto loopHeadBlock = loopBuilder.emitBlock();
    loopBodyBlock = loopBuilder.emitBlock();
    loopBreakBlock = loopBuilder.emitBlock();
    auto loopContinueBlock = loopBuilder.emitBlock();
    builder->emitLoop(loopHeadBlock, loopBreakBlock, loopHeadBlock, 1, &initVal);
    loopBuilder.setInsertInto(loopHeadBlock);
    auto loopParam = loopBuilder.emitParam(initVal->getFullType());
    auto cmpResult = loopBuilder.emitLess(loopParam, finalVal);
    loopBuilder.emitIfElse(cmpResult, loopBodyBlock, loopBreakBlock, loopBreakBlock);
    loopBuilder.setInsertInto(loopBodyBlock);
    loopBuilder.emitBranch(loopContinueBlock);
    loopBuilder.setInsertInto(loopContinueBlock);
    auto newParam = loopBuilder.emitAdd(loopParam->getFullType(), loopParam, loopBuilder.getIntValue(loopBuilder.getIntType(), 1));
    loopBuilder.emitBranch(loopHeadBlock, 1, &newParam);
    return loopParam;
}

void sortBlocksInFunc(IRGlobalValueWithCode* func)
{
    auto order = getReversePostorder(func);
    for (auto block : order)
        block->insertAtEnd(func);
}

void removeLinkageDecorations(IRGlobalValueWithCode* func)
{
    List<IRInst*> toRemove;
    for (auto inst : func->getDecorations())
    {
        switch (inst->getOp())
        {
        case kIROp_ImportDecoration:
        case kIROp_ExportDecoration:
        case kIROp_ExternCppDecoration:
        case kIROp_PublicDecoration:
        case kIROp_KeepAliveDecoration:
        case kIROp_DllImportDecoration:
        case kIROp_CudaDeviceExportDecoration:
        case kIROp_DllExportDecoration:
        case kIROp_HLSLExportDecoration:
            toRemove.add(inst);
            break;
        default:
            break;
        }
    }
    for (auto inst : toRemove)
        inst->removeAndDeallocate();
}

void setInsertBeforeOrdinaryInst(IRBuilder* builder, IRInst* inst)
{
    if (as<IRParam, IRDynamicCastBehavior::NoUnwrap>(inst))
    {
        SLANG_RELEASE_ASSERT(as<IRBlock>(inst->getParent()));
        auto lastParam = as<IRBlock>(inst->getParent())->getLastParam();
        builder->setInsertAfter(lastParam);
    }
    else
    {
        builder->setInsertBefore(inst);
    }
}

void setInsertAfterOrdinaryInst(IRBuilder* builder, IRInst* inst)
{
    if (as<IRParam, IRDynamicCastBehavior::NoUnwrap>(inst))
    {
        SLANG_RELEASE_ASSERT(as<IRBlock>(inst->getParent()));
        auto lastParam = as<IRBlock>(inst->getParent())->getLastParam();
        builder->setInsertAfter(lastParam);
    }
    else
    {
        builder->setInsertAfter(inst);
    }
}

bool areCallArgumentsSideEffectFree(IRCall* call, SideEffectAnalysisOptions options)
{
    // If the function has no side effect and is not writing to any outputs,
    // we can safely treat the call as a normal inst.
    IRFunc* parentFunc = nullptr;
    for (UInt i = 0; i < call->getArgCount(); i++)
    {
        auto arg = call->getArg(i);
        if (isValueType(arg->getDataType()))
            continue;

        // If the argument type is not a known value type,
        // assume it is a pointer or handle through which side effect can take place.
        if (!parentFunc)
        {
            parentFunc = getParentFunc(call);
            if (!parentFunc)
                return false;
        }

        auto module = parentFunc->getModule();
        if (!module)
            return false;

        if (arg->getOp() == kIROp_Var && getParentFunc(arg) == parentFunc)
        {
            IRDominatorTree* dom = nullptr;
            if (isBitSet(options, SideEffectAnalysisOptions::UseDominanceTree))
                dom = module->findOrCreateDominatorTree(parentFunc);

            // If the pointer argument is a local variable (thus can't alias with other addresses)
            // and it is never read from in the function, we can safely treat the call as having
            // no side-effect.
            // This is a conservative test, but is sufficient to detect the most common case where
            // a temporary variable is used as the inout argument and the result stored in the temp
            // variable isn't being used elsewhere in the parent func.
            // 
            // A more aggresive test can check all other address uses reachable from the call site
            // and see if any of them are aliasing with the argument.
            for (auto use = arg->firstUse; use; use = use->nextUse)
            {
                if (as<IRDecoration>(use->getUser()))
                    continue;
                switch (use->getUser()->getOp())
                {
                case kIROp_Store:
                case kIROp_SwizzledStore:
                    // We are fine with stores into the variable, since store operations
                    // are not dependent on whatever we do in the call here.
                    continue;
                default:
                    // Skip the call itself if the var is used as an argument to an out parameter
                    // since we are checking if the call has side effect.
                    // We can't treat the call as side effect free if var is used as an inout parameter,
                    // because if the call is inside a loop there will be a visible side effect after
                    // the call.
                    if (use->getUser() == call)
                    {
                        auto funcType = as<IRFuncType>(call->getCallee()->getDataType());
                        if (!funcType)
                            return false;
                        if (funcType->getParamCount() > i && as<IROutType>(funcType->getParamType(i)))
                            continue;

                        // We are an argument to an inout parameter.
                        // We can only treat the call as side effect free if the call is not inside a loop.
                        // 
                        // If we don't have the loop information here, we will conservatively return false.
                        //
                        if (!dom)
                            return false;

                        // If we have dominator tree available, use it to check if the call is inside a loop.
                        auto callBlock = as<IRBlock>(call->getParent());
                        if (!callBlock) return false;
                        auto varBlock = as<IRBlock>(arg->getParent());
                        if (!varBlock) return false;
                        auto idom = callBlock;
                        while (idom != varBlock)
                        {
                            idom = dom->getImmediateDominator(idom);
                            if (!idom)
                                return false; // If we are here, var does not dominate the call, which should never happen.
                            if (auto loop = as<IRLoop>(idom->getTerminator()))
                            {
                                if (!dom->dominates(loop->getBreakBlock(), callBlock))
                                    return false; // The var is used in a loop, must return false.
                            }
                        }
                        // If we reach here, the var is used as an inout parameter for the call, but the call
                        // is not nested in a loop at an higher nesting level than where the var is defined,
                        // so we can treat the use as DCE-able.
                        continue;
                    }
                    // We have some other unknown use of the variable address, they can
                    // be loads, or calls using addresses derived from the variable,
                    // we will treat the call as having side effect to be safe.
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool isPureFunctionalCall(IRCall* call, SideEffectAnalysisOptions options)
{
    auto callee = getResolvedInstForDecorations(call->getCallee());
    if (callee->findDecoration<IRReadNoneDecoration>())
    {
        return areCallArgumentsSideEffectFree(call, options);
    }
    return false;
}

bool isSideEffectFreeFunctionalCall(IRCall* call, SideEffectAnalysisOptions options)
{
    // If the call has been marked as no-side-effect, we
    // will treat it so, by-passing all other checks.
    if (call->findDecoration<IRNoSideEffectDecoration>())
        return false;

    if (!doesCalleeHaveSideEffect(call->getCallee()))
    {
        return areCallArgumentsSideEffectFree(call, options);
    }
    return false;
}

bool doesCalleeHaveSideEffect(IRInst* callee)
{
    for (auto decor : getResolvedInstForDecorations(callee)->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_NoSideEffectDecoration:
        case kIROp_ReadNoneDecoration:
            return false;
        }
    }
    return true;
}

IRInst* findInterfaceRequirement(IRInterfaceType* type, IRInst* key)
{
    for (UInt i = 0; i < type->getOperandCount(); i++)
    {
        if (auto req = as<IRInterfaceRequirementEntry>(type->getOperand(i)))
        {
            if (req->getRequirementKey() == key)
                return req->getRequirementVal();
        }
    }
    return nullptr;
}

IRInst* findWitnessTableEntry(IRWitnessTable* table, IRInst* key)
{
    for (auto entry : table->getEntries())
    {
        if (entry->getRequirementKey() == key)
            return entry->getSatisfyingVal();
    }
    return nullptr;
}

IRInst* getVulkanPayloadLocation(IRInst* payloadGlobalVar)
{
    IRInst* location = nullptr;
    for (auto decor : payloadGlobalVar->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_VulkanRayPayloadDecoration:
        case kIROp_VulkanCallablePayloadDecoration:
        case kIROp_VulkanHitObjectAttributesDecoration:
            return decor->getOperand(0);
        default:
            continue;
        }
    }
    return location;
}

void moveParams(IRBlock* dest, IRBlock* src)
{
    for (auto param = src->getFirstChild(); param;)
    {
        auto nextInst = param->getNextInst();
        if (as<IRDecoration>(param) || as<IRParam, IRDynamicCastBehavior::NoUnwrap>(param))
        {
            param->insertAtEnd(dest);
        }
        else
        {
            break;
        }
        param = nextInst;
    }
}

void removePhiArgs(IRInst* phiParam)
{
    auto block = cast<IRBlock>(phiParam->getParent());
    UInt paramIndex = 0;
    for (auto p = block->getFirstParam(); p; p = p->getNextParam())
    {
        if (p == phiParam)
            break;
        paramIndex++;
    }
    for (auto predBlock : block->getPredecessors())
    {
        auto termInst = as<IRUnconditionalBranch>(predBlock->getTerminator());
        SLANG_ASSERT(paramIndex < termInst->getArgCount());
        termInst->removeArgument(paramIndex);
    }
}

int getParamIndexInBlock(IRParam* paramInst)
{
    auto block = as<IRBlock>(paramInst->getParent());
    if (!block)
        return -1;
    int paramIndex = 0;
    for (auto param : block->getParams())
    {
        if (param == paramInst)
            return paramIndex;
        paramIndex++;
    }
    return -1;
}

bool isGlobalOrUnknownMutableAddress(IRGlobalValueWithCode* parentFunc, IRInst* inst)
{
    auto root = getRootAddr(inst);

    auto type = unwrapAttributedType(inst->getDataType());
    if (!isPtrLikeOrHandleType(type))
        return false;

    if (root)
    {
        // If this is a global readonly resource, it is not a mutable address.
        if (as<IRParameterGroupType>(root->getDataType()))
        {
            return false;
        }
        if (as<IRHLSLStructuredBufferType>(root->getDataType()))
        {
            return false;
        }
    }

    switch (root->getOp())
    {
    case kIROp_GlobalVar:
    case kIROp_GlobalParam:
    case kIROp_GlobalConstant:
    case kIROp_Var:
    case kIROp_Param:
        break;
    case kIROp_Call:
        return true;
    default:
        return true;
    }

    auto addrInstParent = getParentFunc(root);
    return (addrInstParent != parentFunc);
}

bool isZero(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_IntLit:
        return as<IRIntLit>(inst)->getValue() == 0;
    case kIROp_FloatLit:
        return as<IRFloatLit>(inst)->getValue() == 0.0;
    case kIROp_BoolLit:
        return as<IRBoolLit>(inst)->getValue() == false;
    case kIROp_MakeVector:
    case kIROp_MakeVectorFromScalar:
    case kIROp_MakeMatrix:
    case kIROp_MakeMatrixFromScalar:
    case kIROp_MatrixReshape:
    case kIROp_VectorReshape:
    {
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            if (!isZero(inst->getOperand(i)))
            {
                return false;
            }
        }
        return true;
    }
    case kIROp_CastIntToFloat:
    case kIROp_CastFloatToInt:
        return isZero(inst->getOperand(0));
    default:
        return false;
    }
}

bool isOne(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_IntLit:
        return as<IRIntLit>(inst)->getValue() == 1;
    case kIROp_FloatLit:
        return as<IRFloatLit>(inst)->getValue() == 1.0;
    case kIROp_BoolLit:
        return as<IRBoolLit>(inst)->getValue();
    case kIROp_MakeVector:
    case kIROp_MakeVectorFromScalar:
    case kIROp_MakeMatrix:
    case kIROp_MakeMatrixFromScalar:
    case kIROp_MatrixReshape:
    case kIROp_VectorReshape:
    {
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            if (!isOne(inst->getOperand(i)))
            {
                return false;
            }
        }
        return true;
    }
    case kIROp_CastIntToFloat:
    case kIROp_CastFloatToInt:
        return isOne(inst->getOperand(0));
    default:
        return false;
    }
}

IRPtrTypeBase* isMutablePointerType(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_ConstRefType:
        return nullptr;
    default:
        return as<IRPtrTypeBase>(inst);
    }
}

void initializeScratchData(IRInst* inst)
{
    List<IRInst*> workList;
    workList.add(inst);
    while (workList.getCount() != 0)
    {
        auto item = workList.getLast();
        workList.removeLast();
        item->scratchData = 0;
        for (auto child = item->getLastDecorationOrChild(); child; child = child->getPrevInst())
            workList.add(child);
    }   
}

void resetScratchDataBit(IRInst* inst, int bitIndex)
{
    List<IRInst*> workList;
    workList.add(inst);
    while (workList.getCount() != 0)
    {
        auto item = workList.getLast();
        workList.removeLast();
        item->scratchData &= ~(1ULL << bitIndex);
        for (auto child = item->getLastDecorationOrChild(); child; child = child->getPrevInst())
            workList.add(child);
    }
}

List<IRBlock*> collectBlocksInRegion(
    IRDominatorTree* dom,
    IRLoop* loop,
    bool* outHasMultiLevelBreaks)
{
    return collectBlocksInRegion(dom, loop->getBreakBlock(), loop->getTargetBlock(), true, outHasMultiLevelBreaks);
}

List<IRBlock*> collectBlocksInRegion(
    IRDominatorTree* dom,
    IRLoop* loop)
{
    bool hasMultiLevelBreaks = false;
    return collectBlocksInRegion(dom, loop->getBreakBlock(), loop->getTargetBlock(), true, &hasMultiLevelBreaks);
}

List<IRBlock*> collectBlocksInRegion(
    IRDominatorTree* dom,
    IRSwitch* switchInst,
    bool* outHasMultiLevelBreaks)
{
    return collectBlocksInRegion(dom, switchInst->getBreakLabel(), as<IRBlock>(switchInst->getParent()), false, outHasMultiLevelBreaks);
}

List<IRBlock*> collectBlocksInRegion(
    IRDominatorTree* dom,
    IRSwitch* switchInst)
{
    bool hasMultiLevelBreaks = false;
    return collectBlocksInRegion(dom, switchInst->getBreakLabel(), as<IRBlock>(switchInst->getParent()), false, &hasMultiLevelBreaks);
}

HashSet<IRBlock*> getParentBreakBlockSet(IRDominatorTree* dom, IRBlock* block)
{
    HashSet<IRBlock*> parentBreakBlocksSet;
    for (IRBlock* currBlock = dom->getImmediateDominator(block); 
        currBlock;
        currBlock = dom->getImmediateDominator(currBlock))
    {
        if (auto loopInst = as<IRLoop>(currBlock->getTerminator()))
            if (!dom->dominates(loopInst->getBreakBlock(), block))
                parentBreakBlocksSet.add(loopInst->getBreakBlock());
        else if (auto switchInst = as<IRSwitch>(currBlock->getTerminator()))
            if (!dom->dominates(switchInst->getBreakLabel(), block))
                parentBreakBlocksSet.add(switchInst->getBreakLabel());
    }

    return parentBreakBlocksSet;
}

List<IRBlock*> collectBlocksInRegion(
    IRDominatorTree* dom,
    IRBlock* breakBlock,
    IRBlock* firstBlock,
    bool includeFirstBlock,
    bool* outHasMultiLevelBreaks)
{
    List<IRBlock*> regionBlocks;
    HashSet<IRBlock*> regionBlocksSet;
    auto addBlock = [&](IRBlock* block)
    {
        if (regionBlocksSet.add(block))
            regionBlocks.add(block);
    };

    // Use dominator tree heirarchy to find break blocks of
    // all parent regions. We'll need to this to detect breaks 
    // to outer regions (particularly when our region has no reachable 
    // break block of its own)
    // 
    HashSet<IRBlock*> parentBreakBlocksSet = getParentBreakBlockSet(dom, firstBlock);

    *outHasMultiLevelBreaks = false;

    addBlock(firstBlock);
    for (Index i = 0; i < regionBlocks.getCount(); i++)
    {
        auto block = regionBlocks[i];
        for (auto succ : block->getSuccessors())
        {
            if (parentBreakBlocksSet.contains(succ) && succ != breakBlock)
            {
                *outHasMultiLevelBreaks = true;
                continue;
            }

            if (succ == breakBlock)
                continue;
            if (!dom->dominates(firstBlock, succ))
                continue;
            if (!as<IRUnreachable>(breakBlock->getTerminator()))
            {
                if (dom->dominates(breakBlock, succ))
                    continue;
            }

            addBlock(succ);
        }
    }

    if (!includeFirstBlock)
    {
        regionBlocksSet.remove(firstBlock);
        regionBlocks.remove(firstBlock);
    }

    return regionBlocks;
}

List<IRBlock *> collectBlocksInRegion(IRGlobalValueWithCode *func, IRLoop *loopInst, bool* outHasMultiLevelBreaks)
{
    auto dom = computeDominatorTree(func);
    return collectBlocksInRegion(dom, loopInst, outHasMultiLevelBreaks);
}

List<IRBlock*> collectBlocksInRegion(IRGlobalValueWithCode* func, IRLoop* loopInst)
{
    auto dom = computeDominatorTree(func);
    bool hasMultiLevelBreaks = false;
    return collectBlocksInRegion(dom, loopInst, &hasMultiLevelBreaks);
}

IRVarLayout* findVarLayout(IRInst* value)
{
    if (auto layoutDecoration = value->findDecoration<IRLayoutDecoration>())
        return as<IRVarLayout>(layoutDecoration->getLayout());
    return nullptr;

}

UnownedStringSlice getBuiltinFuncName(IRInst* callee)
{
    auto decor = getResolvedInstForDecorations(callee)->findDecoration<IRKnownBuiltinDecoration>();
    if (!decor)
        return UnownedStringSlice();
    return decor->getName();
}

void hoistInstOutOfASMBlocks(IRBlock* block)
{
    for (auto inst : block->getChildren())
    {
        if (auto asmBlock = as<IRSPIRVAsm>(inst))
        {
            IRInst* next = nullptr;
            for (auto i = asmBlock->getFirstChild(); i; i = next)
            {
                next = i->getNextInst();
                if (!as<IRSPIRVAsmInst>(i) && !as<IRSPIRVAsmOperand>(i))
                    i->insertBefore(asmBlock);
            }
        }
    }
}

IRParam* getParamAt(IRBlock* block, UIndex ii)
{
    UIndex index = 0;
    for (auto param : block->getParams())
    {
        if (ii == index)
            return param;

        index++;
    }
    SLANG_UNEXPECTED("ii >= paramCount");
}

UnownedStringSlice getBasicTypeNameHint(IRType* basicType)
{
    switch (basicType->getOp())
    {
        case kIROp_IntType:
            return UnownedStringSlice::fromLiteral("int");
        case kIROp_Int8Type:
            return UnownedStringSlice::fromLiteral("int8");
        case kIROp_Int16Type:
            return UnownedStringSlice::fromLiteral("int16");
        case kIROp_Int64Type:
            return UnownedStringSlice::fromLiteral("int64");
        case kIROp_IntPtrType:
            return UnownedStringSlice::fromLiteral("intptr");
        case kIROp_UIntType:
            return UnownedStringSlice::fromLiteral("uint");
        case kIROp_UInt8Type:
            return UnownedStringSlice::fromLiteral("uint8");
        case kIROp_UInt16Type:
            return UnownedStringSlice::fromLiteral("uint16");
        case kIROp_UInt64Type:
            return UnownedStringSlice::fromLiteral("uint64");
        case kIROp_UIntPtrType:
            return UnownedStringSlice::fromLiteral("uintptr");
        case kIROp_FloatType:
            return UnownedStringSlice::fromLiteral("float");
        case kIROp_HalfType:
            return UnownedStringSlice::fromLiteral("half");
        case kIROp_DoubleType:
            return UnownedStringSlice::fromLiteral("double");
        case kIROp_BoolType:
            return UnownedStringSlice::fromLiteral("bool");
        case kIROp_VoidType:
            return UnownedStringSlice::fromLiteral("void");
        case kIROp_CharType:
            return UnownedStringSlice::fromLiteral("char");
        default:
            return UnownedStringSlice();
    }
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
                deduplicateContext.deduplicateMap.addIfNotExists(key, inst);
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
                if (getIROpInfo(inst->getOp()).isHoistable())
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

IRType* dropNormAttributes(IRType* const t)
{
    if(const auto a = as<IRAttributedType>(t))
    {
        switch(a->getAttr()->getOp())
        {
            case kIROp_UNormAttr:
            case kIROp_SNormAttr:
                return dropNormAttributes(a->getBaseType());
        }
    }
    return t;
}

}
