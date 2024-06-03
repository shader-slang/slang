// slang-ir-operator-shift-overflow.cpp
#include "slang-ir-operator-shift-overflow.h"

#include "../../slang.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"

namespace Slang {

    class DiagnosticSink;
    struct IRModule;

    void checkForOperatorShiftOverflowRecursive(
        IRInst* inst,
        CompilerOptionSet& optionSet,
        DiagnosticSink* sink)
    {
        if (auto code = as<IRGlobalValueWithCode>(inst))
        {
            for (auto block : code->getBlocks())
            {
                for (auto opShiftLeft : block->getChildren())
                {
                    if (opShiftLeft->getOp() != kIROp_Lsh)
                        continue;

                    SLANG_ASSERT(opShiftLeft->getOperandCount() == 2);

                    IRInst* rhs = opShiftLeft->getOperand(1);
                    auto rhsLit = as<IRIntLit>(rhs);
                    if (!rhsLit)
                        continue;

                    IRInst* lhs = opShiftLeft->getOperand(0);
                    IRType* lhsType = lhs->getDataType();

                    IRSizeAndAlignment sizeAlignment;
                    if (SLANG_FAILED(getNaturalSizeAndAlignment(optionSet, lhsType, &sizeAlignment)))
                        continue;

                    IRIntegerValue shiftAmount = rhsLit->getValue();
                    if (sizeAlignment.size * 8 <= shiftAmount)
                    {
                        sink->diagnose(opShiftLeft, Diagnostics::operatorShiftLeftOverflow, lhsType, shiftAmount);
                    }
                }
            }
        }

        for (auto childInst : inst->getChildren())
        {
            checkForOperatorShiftOverflowRecursive(childInst, optionSet, sink);
        }
    }

    void checkForOperatorShiftOverflow(
        IRModule* module,
        CompilerOptionSet& optionSet,
        DiagnosticSink* sink)
    {
        // Look for `operator<<` instructions
        checkForOperatorShiftOverflowRecursive(module->getModuleInst(), optionSet, sink);
    }

}
