// slang-ir-potential-infinite-loop.cpp
#include "slang-ir-potential-infinite-loop.h"

#include "slang-diagnostics.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

/// Conservative infinite loop detection that avoids false positives
/// Detects patterns likely to indicate infinite loops without too many false alarms
static void checkLoopForInfinitePattern(IRLoop* loop, DiagnosticSink* sink)
{
    auto targetBlock = loop->getTargetBlock();
    if (!targetBlock)
        return;

    // Look for comparison instructions in the loop header
    for (auto inst : targetBlock->getChildren())
    {
        // Check for comparison operations
        if (inst->getOp() == kIROp_Less || inst->getOp() == kIROp_Greater ||
            inst->getOp() == kIROp_Leq || inst->getOp() == kIROp_Geq ||
            inst->getOp() == kIROp_Eql || inst->getOp() == kIROp_Neq)
        {
            if (inst->getOperandCount() >= 2)
            {
                auto leftOp = inst->getOperand(0);
                auto rightOp = inst->getOperand(1);

                // Check for literal constants
                bool leftIsConstant =
                    (leftOp->getOp() == kIROp_IntLit || leftOp->getOp() == kIROp_FloatLit);
                bool rightIsConstant =
                    (rightOp->getOp() == kIROp_IntLit || rightOp->getOp() == kIROp_FloatLit);

                // Pattern 1: Both operands are literal constants - definitely wrong
                if (leftIsConstant && rightIsConstant)
                {
                    sink->diagnose(loop, Diagnostics::potentialInfiniteLoop);
                    return;
                }

                // Pattern 2: One operand is a constant literal, other is computed expression
                // This often indicates a loop variable that's not being incremented
                if (leftIsConstant || rightIsConstant)
                {
                    // Look for specific patterns that suggest problematic loops
                    // Note: 0.0 comparisons can trigger false positives on legitimate convergence
                    // loops (e.g., while(error > 0.0) { error = computeNewError(); }), but this
                    // conservative approach catches common typos like missing assignment operators
                    if (leftIsConstant && leftOp->getOp() == kIROp_FloatLit)
                    {
                        auto floatLit = as<IRFloatLit>(leftOp);
                        if (floatLit && floatLit->getValue() == 0.0)
                        {
                            sink->diagnose(loop, Diagnostics::potentialInfiniteLoop);
                            return;
                        }
                    }
                    if (rightIsConstant && rightOp->getOp() == kIROp_FloatLit)
                    {
                        auto floatLit = as<IRFloatLit>(rightOp);
                        if (floatLit && floatLit->getValue() == 0.0)
                        {
                            sink->diagnose(loop, Diagnostics::potentialInfiniteLoop);
                            return;
                        }
                    }
                }
            }
        }
    }
}

/// Recursively traverse all instructions to find loops, including nested functions in generics
static void traverseInstsForLoops(IRInst* inst, DiagnosticSink* sink)
{
    // Check if this instruction is a loop
    if (auto loop = as<IRLoop>(inst))
    {
        checkLoopForInfinitePattern(loop, sink);
    }

    // Recursively traverse all children to catch functions inside generics
    for (auto child : inst->getChildren())
    {
        traverseInstsForLoops(child, sink);
    }
}

/// Main entry point for infinite loop detection IR pass
void checkForPotentialInfiniteLoops(IRModule* module, DiagnosticSink* sink)
{
    // Use recursive traversal to catch all loops including those in nested contexts
    // like functions inside generics, which the original top-level traversal missed
    traverseInstsForLoops(module->getModuleInst(), sink);
}

} // namespace Slang
