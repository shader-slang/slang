// slang-ir-operator-shift-overflow.cpp
#include "slang-ir-operator-shift-overflow.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang {

    class DiagnosticSink;
    struct IRModule;

    void checkForOperatorShiftOverflowRecursive(
        IRInst* inst,
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

                    IRIntegerValue shiftAmount = rhsLit->getValue();

                    IRInst* lhs = opShiftLeft->getOperand(0);
                    IRUse lhsType = lhs->typeUse;

                    // TODO: Not sure how to get the size of the type.
                    {
                        (void)lhsType;
                        printf("JKWAK found 'operator<<' shifting by %" PRId64 "\n", shiftAmount);
                        //sink->diagnose(child, Diagnostics::operatorShiftLeftOverflow);
                    }
                }
            }
        }

        // TODO: Need to figure out if this can be just "inst->getChildren()"
        for (auto childInst : inst->getDecorationsAndChildren())
        {
            checkForOperatorShiftOverflowRecursive(childInst, sink);
        }
    }

    void checkForOperatorShiftOverflow(
        IRModule* module,
        DiagnosticSink* sink)
    {
        // Look for `operator<<` instructions
        checkForOperatorShiftOverflowRecursive(module->getModuleInst(), sink);
    }

}
