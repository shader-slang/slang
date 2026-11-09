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

    // Backward contexts keep out None slots until optional evidence is specialized. None has
    // no storage, so erase its output slots along with ordinary void parameters and arguments.
    bool isVoidParameterType(IRType* type)
    {
        if (auto outputType = as<IROutParamType>(type))
            type = outputType->getValueType();
        return as<IRVoidType>(type) != nullptr;
    }

    // Remove call arguments before rewriting any function types. Consider a context call that
    // passes a global None variable to an out None parameter: the argument is a pointer, so its
    // value type alone cannot identify the slot. The original signature owns that information.
    void removeVoidCallArguments(IRCall* call)
    {
        auto funcType = cast<IRFuncType>(call->getCallee()->getDataType());
        List<IRInst*> args;
        for (UInt i = 0; i < call->getArgCount(); ++i)
        {
            if (i < funcType->getParamCount() && isVoidParameterType(funcType->getParamType(i)))
                continue;
            auto arg = call->getArg(i);
            if (!as<IRVoidType>(arg->getDataType()))
                args.add(arg);
        }
        if (args.getCount() == Index(call->getArgCount()))
            return;
        IRBuilder builder(module);
        builder.setInsertBefore(call);
        auto newCall = builder.emitCallInst(
            call->getFullType(),
            call->getCallee(),
            args.getCount(),
            args.getBuffer());
        call->transferDecorationsTo(newCall);
        newCall->sourceLoc = call->sourceLoc;
        call->replaceUsesWith(newCall);
        call->removeAndDeallocate();
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
                    bool needToRemove = isVoidParameterType(param->getDataType());
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
        case kIROp_Var:
        case kIROp_GlobalVar:
        case kIROp_FieldAddress:
            {
                // A var's type points to its stored value. This removes storage for None, while
                // retaining a variable that stores an ordinary opaque void pointer
                // (ptr<ptr<void>>).
                if (as<IRVoidType>(cast<IRPtrTypeBase>(inst->getDataType())->getValueType()))
                {
                    IRBuilder builder(module);
                    builder.setInsertBefore(inst);
                    inst->replaceUsesWith(builder.getVoidValue());
                    m_instsToRemove.add(inst);
                }
            }
            break;
        case kIROp_Store:
            if (as<IRVoidType>(as<IRStore>(inst)->getVal()->getDataType()))
                m_instsToRemove.add(inst);
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
                // Walk only the actual parameter types (the
                // `IRFuncType` accessors skip a trailing `IRAttr`
                // operand such as `IRFuncThrowTypeAttr` that
                // attributed function types carry). Reading raw
                // operands here would mistake the attribute for a
                // parameter and rebuild the type without it.
                List<IRType*> newParamTypes;
                for (auto paramType : funcType->getParamTypes())
                {
                    if (isVoidParameterType(paramType))
                    {
                        continue;
                    }
                    newParamTypes.add(paramType);
                }
                if (newParamTypes.getCount() != (Index)funcType->getParamCount())
                {
                    IRBuilder builder(module);
                    builder.setInsertBefore(funcType);
                    auto attr = funcType->getAttr();
                    auto newFuncType = attr ? builder.getFuncType(
                                                  newParamTypes.getCount(),
                                                  newParamTypes.getBuffer(),
                                                  funcType->getResultType(),
                                                  attr)
                                            : builder.getFuncType(
                                                  newParamTypes.getCount(),
                                                  newParamTypes.getBuffer(),
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
        case kIROp_DefaultConstruct:
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
                if (inst->getOp() == kIROp_Load || inst->getOp() == kIROp_DefaultConstruct)
                    m_instsToRemove.add(inst);
            }
        }
    }

    void processModule()
    {
        // Preserve the original parameter roles until every call has been rewritten.
        addToWorkList(module->getModuleInst());
        while (workList.getCount() != 0)
        {
            auto inst = workList.getLast();
            workList.removeLast();
            workListSet.remove(inst);
            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                addToWorkList(child);
            if (auto call = as<IRCall>(inst))
                removeVoidCallArguments(call);
        }

        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.remove(inst);

            processInst(inst);

            if (m_instsToRemove.contains(inst))
                continue;

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
