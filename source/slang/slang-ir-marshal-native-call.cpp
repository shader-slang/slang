// slang-ir-marshal-native-call.h
#include "slang-ir-marshal-native-call.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

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
        default:
            return type;
        }
    }

    IRFuncType* NativeCallMarshallingContext::getNativeFuncType(
        IRBuilder& builder, IRFuncType* declaredFuncType)
    {
        List<IRInst*> nativeParamTypes;
        assert(declaredFuncType->getOp() == kIROp_FuncType);
        for (UInt i = 0; i < declaredFuncType->getParamCount(); ++i)
        {
            auto paramType = declaredFuncType->getParamType(i);
            nativeParamTypes.add(getNativeType(builder, as<IRType>(paramType)));
        }
        IRType* returnType = declaredFuncType->getResultType();
        if (auto resultType = as<IRResultType>(declaredFuncType->getResultType()))
        {
            auto nativeResultType = getNativeType(builder, resultType->getValueType());
            nativeParamTypes.add(builder.getPtrType(nativeResultType));
            returnType = resultType->getErrorType();
        }
        else
        {
            returnType = getNativeType(builder, returnType);
        }
        auto funcType = builder.getFuncType(
            nativeParamTypes.getCount(), (IRType**)nativeParamTypes.getBuffer(), returnType);

        return funcType;
    }

    void NativeCallMarshallingContext::marshalRefManagedValueToNativeValue(
        IRBuilder& builder, IRInst* originalArg, List<IRInst*>& args)
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
        IRBuilder& builder, IRInst* originalArg, List<IRInst*>& args)
    {
        switch (originalArg->getDataType()->getOp())
        {
        case kIROp_InOutType:
        case kIROp_RefType:
            return marshalRefManagedValueToNativeValue(
                builder, originalArg, args);
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
        IRBuilder& builder, IRInst* nativeVal)
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
                SLANG_RELEASE_ASSERT(nativeVal->getDataType()->getOperand(0)->getOp() == kIROp_InterfaceType);
                auto comPtrVar = builder.emitVar(builder.getComPtrType((IRType*)nativeVal->getDataType()->getOperand(0)));
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

    IRInst* NativeCallMarshallingContext::marshalNativeCall(
        IRBuilder& builder,
        IRFuncType* originalFuncType,
        IRFuncType* nativeFuncType,
        IRInst* nativeFunc,
        Int argCount,
        IRInst*const* originalArgs)
    {
        // Marshal parameters to arguments into native func.
        List<IRInst*> args;
        for (Int i = 0; i < argCount; i++)
        {
            marshalManagedValueToNativeValue(builder, originalArgs[i], args);
        }
        IRType* originalReturnType = originalFuncType->getResultType();

        IRVar* resultVar = nullptr;
        if (auto resultType = as<IRResultType>(originalReturnType))
        {
            // Declare a local variable to receive result.
            resultVar = builder.emitVar(getNativeType(builder, resultType->getValueType()));
            args.add(resultVar);
        }

        // Insert call.
        IRInst* call = builder.emitCallInst(nativeFuncType->getResultType(), nativeFunc, args);

        // TODO: marshal output/ref args back to original args.

        IRInst* returnValue = call;

        // Marshal result and out arguments back to managed values.
        if (auto resultType = as<IRResultType>(originalReturnType))
        {
            auto val = builder.emitLoad(resultVar);
            auto err = call;
            val = marshalNativeValueToManagedValue(builder, val);
            auto intErr = err;
            if (err->getDataType()->getOp() != kIROp_IntType)
            {
                intErr = builder.emitConstructorInst(builder.getIntType(), 1, &err);
            }
            auto errIsError = builder.emitLess(intErr, builder.getIntValue(builder.getIntType(), 0));
            IRBlock *trueBlock, *falseBlock, *afterBlock;
            builder.emitIfElseWithBlocks(errIsError, trueBlock, falseBlock, afterBlock);
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
