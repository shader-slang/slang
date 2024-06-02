// slang-ir-operator-shift-overflow.cpp
#include "slang-ir-operator-shift-overflow.h"

#include "../../slang.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"

namespace Slang {

    class DiagnosticSink;
    struct IRModule;

    //struct 
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
                    IRType* lhsType = lhs->getDataType();

                    IRIntegerValue sizeofLhs = 1;
                    switch (lhsType->getOp())
                    {
                    case kIROp_BoolType:
                    case kIROp_Int8Type:
                    case kIROp_UInt8Type:
                    case kIROp_CharType:
                        sizeofLhs = 1;
                        break;

                    case kIROp_Int16Type:
                    case kIROp_UInt16Type:
                        sizeofLhs = 2;
                        break;

                    case kIROp_IntType:
                    case kIROp_UIntType:
                        sizeofLhs = 4;
                        break;

                    case kIROp_Int64Type:
                    case kIROp_UInt64Type:
                        sizeofLhs = 8;
                        break;

                    case kIROp_IntPtrType:
                    case kIROp_UIntPtrType:
                        sizeofLhs = 8;
                        break;
                    }

                    if (sizeofLhs * 8 <= shiftAmount)
                    {
                        (void)lhsType;
                        printf("JKWAK found 'operator<<' shifting by %" PRId64 "\n", shiftAmount);
                        sink->diagnose(opShiftLeft, Diagnostics::operatorShiftLeftOverflow, lhsType, shiftAmount);
                    }
                }
            }
        }

        for (auto childInst : inst->getChildren())
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
