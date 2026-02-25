#include "slang-ir-lower-reinterpret.h"

#include "slang-ir-any-value-inference.h"
#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir.h"

namespace Slang
{

struct ReinterpretOptionalKey
{
    IRType* srcType;
    IRType* destType;

    bool operator==(const ReinterpretOptionalKey& other) const
    {
        return srcType == other.srcType && destType == other.destType;
    }

    HashCode getHashCode() const
    {
        return combineHash(Slang::getHashCode(srcType), Slang::getHashCode(destType));
    }
};

struct ReinterpretLoweringContext
{
    TargetProgram* targetProgram;
    DiagnosticSink* sink;
    IRModule* module;
    OrderedHashSet<IRInst*> workList;

    void addToWorkList(IRInst* inst)
    {
        if (workList.contains(inst))
            return;

        workList.add(inst);
    }

    void processInst(IRInst* inst, IROp targetOp)
    {
        if (inst->getOp() == targetOp)
        {
            switch (targetOp)
            {
            case kIROp_Reinterpret:
                processReinterpret(inst);
                break;
            default:
                break;
            }
        }
    }

    void processModuleForOp(IROp targetOp)
    {
        workList.clear();
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();

            processInst(inst, targetOp);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }
    }

    void processModule()
    {
        // Then process regular Reinterpret instructions
        processModuleForOp(kIROp_Reinterpret);
    }

    void processReinterpret(IRInst* inst)
    {
        auto operand = inst->getOperand(0);
        auto fromType = operand->getDataType();
        auto toType = inst->getDataType();
        SlangInt fromTypeSize = getAnyValueSize(fromType, targetProgram->getTargetReq());
        if (fromTypeSize < 0)
        {
            sink->diagnose(
                inst->sourceLoc,
                Slang::Diagnostics::typeCannotBePackedIntoAnyValue,
                fromType);
        }
        SlangInt toTypeSize = getAnyValueSize(toType, targetProgram->getTargetReq());
        if (toTypeSize < 0)
        {
            sink->diagnose(
                inst->sourceLoc,
                Slang::Diagnostics::typeCannotBePackedIntoAnyValue,
                toType);
        }
        SlangInt anyValueSize = Math::Max(fromTypeSize, toTypeSize);

        IRBuilder builder(module);
        builder.setInsertBefore(inst);
        auto anyValueType =
            builder.getAnyValueType(builder.getIntValue(builder.getUIntType(), anyValueSize));
        auto packInst = builder.emitPackAnyValue(anyValueType, operand);
        auto unpackInst = builder.emitUnpackAnyValue(toType, packInst);
        inst->replaceUsesWith(unpackInst);
        inst->removeAndDeallocate();
    }
};


struct ReinterpretOptionalLoweringContext
{
    TargetProgram* targetProgram;
    DiagnosticSink* sink;
    IRModule* module;
    OrderedHashSet<IRInst*> workList;

    // Cache for ReinterpretOptional helper functions, keyed by (srcType, destType) pair
    Dictionary<ReinterpretOptionalKey, IRFunc*> reinterpretOptionalFuncCache;

    void addToWorkList(IRInst* inst)
    {
        if (workList.contains(inst))
            return;

        workList.add(inst);
    }

    void processInst(IRInst* inst, IROp targetOp)
    {
        if (inst->getOp() == targetOp)
        {
            switch (targetOp)
            {
            case kIROp_ReinterpretOptional:
                processReinterpretOptional(inst);
                break;
            default:
                break;
            }
        }
    }

    void processModuleForOp(IROp targetOp)
    {
        workList.clear();
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();

            processInst(inst, targetOp);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }
    }

    void processModule()
    {
        // Process ReinterpretOptional first since it may generate Reinterpret instructions
        processModuleForOp(kIROp_ReinterpretOptional);
    }

    void processReinterpretOptional(IRInst* inst)
    {
        // ReinterpretOptional reinterprets an optional<T> to optional<U> by:
        // 1. Checking if the source optional has a value
        // 2. If yes: extract the value, reinterpret it, and wrap it back
        // 3. If no: create a none of the destination type
        //
        // We create a helper function with proper if-else control flow,
        // because Select doesn't work on structured types.

        auto operand = inst->getOperand(0);
        auto srcOptionalType = as<IROptionalType>(operand->getDataType());
        auto destOptionalType = as<IROptionalType>(inst->getDataType());

        if (!srcOptionalType || !destOptionalType)
        {
            SLANG_UNEXPECTED("ReinterpretOptional called on non-optional types");
            return;
        }

        // Check if we already have a cached function for this type pair
        ReinterpretOptionalKey cacheKey{srcOptionalType, destOptionalType};
        IRFunc* func = nullptr;
        if (reinterpretOptionalFuncCache.tryGetValue(cacheKey, func))
        {
            // Reuse cached function
            IRBuilder builder(module);
            builder.setInsertBefore(inst);
            auto callResult = builder.emitCallInst(destOptionalType, func, 1, &operand);
            inst->replaceUsesWith(callResult);
            inst->removeAndDeallocate();
            return;
        }

        auto destValueType = destOptionalType->getValueType();

        // Create a helper function that performs the reinterpretation
        IRBuilder builder(module);
        builder.setInsertBefore(inst);

        func = builder.createFunc();
        builder.addNameHintDecoration(func, UnownedStringSlice("reinterpretOptional"));

        IRType* funcParamTypes[] = {srcOptionalType};
        auto funcType = builder.getFuncType(1, funcParamTypes, destOptionalType);
        func->setFullType(funcType);

        builder.setInsertInto(func);

        // Entry block
        auto entryBlock = builder.emitBlock();
        auto param = builder.emitParam(srcOptionalType);

        // Check if the source optional has a value
        auto hasValue = builder.emitOptionalHasValue(param);

        // Create the if-else control flow blocks
        auto trueBlock = builder.emitBlock();
        auto falseBlock = builder.emitBlock();
        auto unreachableBlock = builder.emitBlock();

        // Go back to entry block to emit the branch
        builder.setInsertInto(entryBlock);
        builder.emitIfElse(hasValue, trueBlock, falseBlock, unreachableBlock);

        // True branch: extract, reinterpret, and wrap
        builder.setInsertInto(trueBlock);
        auto extractedValue = builder.emitGetOptionalValue(param);
        auto reinterpretedValue = builder.emitReinterpret(destValueType, extractedValue);
        auto wrappedValue = builder.emitMakeOptionalValue(destOptionalType, reinterpretedValue);
        builder.emitReturn(wrappedValue);

        // False branch: create none and return
        builder.setInsertInto(falseBlock);
        auto noneValue = builder.emitMakeOptionalNone(destOptionalType);
        builder.emitReturn(noneValue);

        // Unreachable block (both branches return, so this is never reached)
        builder.setInsertInto(unreachableBlock);
        builder.emitUnreachable();

        // Cache the function for future use
        reinterpretOptionalFuncCache[cacheKey] = func;

        // Replace the ReinterpretOptional instruction with a call to the helper function
        builder.setInsertBefore(inst);
        auto callResult = builder.emitCallInst(destOptionalType, func, 1, &operand);
        inst->replaceUsesWith(callResult);
        inst->removeAndDeallocate();
    }
};

void lowerReinterpretOptional(IRModule* module, TargetProgram* target, DiagnosticSink* sink)
{
    ReinterpretOptionalLoweringContext context;
    context.module = module;
    context.targetProgram = target;
    context.sink = sink;
    context.processModule();
}

void lowerReinterpret(IRModule* module, TargetProgram* target, DiagnosticSink* sink)
{
    ReinterpretLoweringContext context;
    context.module = module;
    context.targetProgram = target;
    context.sink = sink;
    context.processModule();
}

} // namespace Slang
