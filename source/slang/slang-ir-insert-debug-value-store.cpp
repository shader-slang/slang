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
    case kIROp_EnumType:
        {
            auto enumType = as<IREnumType>(type);
            debuggable = isDebuggableType(enumType->getTagType());
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

    // Build idempotency markers for params that were already instrumented in a prior pass.
    // insertDebugValueStore is called twice: once early in slang-lower-to-ir.cpp (before
    // specialization) and once after specializeModule (to pick up variables whose types were
    // unresolved IRSpecialize the first time). The second call must not create duplicate
    // IRDebugVar records for params already instrumented by the first call.
    //
    // Two complementary signals cover all param kinds:
    //
    //  1. In-params and borrow-in params: the first pass emits an IRDebugValue whose value is
    //     the IRParam directly (for plain in-params) or an IRLoad of the IRParam (for borrow-in
    //     params). Scanning for both shapes gives a set of already-processed by-value params.
    //
    //  2. Out-params (e.g. `this` in an initializer) and proxy-var params: the first pass does
    //     NOT emit an initial IRDebugValue, so signal (1) misses them. However,
    //     copyNameHintAndDebugDecorations copies the IRNameHintDecoration from the param to its
    //     IRDebugVar, and IRDebugVar records for params carry a non-null argIndex operand.
    //     Scanning for such IRDebugVar records and collecting their name hints covers these cases.
    HashSet<IRInst*> alreadyProcessedInParams;
    HashSet<UnownedStringSlice> existingParamDebugVarNames;
    for (auto inst = firstBlock->getFirstInst(); inst; inst = inst->getNextInst())
    {
        if (auto debugValue = as<IRDebugValue>(inst))
        {
            auto val = debugValue->getValue();
            if (as<IRParam>(val))
            {
                alreadyProcessedInParams.add(val);
            }
            else if (auto load = as<IRLoad>(val))
            {
                // Borrow-in params emit IRDebugValue(debugVar, IRLoad(param)).
                if (as<IRParam>(load->getPtr()))
                    alreadyProcessedInParams.add(load->getPtr());
            }
        }
        else if (auto debugVar = as<IRDebugVar>(inst))
        {
            if (debugVar->getArgIndex())
            {
                if (auto nameHint = debugVar->findDecoration<IRNameHintDecoration>())
                    existingParamDebugVarNames.add(nameHint->getName());
            }
        }
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

        // Skip params already instrumented in a previous pass (see comment above).
        // Check both signals: direct IRDebugValue reference (in-params) and name-hint match
        // against an existing argIndex-bearing IRDebugVar (out-params such as `this`).
        if (alreadyProcessedInParams.contains(param))
        {
            paramIndex++;
            continue;
        }
        if (auto nameHint = param->findDecoration<IRNameHintDecoration>())
        {
            if (existingParamDebugVarNames.contains(nameHint->getName()))
            {
                paramIndex++;
                continue;
            }
        }

        auto debugVar = builder.emitDebugVar(
            paramType,
            funcDebugLoc->getSource(),
            funcDebugLoc->getLine(),
            funcDebugLoc->getCol(),
            builder.getIntValue(builder.getUIntType(), paramIndex));
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
                if (auto debugLoc = varInst->findDecoration<IRDebugLocationDecoration>())
                {
                    auto varType = tryGetPointedToType(&builder, varInst->getDataType());
                    if (!isDebuggableType(varType))
                        continue;

                    // Skip IRVar instances that were already instrumented in a previous pass.
                    // insertDebugValueStore always inserts the IRDebugVar immediately before the
                    // IRVar it corresponds to, so the presence of kIROp_DebugVar as the
                    // immediately preceding instruction is a reliable marker.
                    auto prevInst = varInst->getPrevInst();
                    if (prevInst && prevInst->getOp() == kIROp_DebugVar)
                        continue;

                    builder.setInsertBefore(varInst);
                    auto debugVar = builder.emitDebugVar(
                        varType,
                        debugLoc->getSource(),
                        debugLoc->getLine(),
                        debugLoc->getCol());
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
