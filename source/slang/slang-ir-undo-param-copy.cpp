#include "slang-ir-undo-param-copy.h"

#include "slang-ir-dce.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

#include <stdio.h> // For fprintf

namespace Slang
{
// This pass transforms variables decorated with TempCallArgVarDecoration
// by replacing them with direct references to the original parameters.
// This is important for CUDA/OptiX targets where functions like 'IgnoreHit'
// can prevent copy-back operations from executing.
struct UndoParameterCopyVisitor
{
    IRBuilder builder;
    IRModule* module;
    bool changed = false;

    UndoParameterCopyVisitor(IRModule* module)
        : module(module)
    {
        builder.setInsertInto(module);
    }

    // Map from temporary variables (IRVar) to original parameter pointers
    Dictionary<IRInst*, IRInst*> tempVarToOriginalParamPtrMap;

    // Track instructions to remove (temp vars and their initializing stores)
    List<IRInst*> instsToRemove;

    // Process the entire module
    void processModule()
    {
        // Process all functions in the module
        for (auto inst = module->getModuleInst()->getFirstChild(); inst; inst = inst->getNextInst())
        {
            if (auto func = as<IRFunc>(inst))
            {
                processFunc(func);
            }
        }
    }

    // Process a single function
    void processFunc(IRFunc* func)
    {
        tempVarToOriginalParamPtrMap.clear();
        instsToRemove.clear();

        // Pass 1: Identify TempCallArgVars and map them to their original parameter\'s pointer.
        // Also, collect the temp var and its initializing store for removal.
        for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
        {
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                if (auto varInst = as<IRVar>(inst))
                {
                    if (varInst->findDecoration<IRTempCallArgVarDecoration>())
                    {
                        IRStore* initializingStore = nullptr;
                        IRInst* originalParamPtr = nullptr;

                        // Scan for the store that initializes this varInst
                        // This store should be in the same block, after varInst.
                        // The value stored should be an IRLoad from the original parameter pointer.
                        for (auto scanInst = varInst->getNextInst(); scanInst;
                             scanInst = scanInst->getNextInst())
                        {
                            if (auto storeInst = as<IRStore>(scanInst))
                            {
                                if (storeInst->getPtr() == varInst)
                                {
                                    initializingStore = storeInst;

                                    if (auto loadInst = as<IRLoad>(storeInst->getVal()))
                                    {
                                        originalParamPtr = loadInst->getPtr();
                                        tempVarToOriginalParamPtrMap[varInst] = originalParamPtr;
                                        instsToRemove.add(initializingStore);
                                        instsToRemove.add(varInst);
                                    }

                                    break; // Found the initializing store for varInst
                                }
                            }
                            // Stop if we see another var declaration, or a call instruction,
                            // before finding the specific initializing store for varInst,
                            // then varInst is likely not following the simple
                            // copy-in pattern this pass targets, or the store is not immediately
                            // after it. Stop scanning for this varInst's initializer.
                            if (as<IRVar>(scanInst) || as<IRCall>(scanInst))
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (tempVarToOriginalParamPtrMap.getCount() == 0)
        {
            return;
        }

        changed = true;
        // Pass 2: Replace uses of temp vars with their original parameter pointers.
        for (auto& pair : tempVarToOriginalParamPtrMap)
        {
            IRInst* tempVar = pair.first;
            IRInst* originalParamPointer = pair.second;
            tempVar->replaceUsesWith(originalParamPointer);
        }

        // Pass 3: Remove the temp vars and their initializing stores.
        for (auto& inst : instsToRemove)
        {
            if (inst->getParent())
            {
                inst->removeAndDeallocate();
            }
        }
    }
};

void undoParameterCopy(IRModule* module)
{
    UndoParameterCopyVisitor visitor(module);
    visitor.processModule();

    // Run DCE to clean up any dead instructions
    if (visitor.changed)
    {
        eliminateDeadCode(module);
    }
}
} // namespace Slang
