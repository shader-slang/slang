// slang-ir-lower-error-handling.cpp

#include "slang-ir-lower-error-handling.h"

#include "slang-ir-clone.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

struct ErrorHandlingLoweringContext
{
    IRModule* module;
    DiagnosticSink* diagnosticSink;

    InstWorkList workList;
    InstHashSet workListSet;
    List<IRFuncType*> oldFuncTypes;
    HashSet<IRBlock*> catchBlocks;

    ErrorHandlingLoweringContext(IRModule* inModule)
        : module(inModule), workList(inModule), workListSet(inModule)
    {
    }

    void addToWorkList(IRInst* inst)
    {
        if (workListSet.contains(inst))
            return;

        workList.add(inst);
        workListSet.add(inst);
    }

    void processFuncType(IRFuncType* funcType)
    {
        auto throwAttr = funcType->findAttr<IRFuncThrowTypeAttr>();
        if (!throwAttr)
            return;
        IRBuilder builder(module);
        builder.setInsertBefore(funcType);

        auto resultType = builder.getResultType(
            funcType->getResultType(),
            throwAttr->getErrorType(),
            throwAttr->getErrorTypeWitness());

        List<IRType*> paramTypes;
        for (UInt i = 0; i < funcType->getParamCount(); i++)
        {
            if (as<IRAttr>(funcType->getParamType(i)))
                break;
            paramTypes.add(funcType->getParamType(i));
        }
        auto newFuncType = builder.getFuncType(paramTypes, resultType);
        funcType->replaceUsesWith(newFuncType);
        funcType->removeAndDeallocate();
    }

    /*
    void cloneHandlerBlock(
        IRBuilder* builder,
        IRDominatorTree* dom,
        IRBlock* handler,
        IRBlock* merge,
        IRBlock* clonedHandler,
        IRInst* resultVal
    ){
        auto handlerDominatedBlocks = dom->getProperlyDominatedBlocks(handler);
        List<IRBlock*> handlerBlocks;
        IRCloneEnv env;

        catchBlocks.add(handler);
        handlerBlocks.add(handler);
        builder->addInst(clonedHandler);
        env.mapOldValToNew[handler] = clonedHandler;

        for (IRBlock* block : handlerDominatedBlocks)
        {
            if (!dom->properlyDominates(merge, block) && block != merge)
            {
                catchBlocks.add(block);
                handlerBlocks.add(block);
                auto clonedBlock = builder->createBlock();
                builder->addInst(clonedBlock);
                env.mapOldValToNew[block] = clonedBlock;
            }
        }

        IRBlock* handlerBlock = nullptr;
        for (auto block : handlerBlocks)
        {
            auto clonedBlock = as<IRBlock>(env.mapOldValToNew.getValue(block));
            if (handlerBlock == nullptr)
                handlerBlock = clonedBlock;
            builder->setInsertInto(clonedBlock);
            for (auto inst : block->getChildren())
                cloneInst(&env, builder, inst);
        }

        builder->setInsertBefore(clonedHandler->getFirstOrdinaryInst());
        auto errVal = builder->emitGetResultError(resultVal);
        auto errorParam = clonedHandler->getFirstParam();
        errorParam->replaceUsesWith(errVal);
        errorParam->removeAndDeallocate();
    }
    */

    void processTryCall(IRTryCall* tryCall)
    {
        // If we see:
        // ```
        //      value = tryCall(callee, successBlock, failBlock, args)
        //  successBlock:
        //      resultParam = IRParam<resultType>
        //      ... (uses resultParam) ...
        //  failBlock:
        //      errorParam = IRParam<errorType>
        //      (uses errorParam)
        // ```
        // We need to rewrite it as
        // ```
        //      result = call(callee) : Result<callee.returnType, callee.errorType>
        //      isError = isResultError(result)
        //      ifElse(isError, failBlock, successBlock)
        //  successBlock:
        //      value = getResultValue(result) : returnType
        //      ... (replaces resultParam with value)
        //  failBlock:
        //      error = getResultError(result) : errorType
        //      ... (replaces errorParam with error)
        // ```
        IRFuncType* funcType = cast<IRFuncType>(tryCall->getCallee()->getDataType());
        auto resultValueType = funcType->getResultType();
        auto throwAttr = funcType->findAttr<IRFuncThrowTypeAttr>();
        if (!throwAttr)
        {
            SLANG_ASSERT_FAILURE("tryCall applied to callee without a IRFuncThrowTypeAttr");
        }
        auto errorType = throwAttr->getErrorType();

        IRBuilder builder(module);

        auto successBlock = tryCall->getSuccessBlock();
        auto failBlock = tryCall->getFailureBlock();
        //auto mergeBlock = tryCall->getMergeBlock();

        auto parentFunc = cast<IRFunc>(successBlock->getParent());
        SLANG_ASSERT(parentFunc);
        //auto dom = module->findOrCreateDominatorTree(parentFunc);

        builder.setInsertBefore(tryCall);

        auto resultType = builder.getResultType(resultValueType, errorType, throwAttr->getErrorTypeWitness());
        List<IRInst*> args;
        for (UInt i = 0; i < tryCall->getArgCount(); i++)
        {
            args.add(tryCall->getArg(i));
        }
        auto call = builder.emitCallInst(resultType, tryCall->getCallee(), args);
        tryCall->transferDecorationsTo(call);

        auto isError = builder.emitIsResultError(call);
        auto catchBlock = builder.createBlock();
        auto branch = builder.emitIfElse(isError, catchBlock, successBlock, successBlock);

        // Replace the params in failBlock to `getResultError(call)`.
        builder.setInsertAfter(branch->getParent());
        builder.addInst(catchBlock);
        builder.setInsertInto(catchBlock);

        auto errorValue = builder.emitGetResultError(call);
        builder.emitBranch(failBlock, 1, &errorValue);

        //auto errorParam = failBlock->getFirstParam();
        //errorParam->replaceUsesWith(errorValue);
        //errorParam->removeAndDeallocate();
        //cloneHandlerBlock(&builder, dom, failBlock, mergeBlock, handlerBlock, call);

        // Replace the params in successBlock to `getResultValue(call)`.
        builder.setInsertBefore(successBlock->getFirstOrdinaryInst());
        auto resultParam = successBlock->getFirstParam();
        auto resultValue = builder.emitGetResultValue(call);
        resultParam->replaceUsesWith(resultValue);
        resultParam->removeAndDeallocate();

        tryCall->removeAndDeallocate();

        // Some blocks got removed and added, so mark analysis of the
        // function with defer as outdated.
        //module->invalidateAnalysisForInst(parentFunc);
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
        IRBuilder builder(module);
        builder.setInsertBefore(ret);
        auto resultType =
            builder.getResultType(funcType->getResultType(), throwAttr->getErrorType(), throwAttr->getErrorTypeWitness());
        IRInst* resultVal = nullptr;
        auto val = cast<IRReturn>(ret)->getVal();
        resultVal = builder.emitMakeResultValue(resultType, val);
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
        IRBuilder builder(module);
        builder.setInsertBefore(throwInst);
        auto resultType =
            builder.getResultType(funcType->getResultType(), throwAttr->getErrorType(), throwAttr->getErrorTypeWitness());
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
        case kIROp_Return:
            processReturn(cast<IRReturn>(inst));
            break;
        case kIROp_Throw:
            processThrow(cast<IRThrow>(inst));
            break;
        case kIROp_FuncType:
            oldFuncTypes.add(cast<IRFuncType>(inst));
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
            workListSet.remove(inst);

            processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }
    }

    void processModule()
    {
        // Translate all IRTryCall, IRThrow, IRReturn.
        processInsts();

        // Lower all functypes.
        // Function types with an IRThrowTypeAttribute will be translated into a normal function
        // type that returns `Result<T,E>`.
        for (auto funcType : oldFuncTypes)
        {
            processFuncType(funcType);
        }

        /*
        // Remove all catch blocks, their contents were already duplicated
        // before, so they're just dead code now.
        for (IRBlock* catchBlock : catchBlocks)
        {
            auto parentFunc = cast<IRFunc>(catchBlock->getParent());
            SLANG_ASSERT(parentFunc);

            catchBlock->removeAndDeallocate();

            module->invalidateAnalysisForInst(parentFunc);
        }
        */
    }
};

void lowerErrorHandling(IRModule* module, DiagnosticSink* sink)
{
    ErrorHandlingLoweringContext context(module);
    context.diagnosticSink = sink;
    return context.processModule();
}
} // namespace Slang
