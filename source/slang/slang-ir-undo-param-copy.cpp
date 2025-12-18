#include "slang-ir-undo-param-copy.h"

#include "slang-ir-dce.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
// This pass transforms variables decorated with TempCallArgImmutableVarDecoration
// by replacing them with direct references to the original parameters.
// This is important for CUDA/OptiX targets where functions like 'IgnoreHit'
// can prevent copy-back operations from executing.
struct UndoParameterCopyVisitor
{
    IRBuilder builder;
    IRModule* module;
    bool changed = false;

    // Track instructions to remove
    List<IRInst*> instsToRemove;

    UndoParameterCopyVisitor(IRModule* module)
        : module(module)
    {
        builder.setInsertInto(module);
    }

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
        instsToRemove.clear();
        HashSet<IRInst*> originalPtrsForCopyBackCandidates; // Tracks original params that might
                                                            // have a redundant copy-back

        // Single pass to identify temps, replace uses, and identify redundant copy-back stores.
        for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
        {
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                if (auto varInst = as<IRVar>(inst))
                {
                    bool isTempCallArgVar = false;
                    for (auto decor : varInst->getDecorations())
                    {
                        if (as<IRTempCallArgImmutableVarDecoration>(decor) ||
                            as<IRTempCallArgVarDecoration>(decor))
                        {
                            isTempCallArgVar = true;
                            break;
                        }
                    }
                    if (isTempCallArgVar)
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

                                        // Found the pattern: var, store(var, load(originalParam))
                                        this->changed = true;

                                        // Replace uses of varInst with originalParamPtr immediately
                                        varInst->replaceUsesWith(originalParamPtr);

                                        // Mark for removal
                                        instsToRemove.add(initializingStore);
                                        instsToRemove.add(varInst);

                                        // Record originalParamPtr for copy-back optimization check
                                        originalPtrsForCopyBackCandidates.add(originalParamPtr);
                                    }
                                    break; // Found the initializing store for varInst
                                }
                            }
                            // Stop scanning if another var declaration or a call is encountered
                            if (as<IRVar>(scanInst) || as<IRCall>(scanInst))
                            {
                                break;
                            }
                        }
                    }
                }
                else if (auto storeInst = as<IRStore>(inst))
                {
                    // Check for redundant copy-back: store(originalParam, load(originalParam))
                    IRInst* destPtr = storeInst->getPtr();
                    if (originalPtrsForCopyBackCandidates.contains(destPtr))
                    {
                        if (auto loadVal = as<IRLoad>(storeInst->getVal()))
                        {
                            if (loadVal->getPtr() == destPtr)
                            {
                                // This is a redundant copy-back store
                                instsToRemove.add(storeInst);
                                this->changed = true;
                            }
                        }
                    }
                }
            }
        }

        // Removal pass
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
