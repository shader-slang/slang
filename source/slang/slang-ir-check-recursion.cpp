#include "slang-ir-check-recursion.h"

#include "slang-ir-util.h"

namespace Slang
{
bool checkTypeRecursionImpl(
    HashSet<IRInst*>& checkedTypes,
    HashSet<IRInst*>& stack,
    IRInst* type,
    IRInst* field,
    DiagnosticSink* sink)
{
    auto visitElementType = [&](IRInst* elementType, IRInst* field) -> bool
    {
        if (!stack.add(elementType))
        {
            sink->diagnose(field ? field : type, Diagnostics::recursiveType, type);
            return false;
        }
        if (checkedTypes.add(elementType))
            checkTypeRecursionImpl(checkedTypes, stack, elementType, field, sink);
        stack.remove(elementType);
        return true;
    };
    if (auto arrayType = as<IRArrayTypeBase>(type))
    {
        return visitElementType(arrayType->getElementType(), field);
    }
    else if (auto structType = as<IRStructType>(type))
    {
        for (auto sfield : structType->getFields())
            if (!visitElementType(sfield->getFieldType(), sfield))
                return false;
    }
    return true;
}

void checkTypeRecursion(HashSet<IRInst*>& checkedTypes, IRInst* type, DiagnosticSink* sink)
{
    HashSet<IRInst*> stack;
    if (checkedTypes.add(type))
    {
        stack.add(type);
        checkTypeRecursionImpl(checkedTypes, stack, type, nullptr, sink);
    }
}

void checkForRecursiveTypes(IRModule* module, DiagnosticSink* sink)
{
    HashSet<IRInst*> checkedTypes;
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_StructType:
            {
                checkTypeRecursion(checkedTypes, globalInst, sink);
            }
            break;
        default:
            break;
        }
    }
}

bool checkFunctionRecursionImpl(
    HashSet<IRFunc*>& checkedFuncs,
    HashSet<IRFunc*>& callStack,
    IRFunc* func,
    DiagnosticSink* sink)
{
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            auto callInst = as<IRCall>(inst);
            if (!callInst)
                continue;
            auto callee = as<IRFunc>(callInst->getCallee());
            if (!callee)
                continue;
            if (!callStack.add(callee))
            {
                sink->diagnose(callInst, Diagnostics::unsupportedRecursion, callee);
                return false;
            }
            if (checkedFuncs.add(callee))
                checkFunctionRecursionImpl(checkedFuncs, callStack, callee, sink);
            callStack.remove(callee);
        }
    }
    return true;
}

void checkFunctionRecursion(HashSet<IRFunc*>& checkedFuncs, IRFunc* func, DiagnosticSink* sink)
{
    HashSet<IRFunc*> callStack;
    if (checkedFuncs.add(func))
    {
        callStack.add(func);
        checkFunctionRecursionImpl(checkedFuncs, callStack, func, sink);
    }
}

void checkForRecursiveFunctions(TargetRequest* target, IRModule* module, DiagnosticSink* sink)
{
    HashSet<IRFunc*> checkedFuncsForRecursionDetection;
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_Func:
            if (!isCPUTarget(target))
                checkFunctionRecursion(
                    checkedFuncsForRecursionDetection,
                    as<IRFunc>(globalInst),
                    sink);
            break;
        default:
            break;
        }
    }
}

void checkForOutOfBoundAccess(IRModule* module, DiagnosticSink* sink)
{
    // Iterate through all instructions in the IR module
    for (auto globalInst : module->getGlobalInsts())
    {
        // Check functions
        if (auto func = as<IRFunc>(globalInst))
        {
            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    // Check for GetElement and GetElementPtr instructions
                    if (inst->getOp() == kIROp_GetElement || inst->getOp() == kIROp_GetElementPtr)
                    {
                        if (inst->getOperandCount() < 2)
                            continue;

                        auto base = inst->getOperand(0);
                        auto index = inst->getOperand(1);

                        // Check if index is a constant integer
                        auto indexLit = as<IRIntLit>(index);
                        if (!indexLit)
                            continue; // Skip non-constant indices

                        // Get the base type
                        auto baseType = base->getDataType();

                        // Handle pointer-to-array case (for GetElementPtr)
                        if (auto ptrType = as<IRPtrTypeBase>(baseType))
                        {
                            baseType = ptrType->getValueType();
                        }

                        // Check if base is an array type
                        auto arrayType = as<IRArrayTypeBase>(baseType);
                        if (!arrayType)
                            continue; // Skip non-array types

                        // Check if array size is a constant
                        auto arraySizeInst = arrayType->getElementCount();
                        auto arraySizeLit = as<IRIntLit>(arraySizeInst);
                        if (!arraySizeLit)
                            continue; // Skip arrays with non-constant size

                        // Get the actual values
                        IRIntegerValue indexValue = indexLit->getValue();
                        IRIntegerValue arraySizeValue = arraySizeLit->getValue();

                        // Check bounds: index should be >= 0 and < arraySize
                        if (indexValue < 0 || indexValue >= arraySizeValue)
                        {
                            sink->diagnose(
                                inst,
                                Diagnostics::arrayIndexOutOfBounds,
                                indexValue,
                                arraySizeValue);
                        }
                    }
                }
            }
        }
    }
}

} // namespace Slang
