#include "slang-ir-insert-debug-value-store.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
bool DebugValueStoreContext::isTypeKind(IRInst* inst)
{
    if (!inst)
        return true;
    switch (inst->getOp())
    {
    case kIROp_TypeKind:
    case kIROp_TypeType:
        return true;
    default:
        return false;
    }
}
bool DebugValueStoreContext::isDebuggableType(IRType* type)
{
    if (bool* result = m_mapTypeToDebugability.tryGetValue(type))
        return *result;

    bool debuggable = false;
    switch (type->getOp())
    {
    case kIROp_VoidType:
        break;
    case kIROp_StructType:
        {
            auto structType = static_cast<IRStructType*>(type);
            bool structDebuggable = true;
            for (auto field : structType->getFields())
            {
                if (!isDebuggableType(field->getFieldType()))
                {
                    structDebuggable = false;
                    break;
                }
            }
            debuggable = structDebuggable;
            break;
        }
    case kIROp_ArrayType:
    case kIROp_UnsizedArrayType:
        {
            auto arrayType = static_cast<IRArrayTypeBase*>(type);
            debuggable = isDebuggableType(arrayType->getElementType());
            break;
        }
    case kIROp_HLSLInputPatchType:
    case kIROp_HLSLOutputPatchType:
    case kIROp_HLSLTriangleStreamType:
        {
            auto elementType = as<IRType>(type->getOperand(0));
            debuggable = isDebuggableType(elementType);
            break;
        }
    case kIROp_VectorType:
    case kIROp_MatrixType:
    case kIROp_PtrType:
        debuggable = true;
        break;
    case kIROp_Param:
        // Assume generic parameters are debuggable.
        debuggable = true;
        break;
    case kIROp_Specialize:
        {
            auto specType = as<IRSpecialize>(type);
            auto specTypeDebuggable =
                isDebuggableType((IRType*)getResolvedInstForDecorations(specType));
            if (!specTypeDebuggable)
                break;
            for (UInt i = 0; i < specType->getArgCount(); i++)
            {
                auto arg = specType->getArg(i);
                if (isTypeKind(arg->getDataType()) &&
                    !isDebuggableType((IRType*)specType->getArg(i)))
                {
                    specTypeDebuggable = false;
                    break;
                }
            }
            debuggable = false; // specTypeDebuggable;
            break;
        }
    default:
        if (as<IRBasicType>(type))
            debuggable = true;
        break;
    }
    m_mapTypeToDebugability[type] = debuggable;
    return debuggable;
}

void DebugValueStoreContext::insertDebugValueStore(IRFunc* func)
{
    IRBuilder builder(func);
    Dictionary<IRInst*, IRInst*> mapVarToDebugVar;
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return;
    auto funcDebugLoc = func->findDecoration<IRDebugLocationDecoration>();
    if (!funcDebugLoc)
        return;

    // Get the IRDebugFunction for this function to use as the scope for parameters
    IRInst* funcDebugScope = nullptr;
    if (auto debugFuncDecor = func->findDecoration<IRDebugFuncDecoration>())
    {
        // IRDebugFuncDecoration stores the IRDebugFunction as its first operand
        if (debugFuncDecor->getOperandCount() > 0)
            funcDebugScope = debugFuncDecor->getOperand(0);
    }

    List<IRInst*> params;
    for (auto param : firstBlock->getParams())
    {
        params.add(param);
    }
    Index paramIndex = 0;
    for (auto param : params)
    {
        builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
        auto paramType = param->getDataType();
        bool isRefParam = false;
        if (auto outType = as<IROutParamTypeBase>(paramType))
        {
            isRefParam = true;
            paramType = outType->getValueType();
        }
        else if (auto ptrType = as<IRBorrowInParamType>(param->getDataType()))
        {
            isRefParam = true;
            paramType = ptrType->getValueType();
        }
        if (!isDebuggableType(paramType))
            continue;

        // Use the function's debug scope as the parent for parameter debug vars
        // If we don't have one, use the paramIndex as a fallback (will be nullptr)
        IRInst* paramScope = funcDebugScope ? funcDebugScope : builder.getIntValue(builder.getUIntType(), paramIndex);

        auto debugVar = builder.emitDebugVar(
            paramType,
            funcDebugLoc->getSource(),
            funcDebugLoc->getLine(),
            funcDebugLoc->getCol(),
            paramScope);
        copyNameHintAndDebugDecorations(debugVar, param);

        mapVarToDebugVar[param] = debugVar;

        // Map any in-param proxy vars to the debug var.
        bool hasProxyVar = false;
        for (auto use = param->firstUse; use; use = use->nextUse)
        {
            if (auto inParamProxyVarDecor = as<IRInParamProxyVarDecoration>(use->getUser()))
            {
                mapVarToDebugVar[inParamProxyVarDecor->parent] = debugVar;
                hasProxyVar = true;
            }
        }

        // Store the initial value of the parameter into the debug var.
        IRInst* paramVal = nullptr;
        if (!isRefParam)
        {
            paramVal = param;
        }
        else if (
            as<IRBorrowInOutParamType>(param->getDataType()) ||
            as<IRBorrowInParamType>(param->getDataType()))
        {
            paramVal = builder.emitLoad(param);
        }

        if (paramVal && !hasProxyVar)
        {
            builder.emitDebugValue(debugVar, paramVal);
        }
        paramIndex++;
    }

    for (auto block : func->getBlocks())
    {
        IRInst* nextInst = nullptr;
        for (auto inst = block->getFirstInst(); inst; inst = nextInst)
        {
            nextInst = inst->getNextInst();
            if (auto varInst = as<IRVar>(inst))
            {
                // First, check if there's already a debug var for this variable
                // (created during IR lowering with proper scope)
                IRDebugVar* existingDebugVar = nullptr;
                for (auto use = varInst->firstUse; use; use = use->nextUse)
                {
                    if (auto candidate = as<IRDebugVar>(use->getUser()->getParent()))
                    {
                        // Check if this is a debug var that follows this variable
                        if (candidate->getDataType() &&
                            as<IRPtrType>(candidate->getDataType()) &&
                            candidate->getOperand(0) && // has source
                            !existingDebugVar)
                        {
                            existingDebugVar = candidate;
                            break;
                        }
                    }
                }

                // Look forward in the same block for a debug var that was created for this variable
                if (!existingDebugVar)
                {
                    for (auto searchInst = varInst->getNextInst(); searchInst; searchInst = searchInst->getNextInst())
                    {
                        if (auto candidateDebugVar = as<IRDebugVar>(searchInst))
                        {
                            // Check if it matches the location
                            if (auto debugLoc = varInst->findDecoration<IRDebugLocationDecoration>())
                            {
                                if (candidateDebugVar->getSource() == debugLoc->getSource() &&
                                    candidateDebugVar->getLine() == debugLoc->getLine() &&
                                    candidateDebugVar->getCol() == debugLoc->getCol())
                                {
                                    auto varType = tryGetPointedToType(&builder, varInst->getDataType());
                                    auto debugVarType = tryGetPointedToType(&builder, candidateDebugVar->getDataType());
                                    if (varType == debugVarType)
                                    {
                                        existingDebugVar = candidateDebugVar;
                                        break;
                                    }
                                }
                            }
                        }
                        // Stop searching after we hit another var or significant instruction
                        if (as<IRVar>(searchInst) || as<IRStore>(searchInst))
                            break;
                    }
                }

                if (existingDebugVar)
                {
                    mapVarToDebugVar[varInst] = existingDebugVar;
                }
                else if (auto debugLoc = varInst->findDecoration<IRDebugLocationDecoration>())
                {
                    auto varType = tryGetPointedToType(&builder, varInst->getDataType());
                    builder.setInsertBefore(varInst);
                    if (!isDebuggableType(varType))
                        continue;
                    auto debugVar = builder.emitDebugVar(
                        varType,
                        debugLoc->getSource(),
                        debugLoc->getLine(),
                        debugLoc->getCol(),
                        nullptr);  // scope will be determined during SPIRV emission via findDebugScope
                    copyNameHintAndDebugDecorations(debugVar, varInst);
                    mapVarToDebugVar[varInst] = debugVar;
                }
            }
        }
    }

    // Collect all stores and insert debug value insts to update debug vars.

    // Helper func to insert debugValue updates.
    auto setDebugValue = [&](IRInst* debugVar, IRInst* newValue, ArrayView<IRInst*> accessChain)
    {
        auto ptr = builder.emitElementAddress(debugVar, accessChain);
        builder.emitDebugValue(ptr, newValue);
    };
    for (auto block : func->getBlocks())
    {
        IRInst* nextInst = nullptr;
        for (auto inst = block->getFirstInst(); inst; inst = nextInst)
        {
            nextInst = inst->getNextInst();

            if (auto storeInst = as<IRStore>(inst))
            {
                List<IRInst*> accessChain;
                auto varInst = getRootAddr(storeInst->getPtr(), accessChain);
                IRInst* debugVar = nullptr;
                if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                {
                    builder.setInsertAfter(storeInst);
                    setDebugValue(debugVar, storeInst->getVal(), accessChain.getArrayView());
                }
            }
            else if (auto swizzledStore = as<IRSwizzledStore>(inst))
            {
                List<IRInst*> accessChain;
                auto varInst = getRootAddr(swizzledStore->getDest(), accessChain);
                IRInst* debugVar = nullptr;
                if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                {
                    builder.setInsertAfter(swizzledStore);
                    auto loadVal = builder.emitLoad(swizzledStore->getDest());
                    setDebugValue(debugVar, loadVal, accessChain.getArrayView());
                }
            }
            else if (auto callInst = as<IRCall>(inst))
            {
                auto funcValue = getResolvedInstForDecorations(callInst->getCallee());
                if (!funcValue)
                    continue;
                for (UInt i = 0; i < callInst->getArgCount(); i++)
                {
                    auto arg = callInst->getArg(i);
                    if (!as<IRPtrTypeBase>(arg->getDataType()))
                        continue;
                    List<IRInst*> accessChain;
                    auto varInst = getRootAddr(arg, accessChain);
                    IRInst* debugVar = nullptr;
                    if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                    {
                        builder.setInsertAfter(callInst);
                        auto loadVal = builder.emitLoad(arg);
                        setDebugValue(debugVar, loadVal, accessChain.getArrayView());
                    }
                }
            }
        }
    }
}

void insertDebugValueStore(DebugValueStoreContext& context, IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto genericInst = as<IRGeneric>(globalInst))
        {
            if (auto func = as<IRFunc>(findGenericReturnVal(genericInst)))
            {
                context.insertDebugValueStore(func);
            }
        }
        else if (auto func = as<IRFunc>(globalInst))
        {
            context.insertDebugValueStore(func);
        }
    }
}
} // namespace Slang
