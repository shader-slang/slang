// slang-ir-marshal-native-call.h
#include "slang-ir-marshal-native-call.h"

#include "slang/ir/slang-ir-insts.h"
#include "slang/ir/slang-ir.h"
#include "slang/slang-rich-diagnostics.h"

namespace Slang
{

IRType* NativeCallMarshallingContext::getNativeType(IRBuilder& builder, IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_StringType:
        return builder.getNativeStringType();
    case kIROp_InterfaceType:
        return builder.getNativePtrType(type);
    case kIROp_ComPtrType:
        return builder.getNativePtrType((IRType*)as<IRComPtrType>(type)->getOperand(0));
    case kIROp_ArrayType:
    case kIROp_UnsizedArrayType:
        {
            auto arrayType = as<IRArrayType>(type);
            auto elementType = arrayType->getElementType();
            auto nativeElementType = getNativeType(builder, elementType);
            return builder.getArrayTypeBase(
                elementType->getOp(),
                nativeElementType,
                arrayType->getElementCount());
        }
    case kIROp_BorrowInOutParamType:
    case kIROp_RefParamType:
    case kIROp_BorrowInParamType:
    case kIROp_OutParamType:
        return builder.getPtrType(getNativeType(builder, (IRType*)type->getOperand(0)));
    default:
        return type;
    }
}

/// Returns whether a value of `type` is plain old data for a native call: its bytes mean the
/// same thing to native code, and copying them neither creates nor ends any ownership.
///
/// For example, `float3`, `NativeString`, `int*`, `Tuple<int, float>`, and
/// `struct { int a; float b[4]; }` qualify, while `String`, a class, a resource, a COM pointer, an
/// unsized array, and any struct, tuple, or array containing one of them do not.
///
/// `typesInProgress` holds the aggregates whose classification is under way. A self-referential
/// type such as `struct Node { int value; Node* next; }` reaches `Node` again through the pointer;
/// that occurrence adds no new element types, so it counts as plain data and the recursion stops.
static bool isPlainOldDataForNativeCall(IRType* type, HashSet<IRType*>& typesInProgress)
{
    if (as<IRBasicType>(type))
        return true;
    switch (type->getOp())
    {
    case kIROp_NativeStringType:
        return true;
    case kIROp_VectorType:
    case kIROp_MatrixType:
        return isPlainOldDataForNativeCall((IRType*)type->getOperand(0), typesInProgress);
    case kIROp_PtrType:
        {
            // A raw pointer is plain data only if native code can use what it points to.
            auto valueType = as<IRPtrTypeBase>(type)->getValueType();
            return as<IRVoidType>(valueType) ||
                   isPlainOldDataForNativeCall(valueType, typesInProgress);
        }
    case kIROp_ArrayType:
        return isPlainOldDataForNativeCall(
            as<IRArrayType>(type)->getElementType(),
            typesInProgress);
    case kIROp_UnsizedArrayType:
        // An unsized array has no size that native code could agree on. A trailing unsized
        // member could later map to a C flexible array member, but no form is supported yet.
        return false;
    case kIROp_StructType:
    case kIROp_TupleType:
        {
            if (!typesInProgress.add(type))
                return true;
            bool result = true;
            if (auto structType = as<IRStructType>(type))
            {
                for (auto field : structType->getFields())
                    result = result &&
                             isPlainOldDataForNativeCall(field->getFieldType(), typesInProgress);
            }
            else
            {
                // The DLL passes run before `lowerTuples`, so a tuple is still a tuple here; its
                // operands are its element types.
                for (UInt i = 0; i < type->getOperandCount(); ++i)
                    result =
                        result &&
                        isPlainOldDataForNativeCall((IRType*)type->getOperand(i), typesInProgress);
            }
            typesInProgress.remove(type);
            return result;
        }
    default:
        return false;
    }
}

/// Returns whether a value of `type` is plain old data for a native call.
static bool isPlainOldDataForNativeCall(IRType* type)
{
    HashSet<IRType*> typesInProgress;
    return isPlainOldDataForNativeCall(type, typesInProgress);
}

/// Returns whether `type` is a `[COM]` interface or a `ComPtr` to one, which a native signature
/// accepts because COM reference counting defines its ownership.
static bool isComInterfaceForNativeCall(IRType* type)
{
    return type->getOp() == kIROp_InterfaceType || type->getOp() == kIROp_ComPtrType;
}

bool NativeCallMarshallingContext::isFunctionSignatureValidForNativeCall(IRFunc* func)
{
    bool isSupported = true;
    auto funcType = func->getDataType();
    auto diagnose = [&](IRType* type, const char* position)
    {
        diagnosticSink->diagnose(Diagnostics::TypeNotSupportedInNativeSignature{
            .func = func,
            .type = type,
            .position = position,
            .location = func->sourceLoc});
        isSupported = false;
    };

    for (UInt i = 0; i < funcType->getParamCount(); ++i)
    {
        auto paramType = funcType->getParamType(i);
        if (auto ptrType = as<IRPtrTypeBase>(paramType);
            ptrType && paramType->getOp() != kIROp_PtrType)
        {
            // `out`, `inout`, `ref`, and `__constref` pass the address of the caller's storage,
            // so native code reads and writes the pointee directly.
            auto valueType = ptrType->getValueType();
            if (!isPlainOldDataForNativeCall(valueType) && !isComInterfaceForNativeCall(valueType))
                diagnose(valueType, "by-reference parameter");
        }
        else if (
            !isPlainOldDataForNativeCall(paramType) && !isComInterfaceForNativeCall(paramType) &&
            paramType->getOp() != kIROp_StringType)
        {
            diagnose(paramType, "parameter");
        }
    }

    auto resultType = funcType->getResultType();
    if (!isPlainOldDataForNativeCall(resultType) && !isComInterfaceForNativeCall(resultType))
        diagnose(resultType, "result");
    return isSupported;
}

IRFuncType* NativeCallMarshallingContext::getNativeFuncType(
    IRBuilder& builder,
    IRFuncType* declaredFuncType)
{
    List<IRInst*> nativeParamTypes;
    SLANG_ASSERT(declaredFuncType->getOp() == kIROp_FuncType);
    for (UInt i = 0; i < declaredFuncType->getParamCount(); ++i)
    {
        auto paramType = declaredFuncType->getParamType(i);
        nativeParamTypes.add(getNativeType(builder, (IRType*)(paramType)));
    }
    IRType* returnType = declaredFuncType->getResultType();
    if (auto resultType = as<IRResultType>(declaredFuncType->getResultType()))
    {
        auto nativeResultType = getNativeType(builder, resultType->getValueType());
        auto nativeErrorType = getNativeType(builder, resultType->getErrorType());
        nativeParamTypes.add(builder.getPtrType(nativeResultType));
        nativeParamTypes.add(builder.getPtrType(nativeErrorType));
        returnType = builder.getIntType();
    }
    else
    {
        returnType = getNativeType(builder, returnType);
    }
    auto funcType = builder.getFuncType(
        nativeParamTypes.getCount(),
        (IRType**)nativeParamTypes.getBuffer(),
        returnType);

    return funcType;
}

void NativeCallMarshallingContext::marshalRefManagedValueToNativeValue(
    IRBuilder& builder,
    IRInst* originalArg,
    List<IRInst*>& args)
{
    auto ptrTypeBase = as<IRPtrTypeBase>(originalArg->getDataType());
    SLANG_RELEASE_ASSERT(ptrTypeBase);
    switch (ptrTypeBase->getValueType()->getOp())
    {
    case kIROp_InterfaceType:
    case kIROp_ComPtrType:
        args.add(builder.emitGetManagedPtrWriteRef(originalArg));
        break;
    default:
        args.add(originalArg);
        break;
    }
}

void NativeCallMarshallingContext::marshalManagedValueToNativeValue(
    IRBuilder& builder,
    IRType* originalParamType,
    IRInst* originalArg,
    List<IRInst*>& args)
{
    switch (originalParamType->getOp())
    {
    case kIROp_BorrowInOutParamType:
    case kIROp_RefParamType:
    case kIROp_BorrowInParamType:
    case kIROp_OutParamType:
        return marshalRefManagedValueToNativeValue(builder, originalArg, args);
    case kIROp_StringType:
        {
            auto nativeStr = builder.emitGetNativeString(originalArg);
            args.add(nativeStr);
        }
        break;
    case kIROp_InterfaceType:
        {
            auto nativePtr = builder.emitGetNativePtr(originalArg);
            args.add(nativePtr);
        }
        break;
    default:
        args.add(originalArg);
        break;
    }
}

IRInst* NativeCallMarshallingContext::marshalNativeValueToManagedValue(
    IRBuilder& builder,
    IRInst* nativeVal)
{
    switch (nativeVal->getDataType()->getOp())
    {
    case kIROp_NativeStringType:
        {
            return builder.emitMakeString(nativeVal);
        }
        break;
    case kIROp_NativePtrType:
        {
            SLANG_RELEASE_ASSERT(
                nativeVal->getDataType()->getOperand(0)->getOp() == kIROp_InterfaceType);
            auto comPtrVar = builder.emitVar(
                builder.getComPtrType((IRType*)nativeVal->getDataType()->getOperand(0)));
            builder.emitManagedPtrAttach(comPtrVar, nativeVal);
            return builder.emitLoad(comPtrVar);
        }
        break;
    case kIROp_InterfaceType:
        {
            auto comPtrVar = builder.emitVar(nativeVal->getDataType());
            builder.emitManagedPtrAttach(comPtrVar, nativeVal);
            return builder.emitLoad(comPtrVar);
        }
        break;
    default:
        return nativeVal;
        break;
    }
}

void NativeCallMarshallingContext::marshalManagedValueToNativeResultValue(
    IRBuilder& builder,
    IRInst* originalArg,
    List<IRInst*>& args)
{
    switch (originalArg->getDataType()->getOp())
    {
    case kIROp_BorrowInOutParamType:
    case kIROp_RefParamType:
    case kIROp_BorrowInParamType:
        SLANG_UNREACHABLE("out and ref types should be handled before reaching here.");
        break;
    case kIROp_StringType:
    case kIROp_ClassType:
        SLANG_UNREACHABLE("isFunctionSignatureValidForNativeCall rejects managed native results.");
        break;
    case kIROp_InterfaceType:
        {
            auto nativePtr = builder.emitManagedPtrDetach(
                builder.getNativePtrType(originalArg->getDataType()),
                originalArg);
            args.add(nativePtr);
        }
        break;
    case kIROp_ComPtrType:
        {
            auto nativePtr = builder.emitManagedPtrDetach(
                builder.getNativePtrType(
                    (IRType*)cast<IRComPtrType>(originalArg->getDataType())->getOperand(0)),
                originalArg);
            args.add(nativePtr);
        }
        break;
    default:
        args.add(originalArg);
        break;
    }
}

IRInst* NativeCallMarshallingContext::marshalNativeArgToManagedArg(
    IRBuilder& builder,
    const List<IRInst*>& args,
    Index& consumeIndex,
    IRType* expectedManagedType)
{
    // For now, all managed values maps to one native value, so we just call
    // `marshalNativeValueToManagedValue`. This function can be extended in the future to support
    // things like `List` that maps to more than one native args.
    SLANG_UNUSED(expectedManagedType);
    auto managedVal = marshalNativeValueToManagedValue(builder, args[consumeIndex]);
    consumeIndex++;
    return managedVal;
}

IRFunc* NativeCallMarshallingContext::generateDLLExportWrapperFunc(
    IRBuilder& builder,
    IRFunc* originalFunc)
{
    builder.setInsertBefore(originalFunc);
    auto funcType = getNativeFuncType(builder, originalFunc->getDataType());
    auto newFunc = builder.createFunc();
    newFunc->setFullType(funcType);
    builder.setInsertInto(newFunc);
    builder.emitBlock();
    List<IRInst*> params;
    for (UInt i = 0; i < funcType->getParamCount(); i++)
    {
        auto paramType = funcType->getParamType(i);
        params.add(builder.emitParam(paramType));
    }
    List<IRInst*> args;
    Index nativeParamConsumeIndex = 0;
    for (UInt i = 0; i < originalFunc->getParamCount(); i++)
    {
        auto managedParamType = originalFunc->getParamType(i);
        auto managedArg = marshalNativeArgToManagedArg(
            builder,
            params,
            nativeParamConsumeIndex,
            managedParamType);
        args.add(managedArg);
    }
    auto originalReturnType = originalFunc->getResultType();
    auto callInst = builder.emitCallInst(originalReturnType, originalFunc, args);
    if (const auto resultType = as<IRResultType>(originalReturnType); resultType)
    {
        auto isResultError = builder.emitIsResultError(callInst);
        IRBlock* trueBlock = nullptr;
        IRBlock* falseBlock = nullptr;
        IRBlock* afterBlock = nullptr;

        builder.emitIfElseWithBlocks(isResultError, trueBlock, falseBlock, afterBlock);

        builder.setInsertInto(falseBlock);
        auto resultVal = builder.emitGetResultValue(callInst);
        List<IRInst*> nativeVals;
        marshalManagedValueToNativeResultValue(builder, resultVal, nativeVals);
        for (Index i = 0; i < nativeVals.getCount(); i++)
        {
            SLANG_RELEASE_ASSERT(nativeParamConsumeIndex < params.getCount());
            builder.emitStore(params[nativeParamConsumeIndex], nativeVals[i]);
            nativeParamConsumeIndex++;
        }
        // S_OK
        builder.emitReturn(builder.getIntValue(builder.getIntType(), 0));

        builder.setInsertInto(trueBlock);
        nativeVals.clear();
        auto errorVal = builder.emitGetResultError(callInst);
        marshalManagedValueToNativeResultValue(builder, errorVal, nativeVals);
        for (Index i = 0; i < nativeVals.getCount(); i++)
        {
            SLANG_RELEASE_ASSERT(nativeParamConsumeIndex < params.getCount());
            builder.emitStore(params[nativeParamConsumeIndex], nativeVals[i]);
            nativeParamConsumeIndex++;
        }
        // E_FAIL
        builder.emitReturn(builder.getIntValue(builder.getIntType(), 0x80004005));

        builder.setInsertInto(afterBlock);
        builder.emitUnreachable();
    }
    else
    {
        List<IRInst*> nativeVals;
        marshalManagedValueToNativeResultValue(builder, callInst, nativeVals);
        for (Index i = 1; i < nativeVals.getCount(); i++)
        {
            SLANG_RELEASE_ASSERT(nativeParamConsumeIndex < params.getCount());
            builder.emitStore(params[nativeParamConsumeIndex], nativeVals[i]);
            nativeParamConsumeIndex++;
        }
        builder.emitReturn(nativeVals[0]);
    }
    return newFunc;
}

IRInst* NativeCallMarshallingContext::marshalNativeCall(
    IRBuilder& builder,
    IRFuncType* originalFuncType,
    IRFuncType* nativeFuncType,
    IRInst* nativeFunc,
    Int argCount,
    IRInst* const* originalArgs)
{
    // Marshal parameters to arguments into native func.
    List<IRInst*> args;
    for (Int i = 0; i < argCount; i++)
    {
        auto paramType = originalFuncType->getParamType(i);
        marshalManagedValueToNativeValue(builder, paramType, originalArgs[i], args);
    }
    IRType* originalReturnType = originalFuncType->getResultType();

    IRVar* resultVar = nullptr;
    IRVar* errorVar = nullptr;
    if (auto resultType = as<IRResultType>(originalReturnType))
    {
        // Declare a local variable to receive result.
        resultVar = builder.emitVar(getNativeType(builder, resultType->getValueType()));
        errorVar = builder.emitVar(getNativeType(builder, resultType->getErrorType()));
        args.add(resultVar);
        args.add(errorVar);
    }

    // Insert call.
    IRInst* call = builder.emitCallInst(nativeFuncType->getResultType(), nativeFunc, args);

    // TODO: marshal output/ref args back to original args.

    IRInst* returnValue = call;

    // Marshal result and out arguments back to managed values.
    if (auto resultType = as<IRResultType>(originalReturnType))
    {
        auto val = builder.emitLoad(resultVar);
        auto err = builder.emitLoad(errorVar);
        auto tag = call;
        val = marshalNativeValueToManagedValue(builder, val);
        err = marshalNativeValueToManagedValue(builder, err);
        auto intTag = tag;
        if (tag->getDataType()->getOp() != kIROp_IntType)
        {
            intTag = builder.emitCast(builder.getIntType(), tag);
        }
        auto tagIsError = builder.emitLess(intTag, builder.getIntValue(builder.getIntType(), 0));
        IRBlock *trueBlock, *falseBlock, *afterBlock;
        builder.emitIfElseWithBlocks(tagIsError, trueBlock, falseBlock, afterBlock);
        builder.setInsertInto(trueBlock);
        returnValue = builder.emitMakeResultError(resultType, err);
        builder.emitBranch(afterBlock, 1, &returnValue);
        builder.setInsertInto(falseBlock);
        returnValue = builder.emitMakeResultValue(resultType, val);
        builder.emitBranch(afterBlock, 1, &returnValue);
        builder.setInsertInto(afterBlock);
        returnValue = builder.emitParam(resultType);
    }
    else
    {
        returnValue = marshalNativeValueToManagedValue(builder, call);
    }
    return returnValue;
}

} // namespace Slang
