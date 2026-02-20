// slang-ir-cleanup-void.cpp

#include "slang-ir-cleanup-void.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
struct CleanUpVoidContext
{
    IRModule* module;

    InstWorkList workList;
    InstHashSet workListSet;
    InstHashSet m_instsToRemove;
    // Maps IRDebugVar old argument index -> new argument index after IRFunc void-parameter removal.
    // A value of -1 means the corresponding IRFunc parameter was a VoidType and removed.
    Dictionary<IRFunc*, List<Index>> m_debugVarRemappedArgIndices;

    CleanUpVoidContext(IRModule* inModule)
        : module(inModule), workList(inModule), workListSet(inModule), m_instsToRemove(inModule)
    {
    }

    void addToWorkList(IRInst* inst)
    {
        for (auto ii = inst->getParent(); ii; ii = ii->getParent())
        {
            if (as<IRGeneric>(ii))
                return;
        }

        if (workListSet.contains(inst))
            return;

        workList.add(inst);
        workListSet.add(inst);
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_Call:
        case kIROp_MakeStruct:
            {
                // Remove void argument.
                List<IRInst*> newArgs;
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    auto arg = inst->getOperand(i);
                    if (arg->getDataType() && arg->getDataType()->getOp() == kIROp_VoidType)
                    {
                        continue;
                    }
                    newArgs.add(arg);
                }
                if (newArgs.getCount() != (Index)inst->getOperandCount())
                {
                    IRBuilder builder(module);
                    builder.setInsertBefore(inst);
                    auto newCall = builder.emitIntrinsicInst(
                        inst->getFullType(),
                        inst->getOp(),
                        newArgs.getCount(),
                        newArgs.getBuffer());
                    inst->replaceUsesWith(newCall);
                    inst->removeAndDeallocate();
                    inst = newCall;
                }
            }
            break;
        case kIROp_Func:
            {
                // Remove void parameters.
                // To keep the ArgIndex operand of the IRDebugVars
                // corresponding to the function parameters correct, we build a
                // map to update the IRDebugVar argument indices later.
                List<IRParam*> paramsToRemove;
                auto func = as<IRFunc>(inst);
                List<Index> argIndexRemap; // Indexed by old ArgIndex, with value of new ArgIndex
                argIndexRemap.reserve(func->getParamCount());
                Index nextKeptArgIndex = 0;
                for (auto param : func->getParams())
                {
                    bool needToRemove = (param->getDataType()->getOp() == kIROp_VoidType);
                    argIndexRemap.add(needToRemove ? -1 : nextKeptArgIndex);

                    if (needToRemove)
                    {
                        paramsToRemove.add(param);
                    }
                    else
                    {
                        nextKeptArgIndex++;
                    }
                }
                IRBuilder builder(module);
                builder.setInsertBefore(func);
                for (auto param : paramsToRemove)
                {
                    auto voidVal = builder.getVoidValue();
                    param->replaceUsesWith(voidVal);
                    param->removeAndDeallocate();
                }

                if (paramsToRemove.getCount() != 0)
                {
                    m_debugVarRemappedArgIndices.add(func, argIndexRemap);
                }
            }
            break;
        case kIROp_DebugValue:
            {
                // Remove void values.
                if (auto debugValue = as<IRDebugValue>(inst))
                {
                    if (auto valueType = debugValue->getValue()->getDataType())
                    {
                        if (valueType->getOp() == kIROp_VoidType)
                        {
                            m_instsToRemove.add(debugValue);
                        }
                    }
                }
            }
            break;
        case kIROp_DebugVar:
            {
                // 1. Update the DebugVar's ArgIndex operand or mark the
                // DebugVar for removal if it was a VoidType parameter when the
                // Func was processed.
                // 2. Detect VoidType DebugVars
                // 3. Remove VoidType DebugVars and any DebugValue users of the DebugVar
                auto debugVar = as<IRDebugVar>(inst);
                bool removeDebugVar = false;

                // DebugVar corresponding to a function parameter?
                if (auto argIndexInst = as<IRIntLit>(debugVar->getArgIndex()))
                {
                    if (auto parentFunc = getParentFunc(debugVar))
                    {
                        if (auto remappedArgIndices =
                                m_debugVarRemappedArgIndices.tryGetValue(parentFunc))
                        {
                            Index oldArgIndex = (Index)argIndexInst->getValue();
                            if (oldArgIndex >= 0 && oldArgIndex < remappedArgIndices->getCount())
                            {
                                Index newArgIndex = (*remappedArgIndices)[oldArgIndex];
                                if (newArgIndex < 0)
                                {
                                    removeDebugVar = true;
                                }
                                else if (newArgIndex != oldArgIndex)
                                {
                                    // Update the ArgIndex operand
                                    IRBuilder builder(module);
                                    builder.setInsertBefore(debugVar);
                                    debugVar->setArgIndex(
                                        builder.getIntValue(builder.getUIntType(), newArgIndex));
                                }
                            }
                        }
                    }
                }

                // Detect any VoidType DebugVars, e.g. from locally declared
                // variables
                auto debugVarType = debugVar->getDataType();
                while (auto ptrType = as<IRPtrTypeBase>(debugVarType))
                {
                    debugVarType = ptrType->getValueType();
                }
                if (debugVarType && debugVarType->getOp() == kIROp_VoidType)
                {
                    removeDebugVar = true;
                }

                // Remove the VoidType DebugVar and any DebugValue users of the DebugVar
                if (removeDebugVar)
                {
                    List<IRInst*> debugValueUsersToRemove;
                    bool hasOnlyDebugValueUsers =
                        true; // Keep the DebugVar if there are non-DebugValue users
                    for (auto use = debugVar->firstUse; use; use = use->nextUse)
                    {
                        if (auto debugValue = as<IRDebugValue>(use->getUser()))
                        {
                            debugValueUsersToRemove.add(debugValue);
                        }
                        else
                        {
                            hasOnlyDebugValueUsers = false;
                        }
                    }

                    for (auto debugValue : debugValueUsersToRemove)
                    {
                        m_instsToRemove.add(debugValue);
                    }
                    if (hasOnlyDebugValueUsers)
                    {
                        m_instsToRemove.add(debugVar);
                    }
                }
            }
            break;
        case kIROp_FuncType:
            {
                auto funcType = as<IRFuncType>(inst);
                List<IRInst*> newOperands;
                for (UInt i = 1; i < funcType->getOperandCount(); i++)
                {
                    auto operand = funcType->getOperand(i);
                    if (operand->getOp() == kIROp_VoidType)
                    {
                        continue;
                    }
                    newOperands.add(operand);
                }
                if (newOperands.getCount() != (Index)funcType->getParamCount())
                {
                    IRBuilder builder(module);
                    builder.setInsertBefore(funcType);
                    auto newFuncType = builder.getFuncType(
                        newOperands.getCount(),
                        (IRType**)newOperands.getBuffer(),
                        funcType->getResultType());
                    if (newFuncType != funcType)
                    {
                        funcType->replaceUsesWith(newFuncType);
                        funcType->removeAndDeallocate();
                    }
                    inst = newFuncType;
                }
            }
            break;
        case kIROp_StructType:
            {
                List<IRInst*> toRemove;
                for (auto child : inst->getChildren())
                {
                    if (auto field = as<IRStructField>(child))
                    {
                        if (field->getFieldType()->getOp() == kIROp_VoidType)
                        {
                            toRemove.add(field);
                        }
                    }
                }
                for (auto ii : toRemove)
                    ii->removeAndDeallocate();
            }
            break;
        default:
            break;
        }

        // If inst has void type, all uses of it should be replaced with void val.
        // We should do this only for a subset of opcodes known to be safe.
        switch (inst->getOp())
        {
        case kIROp_Load:
        case kIROp_GetElement:
        case kIROp_GetOptionalValue:
        case kIROp_FieldExtract:
        case kIROp_GetTupleElement:
        case kIROp_GetResultError:
        case kIROp_GetResultValue:
        case kIROp_Call:
        case kIROp_UpdateElement:
        case kIROp_GetTargetTupleElement:
            if (inst->getDataType()->getOp() == kIROp_VoidType)
            {
                IRBuilder builder(module);
                builder.setInsertBefore(inst);
                inst->replaceUsesWith(builder.getVoidValue());
            }
        }
    }

    void processModule()
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

        for (auto inst : m_instsToRemove.getHashSet())
        {
            inst->removeAndDeallocate();
        }
    }
};

void cleanUpVoidType(IRModule* module)
{
    CleanUpVoidContext context(module);
    context.processModule();
}
} // namespace Slang
