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

// True if `type` (after unwrapping attributed types) is a leaf opaque handle eligible for a
// source-level debug variable: a texture or a sampler-state-family type. Deliberately narrower than
// the broad isResourceType legalization classifier — no array unwrapping, no buffer / pointer-like
// / subpass / builtin-generic shapes — because only these leaf handles get a DebugLocalVariable
// with no backing OpVariable.
static bool isSupportedOpaqueDebugHandleType(IRType* type)
{
    type = (IRType*)unwrapAttributedType(type);
    return as<IRTextureTypeBase>(type) || as<IRSamplerStateTypeBase>(type);
}

// True if a function-local `var`/`let` of `type` should get a source-level DebugVar. Extends
// isDebuggableType (scalars/vectors/matrices and aggregates of those) with the supported opaque
// leaf handles: unlike plain data, a handle gets a DebugLocalVariable with no backing OpVariable
// and is bound to its lowered SSA value by a DebugValue. This is a type decision only; whether the
// concrete lowered value is a representable DebugValue operand is checked in SPIR-V legalization.
bool DebugValueStoreContext::isDebugVarTypeSupported(IRType* type)
{
    return isDebuggableType(type) || isSupportedOpaqueDebugHandleType(type);
}

void DebugValueStoreContext::insertDebugValueStore(IRFunc* func)
{
    IRBuilder builder(func);
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return;
    auto funcDebugLoc = func->findDecoration<IRDebugLocationDecoration>();
    if (!funcDebugLoc)
        return;
    auto funcDebugDecor = func->findDecoration<IRDebugFuncDecoration>();
    SLANG_RELEASE_ASSERT(funcDebugDecor && "function has DebugLocation but no DebugFunc");
    auto debugFunc = funcDebugDecor->getDebugFunc();
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
        auto debugVar = builder.emitDebugVar(
            paramType,
            funcDebugLoc->getSource(),
            funcDebugLoc->getLine(),
            funcDebugLoc->getCol(),
            debugFunc,
            builder.getIntValue(builder.getUIntType(), paramIndex));
        copyNameHintAndDebugDecorations(debugVar, param);

        // Lowering copies some `in` parameters into a proxy variable that the body reads and
        // writes in place of the parameter, so that variable's storage holds the parameter.
        IRInst* proxyVar = nullptr;
        for (auto use = param->firstUse; use; use = use->nextUse)
        {
            if (auto inParamProxyVarDecor = as<IRInParamProxyVarDecoration>(use->getUser()))
                proxyVar = inParamProxyVarDecor->parent;
        }

        if (proxyVar)
        {
            builder.setInsertAfter(proxyVar);
            builder.emitDebugDeclare(debugVar, proxyVar);
        }
        else if (isRefParam)
        {
            // An `out`, `inout` or `borrow` parameter points at the storage that holds its value.
            builder.emitDebugDeclare(debugVar, param);
        }
        else
        {
            builder.emitDebugValue(debugVar, param);
        }
        paramIndex++;
    }

    // A local variable is described by its storage from the point of its declaration on;
    // passes that remove or replace the storage rewrite the `DebugDeclare`.
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
                    builder.setInsertBefore(varInst);
                    if (!isDebugVarTypeSupported(varType))
                        continue;
                    auto debugVar = builder.emitDebugVar(
                        varType,
                        debugLoc->getSource(),
                        debugLoc->getLine(),
                        debugLoc->getCol(),
                        debugLoc->getScope());
                    copyNameHintAndDebugDecorations(debugVar, varInst);
                    builder.setInsertAfter(varInst);
                    builder.emitDebugDeclare(debugVar, varInst);
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
