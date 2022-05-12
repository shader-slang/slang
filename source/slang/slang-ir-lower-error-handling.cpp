// slang-ir-lower-error-handling.cpp

#include "slang-ir-lower-error-handling.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct ErrorHandlingLoweringContext
{
    IRModule* module;
    DiagnosticSink* diagnosticSink;

    SharedIRBuilder sharedBuilder;

    List<IRInst*> workList;
    HashSet<IRInst*> workListSet;

    void addToWorkList(IRInst* inst)
    {
        if (workListSet.Contains(inst))
            return;

        workList.add(inst);
        workListSet.Add(inst);
    }

    void processFuncType(IRFuncType* funcType)
    {
        auto throwAttr = funcType->findAttr<IRFuncThrowTypeAttr>();
        if (!throwAttr)
            return;
        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(funcType);
        auto resultType =
            builder.getResultType(funcType->getResultType(), throwAttr->getErrorType());
        List<IRType*> paramTypes;
        for (UInt i = 0; i < funcType->getParamCount(); i++)
        {
            if (as<IRAttr>(funcType->getParamType(i)))
                break;
            paramTypes.add(funcType->getParamType(i));
        }
        auto newFuncType = builder.getFuncType(paramTypes, resultType);
        sharedBuilder.replaceGlobalInst(funcType, newFuncType);
    }

    void processTryCall(IRTryCall* tryCall)
    {
        // If we see:
        // ```
        //      value = tryCall(callee, errorVar, successBlock, failBlock, args) : returnType
        //  successBlock:
        //      ... (uses value) ...
        //  failBlock:
        //      errorVal = load(errorVar);
        //      (uses errorVal)
        // ```
        // We need to rewrite it as
        // ```
        //      result = call(callee) : Result<callee.returnType, callee.errorType>
        //      isError = isResultError(result)
        //      ifElse(isError, failBlock, successBlock)
        //  successBlock:
        //      value = getResultValue(result) : returnType
        //      ...
        //  failBlock:
        //      error = getResultError(result) : errorType
        //      store(errorVAR, error);
        //      ...
        // ```
        IRFuncType* funcType = cast<IRFuncType>(tryCall->getCallee()->getDataType());
        auto resultValueType = funcType->getResultType();
        auto throwAttr = funcType->findAttr<IRFuncThrowTypeAttr>();
        if (!throwAttr)
        {
            SLANG_ASSERT_FAILURE("tryCall applied to callee without a IRFuncThrowTypeAttr");
        }
        auto errorType = throwAttr->getErrorType();

        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(tryCall);

        auto resultType = builder.getResultType(resultValueType, errorType);
        List<IRInst*> args;
        for (UInt i = 0; i < tryCall->getArgCount(); i++)
        {
            args.add(tryCall->getArg(i));
        }
        auto call = builder.emitCallInst(resultType, tryCall->getCallee(), args);
        auto isFail = builder.emitIsResultError(call);
        builder.emitIf(isFail, tryCall->getFailureBlock(), tryCall->getSuccessBlock());
        auto failBlock = tryCall->getFailureBlock();
        builder.setInsertBefore(failBlock->getFirstOrdinaryInst());

        // The generated code in fail block will load error code from `tryCall->errorVar`.
        // We need to store the actual error code at the begining of the fail block so the
        // rest of it can successfully load the error code.
        auto errorVar = tryCall->getError();
        builder.emitStore(errorVar, builder.emitGetResultError(call));
        auto successBlock = tryCall->getSuccessBlock();
        builder.setInsertBefore(successBlock->getFirstOrdinaryInst());
        if (resultValueType->getOp() != kIROp_VoidType)
        {
            auto resultValue = builder.emitGetResultValue(call);
            tryCall->replaceUsesWith(resultValue);
        }
        tryCall->removeAndDeallocate();
    }

    void processReturn(IRReturn* ret)
    {
        auto parentFunc = getParentFunc(ret);
        if (!parentFunc)
            return;
        auto funcType = cast<IRFuncType>(parentFunc->getDataType());
        auto throwAttr = funcType->findAttr<IRFuncThrowTypeAttr>();
        if (!throwAttr)
            return;

        // If we are in a throwing function and sees a `return(val)` inst,
        // replace it with a `return makeResultValue(val)`, so that it returns a `Result<T,E>` type.
        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(ret);
        auto resultType =
            builder.getResultType(funcType->getResultType(), throwAttr->getErrorType());
        IRInst* resultVal = nullptr;
        if (ret->getOp() == kIROp_ReturnVal)
        {
            auto val = cast<IRReturnVal>(ret)->getVal();
            resultVal = builder.emitMakeResultValue(resultType, val);
        }
        else
        {
            resultVal = builder.emitMakeResultValueVoid(resultType);
        }
        builder.emitReturn(resultVal);
        ret->removeAndDeallocate();
    }

    void processThrow(IRThrow* throwInst)
    {
        auto parentFunc = getParentFunc(throwInst);
        SLANG_ASSERT(parentFunc);
        auto funcType = cast<IRFuncType>(parentFunc->getDataType());
        auto throwAttr = funcType->findAttr<IRFuncThrowTypeAttr>();
        SLANG_ASSERT(throwAttr);

        // If we are in a throwing function and sees a `throw(e)` inst,
        // replace it with a `return makeResultError(e)`.
        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(throwInst);
        auto resultType =
            builder.getResultType(funcType->getResultType(), throwAttr->getErrorType());
        IRInst* resultVal = builder.emitMakeResultError(resultType, throwInst->getValue());
        builder.emitReturn(resultVal);
        throwInst->removeAndDeallocate();
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_TryCall:
            processTryCall(cast<IRTryCall>(inst));
            break;
        case kIROp_ReturnVal:
        case kIROp_ReturnVoid:
            processReturn(cast<IRReturn>(inst));
            break;
        case kIROp_Throw:
            processThrow(cast<IRThrow>(inst));
            break;
        default:
            break;
        }
    }

    void processInsts()
    {
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.Remove(inst);

            processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }
    }

    void processModule()
    {
        // Deduplicate equivalent types.
        sharedBuilder.deduplicateAndRebuildGlobalNumberingMap();   

        // Translate all IRTryCall, IRThrow, IRReturn, IRReturnVal.
        processInsts();

        // Lower all functypes.
        // Function types with an IRThrowTypeAttribute will be translated into a normal function
        // type that returns `Result<T,E>`.
        List<IRFuncType*> oldFuncTypes;
        for (auto child : module->getGlobalInsts())
        {
            switch (child->getOp())
            {
            case kIROp_FuncType:
                oldFuncTypes.add(cast<IRFuncType>(child));
                break;
            default:
                break;
            }
        }
        for (auto funcType : oldFuncTypes)
        {
            processFuncType(funcType);
        }
    }
};

void lowerErrorHandling(IRModule* module, DiagnosticSink* sink)
{
    ErrorHandlingLoweringContext context;
    context.module = module;
    context.diagnosticSink = sink;
    context.sharedBuilder.init(module);
    return context.processModule();
}
}
