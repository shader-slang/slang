// slang-ir-operator-shift-overflow.cpp
#include "slang-ir-operator-shift-overflow.h"

#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang.h"

namespace Slang
{

class DiagnosticSink;
struct IRModule;

void checkForOperatorShiftOverflowRecursive(IRInst* inst, DiagnosticSink* sink)
{
    if (auto code = as<IRGlobalValueWithCode>(inst))
    {
        for (auto block : code->getBlocks())
        {
            for (auto opInst : block->getChildren())
            {
                switch (opInst->getOp())
                {
                case kIROp_Lsh:
                    {
                        SLANG_ASSERT(opInst->getOperandCount() == 2);

                        IRInst* lhs = opInst->getOperand(0);
                        IRType* lhsType = lhs->getDataType();

                        // For vector/matrix types, check the element type — not the aggregate size.
                        // e.g. uint8_t4 has 4-byte aggregate but 1-byte elements.
                        IRType* lhsElemType = lhsType;
                        if (auto vecType = as<IRVectorType>(lhsType))
                            lhsElemType = vecType->getElementType();
                        else if (auto matType = as<IRMatrixType>(lhsType))
                            lhsElemType = matType->getElementType();

                        IRSizeAndAlignment lhsSizeAlignment;
                        if (SLANG_FAILED(getNaturalSizeAndAlignment(
                                nullptr,
                                lhsElemType,
                                &lhsSizeAlignment)))
                            break;

                        IRInst* rhs = opInst->getOperand(1);
                        auto rhsLit = as<IRIntLit>(rhs);
                        if (rhsLit)
                        {
                            // For literal shift amounts, warn if the shift overflows the LHS type.
                            IRIntegerValue shiftAmount = rhsLit->getValue();
                            if (lhsSizeAlignment.size * 8 <= shiftAmount)
                            {
                                sink->diagnose(Diagnostics::OperatorShiftLeftOverflow{
                                    .lhsType = lhsType,
                                    .shiftAmount = shiftAmount,
                                    .location = opInst->sourceLoc,
                                });
                            }
                        }
                        else if (lhsSizeAlignment.size < 4)
                        {
                            // For non-literal shift amounts on a narrow LHS type (< 32 bits),
                            // warn only when the shift amount type is wider than the LHS. This
                            // targets the C/C++ integer-promotion surprise: `uint8_t x; x << n`
                            // keeps the result as uint8_t in Slang, not widened to int as in C.
                            IRType* rhsType = rhs->getDataType();
                            if (auto rhsVecType = as<IRVectorType>(rhsType))
                                rhsType = rhsVecType->getElementType();
                            else if (auto rhsMatType = as<IRMatrixType>(rhsType))
                                rhsType = rhsMatType->getElementType();
                            IRSizeAndAlignment rhsSizeAlignment;
                            if (SLANG_SUCCEEDED(getNaturalSizeAndAlignment(
                                    nullptr,
                                    rhsType,
                                    &rhsSizeAlignment)) &&
                                rhsSizeAlignment.size > lhsSizeAlignment.size)
                            {
                                sink->diagnose(Diagnostics::OperatorShiftOnNarrowType{
                                    .lhsType = lhsType,
                                    .location = opInst->sourceLoc,
                                });
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    for (auto childInst : inst->getChildren())
    {
        checkForOperatorShiftOverflowRecursive(childInst, sink);
    }
}

void checkForOperatorShiftOverflow(IRModule* module, DiagnosticSink* sink)
{
    // Look for `operator<<` instructions
    checkForOperatorShiftOverflowRecursive(module->getModuleInst(), sink);
}

} // namespace Slang
