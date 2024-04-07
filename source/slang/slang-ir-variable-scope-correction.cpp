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
    IRInst* _processInstruction(IRDominatorTree* dominatorTree, IRInst* instAfterParam,
            IRInst* originInst, IRInst* useInst, const List<IRLoop*>& loopHeaderList, const HashSet<IRInst*> workList);
    IRInst* _processUnstorableInst(IRInst* inst, IRInst* user);
    IRInst* _processStorableInst(IRInst* insertLoc, IRInst* inst, IRInst* user);

    void _replaceOperand(IRInst* inst, IRInst* oldOperand, IRInst* newOperand);
    bool _isStorableInst(IRInst* inst);
    bool _isOutOfScopeUse(IRInst* inst, IRDominatorTree* domTree, const List<IRLoop*>& loopHeaderList);

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

    HashSet<IRInst*> workList;

    Dictionary<IRBlock*, List<IRLoop*>> loopHeaderMap;

    bool enableLog = false;

    // traverse all blocks in the function
    for (auto block : funcInst->getBlocks())
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
    for (auto block : funcInst->getBlocks())
    {
        if(loopHeaderMap.containsKey(block))
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
                workList.add(inst);
            }
        }
    }

    auto instAfterParam = funcInst->getFirstBlock()->getFirstChild();
    while(instAfterParam->getOp() == kIROp_Param)
    {
        instAfterParam = instAfterParam->getNextInst();
    }

    // for(auto inst: workList)
    for(auto it = workList.begin(); it != workList.end(); it++)
    {
        auto inst = *it;
        if (auto loopHeaderList = loopHeaderMap.tryGetValue(getBlock(inst)))
        {
            for (auto use = inst->firstUse; use; use=use->nextUse)
            {
                _processInstruction(dominatorTree, instAfterParam, inst, use->getUser(), *loopHeaderList, workList);
            }
        }
    }


    debugPrint(0, nullptr, "", enableLog);
}

// Check if the instruction is used outside of the loop.
// The loopHeaderList contains all the loop headers where the original instruction is defined.
// So we if the block of the user instruction is dominated by the break block of the loop header,
// it means that it was out of the loop, so it's out of the scope of the loop.
// Note the reason we use the loopHeaderList is because there could be nested loops, so we need to
// check all the loop headers from inner to outer.
bool VariableScopeCorrectionContext::_isOutOfScopeUse(IRInst * userInst, IRDominatorTree* domTree, const List<IRLoop*>& loopHeaderList)
{
    if (auto block = getBlock(userInst))
    {
        // If the use site of this instruction is dominated by the break block, it means that the
        // instruction is used after the break block, so we need to make that instruction available globally.
        // By doing so, we record all the users of this instructions.
        for(auto loopHeader : loopHeaderList)
        {
            auto breakBlock = loopHeader->getBreakBlock();
            if (domTree->dominates(breakBlock, block))
            {
                return true;
            }
        }
    }
    return false;
}

IRInst* VariableScopeCorrectionContext::_processInstruction(IRDominatorTree* dominatorTree, IRInst* instAfterParam,
        IRInst* originInst, IRInst* useInst, const List<IRLoop*>& loopHeaderList, const HashSet<IRInst*> workList)
{
    // For a given instruction, we need to check all the users of this instruction to see
    // if the user is out of the scope of the loop. If so, we need to make the instruction available globally.
    // In addition, we have to recursively check all the operands of this instruction to see if they are also
    // out of the scope of the loop.

    // Check if the user of this instruction is out of the scope of the loop
    if(_isOutOfScopeUse(useInst, dominatorTree, loopHeaderList))
    {
        bool isStoable = _isStorableInst(originInst);
        IRInst* newInst = nullptr;
        if(isStoable)
        {
            newInst = _processStorableInst(instAfterParam, originInst, useInst);
        }
        else
        {
            // duplicate the instruction right before the use site.
            newInst = _processUnstorableInst(originInst, useInst);
            // When we duplicate the out-of-scope instruction, the side-effect is that the operands of the instruction
            // could also be out of the scope. So we have to recursively check all the operands of the instruction. Such
            // issue dooesn't happen for the storable instruction because we store the result of the instruction in a new
            // variable visible in the whole function.
            UInt operandCount = newInst->getOperandCount();
            for (UInt i = 0; i < operandCount; i++)
            {
                auto operand = newInst->getOperand(i);

                // workList contains all the instructions in the loop region, so if the operands in the list, then we have to
                // handle the operand as well
                if (workList.contains(operand))
                {
                    // Recursively check all the operands
                    // Now the newly inserted inst is the user of the operand, so we pass 'operand' as the origin instruction, and
                    // 'newInst' as the user instruction.
                    if (auto newOperand = _processInstruction(dominatorTree, instAfterParam, operand, newInst, loopHeaderList, workList))
                    {
                        // Replace the operand with the new instruction
                        newInst->setOperand(i, newOperand);
                    }
                }
            }
        }
        // Replace the operands of user with the new instruction
        _replaceOperand(useInst, originInst, newInst);
        return newInst;
    }
    return nullptr;
}

// 1. Duplicate the original instruction
// 2. Insert the duplicated instruction right before the use site
// 3. return the load instruction
IRInst* VariableScopeCorrectionContext::_processUnstorableInst(IRInst* inst, IRInst* user)
{
    IRCloneEnv cloneEnv;
    m_builder.setInsertBefore(user);

    // duplicate the invisible instruction and insert it right before the use site
    auto clonedInst = cloneInst(&cloneEnv, &m_builder, inst);
    clonedInst->insertAt(IRInsertLoc::before(user));

    return clonedInst;
}

// 1. Declare a new variable at the beginning of the function
// 2. Insert a store instruction after the original instruction
// 3. Insert a load instruction before the use site
// 3. return the load instruction
IRInst* VariableScopeCorrectionContext::_processStorableInst(IRInst* insertLoc, IRInst* inst, IRInst* user)
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

    return loadInst;
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

