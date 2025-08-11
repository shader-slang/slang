// slang-check-out-of-bound-access.cpp
#include "slang-check-out-of-bound-access.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

struct OutOfBoundAccessChecker : public InstPassBase
{
    DiagnosticSink* sink;

    OutOfBoundAccessChecker(IRModule* inModule, DiagnosticSink* inSink)
        : InstPassBase(inModule), sink(inSink)
    {
    }


    void checkArrayAccess(IRInst* inst, IRInst* base, IRInst* index)
    {
        // Check if index is a constant integer
        auto indexLit = as<IRIntLit>(index);
        if (!indexLit)
            return; // Skip non-constant indices

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
            return; // Skip non-array types

        // Check if array size is a constant
        auto arraySizeInst = arrayType->getElementCount();
        auto arraySizeLit = as<IRIntLit>(arraySizeInst);
        if (!arraySizeLit)
            return; // Skip arrays with non-constant size

        // Get the actual values
        IRIntegerValue indexValue = indexLit->getValue();
        IRIntegerValue arraySizeValue = arraySizeLit->getValue();

        // Check bounds: index should be >= 0 and < arraySize
        if (indexValue < 0 || indexValue >= arraySizeValue)
        {
            sink->diagnose(inst, Diagnostics::arrayIndexOutOfBounds, indexValue, arraySizeValue);
        }
    }

    void processModule()
    {
        processAllInsts(
            [&](IRInst* inst)
            {
                switch (inst->getOp())
                {
                case kIROp_GetElement:
                case kIROp_GetElementPtr:
                    {
                        if (inst->getOperandCount() < 2)
                            return;

                        auto base = inst->getOperand(0);
                        auto index = inst->getOperand(1);

                        checkArrayAccess(inst, base, index);
                    }
                    break;
                }
            });
    }
};

void checkForOutOfBoundAccess(IRModule* module, DiagnosticSink* sink)
{
    OutOfBoundAccessChecker checker(module, sink);
    checker.processModule();
}

} // namespace Slang