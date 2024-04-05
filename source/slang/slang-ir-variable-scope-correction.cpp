#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-ir-clone.h"

#include "slang-ir-dominators.h"
#include "slang-ir-variable-scope-correction.h"
#include "slang-ir-util.h"

namespace Slang
{

namespace { // anonymous
struct VariableScopeCorrectionContext
{
    VariableScopeCorrectionContext(IRModule* module):
        m_module(module), m_builder(module)
    {
    }

    void processModule();

    /// Process a function in the module
    void _processFunction(IRFunc* funcInst);
    void _processInstructions(IRFunc* funcInst, const Dictionary<IRInst*, List<IRInst*>>& workListMap);
    void _processStorableInst(IRInst* insertLoc, IRInst* inst, IRInst* user);
    void _processUnstorableInst(IRInst* funcInst, IRInst* inst, IRInst* user);
    void _replaceOperand(IRInst* inst, IRInst* oldOperand, IRInst* newOperand);
    bool _isStorableInst(IRInst* inst);

    IRModule* m_module;
    IRBuilder m_builder;
};

void VariableScopeCorrectionContext::processModule()
{
    IRModuleInst* moduleInst = m_module->getModuleInst();
    for (IRInst* child : moduleInst->getChildren())
    {
        // We want to find all of the functions, and process them
        if (auto funcInst = as<IRFunc>(child))
        {
            if (funcInst->getFirstBlock())
            {
                _processFunction(funcInst);
            }
        }
    }
}

static void debugPrint(uint32_t indent, IRInst* inst, const char* message, bool enable)
{
    if (!enable)
        return;

    uint32_t debugid = 0;
    for (uint32_t i = 0; i < indent; i++)
    {
        printf("    ");
    }

    if (inst) {
#ifdef SLANG_ENABLE_IR_BREAK_ALLOC
        debugid = inst->_debugUID;
#endif
    }

    if (debugid != 0)
        printf("%s, id: %d\n", message, debugid);
    else
        printf("\n");
}

void VariableScopeCorrectionContext::_processFunction(IRFunc* funcInst)
{
    IRDominatorTree* dominatorTree = m_module->findOrCreateDominatorTree(funcInst);
    Dictionary<IRInst*, List<IRInst*>> workListMap;
    Dictionary<IRBlock*, List<IRLoop*>> loopHeaderMap;

    bool enableLog = false;

    // traverse all blocks in the function
    for (auto block = funcInst->getFirstBlock(); block; block = block->getNextBlock())
    {
        uint32_t indent = 0;
        debugPrint(indent++, block, "processing block", enableLog);

        // Traverse all the dominators of a given block to check whether this given block is in a loop region.
        // Loop region blocks are the blocks that are dominated by the loop header block
        // but not dominated by the loop break block.
        auto dominatorBlock = dominatorTree->getImmediateDominator(block);
        List<IRLoop*> loopHeaderList;
        for (; dominatorBlock; dominatorBlock = dominatorTree->getImmediateDominator(dominatorBlock))
        {
            debugPrint(indent++, dominatorBlock, "dominator block", enableLog);

            // Find if the block is loop header block
            if (auto loopHeader = as<IRLoop>(dominatorBlock->getTerminator()))
            {
                debugPrint(indent, loopHeader, "loop header", enableLog);
                // Get the break block of the loop and check if such block
                auto breakBlock = loopHeader->getBreakBlock();
                debugPrint(indent++, breakBlock, "break block", enableLog);

                // Check if the current block is dominated by the break block. If so, it means that the block is in the loop region.
                if (!dominatorTree->dominates(breakBlock, block))
                {
                    debugPrint(indent++, block, "block is not dominated by break block", enableLog);
                    loopHeaderList.add(loopHeader);
                }
            }
        }
        loopHeaderMap.add(block, loopHeaderList);
    }

    if (loopHeaderMap.getCount() == 0)
    {
        return;
    }

    debugPrint(0, nullptr, "", enableLog);
    // Traverse all the instructions in function.
    for (auto block = funcInst->getFirstBlock(); block; block = block->getNextBlock())
    {
        if(auto loopHeaderList = loopHeaderMap.tryGetValue(block))
        {
            for (auto inst = block->getFirstChild(); inst; inst = inst->getNextInst())
            {
                List<IRInst*> instList;
                // Don't process the variable declaration instruction because the code is not emitted for them unless there is a use.
                if (inst->getOp() == kIROp_Var)
                {
                    continue;
                }
                // traverse all uses of this instruction
                debugPrint(0, getBlock(inst), "inst is defined in block", enableLog);
                for (auto use = inst->firstUse; use; use=use->nextUse)
                {
                    uint32_t indent = 0;
                    debugPrint(indent++, getBlock(use->getUser()), "inst's use block is", enableLog);
                    if (auto userBlock = getBlock(use->getUser()))
                    {
                        // If the use site of this instruction is dominated by the break block, it means that the
                        // instruction is used after the break block, so we need to make that instruction available globally.
                        // By doing so, we record all the users of this instructions.
                        for(auto loopHeader : *loopHeaderList)
                        {
                            auto breakBlock = loopHeader->getBreakBlock();
                            if (dominatorTree->dominates(breakBlock, userBlock))
                            {
                                debugPrint(indent, block, "inst is defined in loop but used outside of loop", enableLog);
                                instList.add(use->getUser());
                                break;
                            }
                        }
                    }
                }
                if (instList.getCount() > 0)
                {
                    workListMap.add(inst, instList);
                }
            }
        }
    }

    if (workListMap.getCount() == 0)
    {
        return;
    }

    debugPrint(0, nullptr, "", enableLog);
    _processInstructions(funcInst, workListMap);
}

void VariableScopeCorrectionContext::_processInstructions(IRFunc* funcInst, const Dictionary<IRInst*, List<IRInst*>>& workListMap)
{
    bool enableLog = false;

    // The first instruction in the function is the parameter list, so we need to skip it.
    auto instAfterParam = funcInst->getFirstBlock()->getFirstChild();
    while(instAfterParam->getOp() == kIROp_Param)
    {
        instAfterParam = instAfterParam->getNextInst();
    }

    for(auto it = workListMap.begin(); it != workListMap.end(); it++)
    {
        auto inst = it->first;
        auto list = it->second;
        debugPrint(0, inst, "inst", enableLog);
        bool isStoable = _isStorableInst(inst);

        for(auto user : list)
        {
            debugPrint(1, user, "The user", enableLog);
            if(isStoable)
            {
                _processStorableInst(instAfterParam, inst, user);
            }
            else
            {
                _processUnstorableInst(instAfterParam, inst, user);
            }
        }
    }
}

// For unstorable instructions, we need to duplicate the instruction right before the use site.
// For the operands of the instruction, if the operand is in the same block as the instruction,
// we need to declare a new variable at the beginning of the function
void VariableScopeCorrectionContext::_processUnstorableInst(IRInst* insertLoc, IRInst* inst, IRInst* user)
{
    IRCloneEnv cloneEnv;
    m_builder.setInsertBefore(user);

    // duplicate the invisible instruction and insert it right before the use site
    auto clonedInst = cloneInst(&cloneEnv, &m_builder, inst);
    clonedInst->insertAt(IRInsertLoc::before(user));

    // take care the operands of the duplicated instruction because they could also be invisible at use site
    SlangUInt operandCount = inst->getOperandCount();
    for (SlangUInt i = 0; i < operandCount; i++)
    {
        auto operand = inst->getOperand(i);

        // if the operand is the same block as the instruction, we need special handling for the operand
        // because the operand is still not available in the block where the instruction is used.
        if (getBlock(operand) == getBlock(inst))
        {
            // 1. Add a store instruction after this operand, e.g.: store(dstPtr, operand)
            // 2. Declare the dstPtr at beginning of the function, so it's globally available
            // 3. Replace the operand with the dstPtr
            if (auto type = operand->getDataType())
            {
                m_builder.setInsertBefore(insertLoc);
                auto dstPtr = m_builder.emitVar(type);

                m_builder.setInsertAfter(operand);
                m_builder.emitStore(dstPtr, operand);

                clonedInst->setOperand(i, dstPtr);
            }
        }
    }

    // last, because the user is still referencing the original instruction, we need to
    // replace operands in the use site instruction with the cloned instruction
    _replaceOperand(user, inst, clonedInst);
}

// For storable instructions, we don't want to duplicate it at use site, so we just need to store the result after the instruction
void VariableScopeCorrectionContext::_processStorableInst(IRInst* insertLoc, IRInst* inst, IRInst* user)
{
    auto type = inst->getDataType();
    // store instruction must have a result type
    SLANG_ASSERT(type);

    // declare a new variable at the beginning of the function used to store the result of the instruction
    m_builder.setInsertBefore(insertLoc);
    auto dstPtr = m_builder.emitVar(type);

    // insert a store instruction after the instruction
    m_builder.setInsertAfter(inst);
    m_builder.emitStore(dstPtr, inst);

    // last, replace operands in the use site instruction with the new variable
    // Note, because "dstPtr" is a pointer type, we have to insert a load(dstPtr) instruction before use it.
    // Simply replace any operand with pointer could generate error code.
    m_builder.setInsertBefore(user);
    auto loadInst = m_builder.emitLoad(dstPtr->getDataType(), dstPtr);
    _replaceOperand(user, inst,  loadInst);
}

void  VariableScopeCorrectionContext::_replaceOperand(IRInst* inst, IRInst* oldOperand, IRInst* newOperand)
{
    // TODO: I'd like to use "user->replaceUsesWith(clonedInst);" here, but it seems not replace the operands at all.
    SlangUInt operandCount = inst->getOperandCount();

    // Traverse all the operands to find out which one is the invisible instruction, and replace it with the new one.
    for (SlangUInt i = 0; i < operandCount; i++)
    {
        auto operand = inst->getOperand(i);

        if (operand == oldOperand)
        {
            inst->setOperand(i, newOperand);
        }
    }
}

bool VariableScopeCorrectionContext::_isStorableInst(IRInst* inst)
{
    auto type = inst->getDataType();

    // If the instruction has a result, and the result is not a void type, we consider it as a storable instruction
    if (type)
    {
        // Take care of pointer type, because we can't store a pointer type, so pointer type is regarded as a non-storable instruction
        if (type->getOp() == kIROp_PtrType)
        {
            return false;
        }
        return true;
    }
    return false;
}

} // anonymous

void applyVariableScopeCorrection(IRModule* module)
{
    VariableScopeCorrectionContext context(module);

    context.processModule();
}

} // namespace Slang

