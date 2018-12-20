// ir.cpp
#include "ir.h"
#include "ir-insts.h"

#include "../core/basic.h"
#include "../core/slang-cpu-defines.h"

#include "mangle.h"

namespace Slang
{
    struct IRSpecContext;

    IRInst* cloneGlobalValueWithLinkage(
        IRSpecContext*          context,
        IRInst*                 originalVal,
        IRLinkageDecoration*    originalLinkage);

    struct IROpMapEntry
    {
        IROp        op;
        IROpInfo    info;
    };

    // TODO: We should ideally be speeding up the name->inst
    // mapping by using a dictionary, or even by pre-computing
    // a hash table to be stored as a `static const` array.
    //
    // NOTE! That this array is now constructed in such a way that looking up 
    // an entry from an op is fast, by keeping blocks of main, and pseudo ops in same order
    // as the ops themselves. Care must be taken to keep this constraint.
    static const IROpMapEntry kIROps[] =
    {
        
    // Main ops in order
#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    { kIROp_##ID, { #MNEMONIC, ARG_COUNT, FLAGS, } },
#include "ir-inst-defs.h"

    // Pseudo ops
#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  /* empty */
#define PSEUDO_INST(ID)  \
    { kIRPseudoOp_##ID, { #ID, 0, 0 } },

    // First is 'invalid' 
    { kIROp_Invalid,{ "invalid", 0, 0 } },
    // Then all the other psuedo ops
#include "ir-inst-defs.h"

    };

    IROpInfo getIROpInfo(IROp opIn)
    {
        const int op = opIn & kIROpMeta_PseudoOpMask;
        if ((op & kIROpMeta_IsPseudoOp) && op < kIRPseudoOp_LastPlusOne)
        {
            // It's a pseudo op
            const int index = op - kIRPseudoOp_First;
            // Pseudo ops start from kIROpcount
            const auto& entry =  kIROps[kIROpCount + index];
            SLANG_ASSERT(entry.op == op);
            return entry.info;
        }
        else if (op < kIROpCount)
        {
            // It's a main op
            const auto& entry = kIROps[op];
            SLANG_ASSERT(entry.op == op);
            return entry.info;
        }

        // Don't know what this is
        SLANG_ASSERT(!"Invalid op");
        SLANG_ASSERT(kIROps[kIROpCount].op == kIROp_Invalid);
        return kIROps[kIROpCount].info;
    }

    IROp findIROp(const UnownedStringSlice& name)
    {
        for (auto ee : kIROps)
        {
            if (name == ee.info.name)
                return ee.op;
        }

        return IROp(kIROp_Invalid);
    }

    

    //

    void IRUse::debugValidate()
    {
#ifdef _DEBUG
        auto uv = this->usedValue;
        if(!uv)
        {
            assert(!nextUse);
            assert(!prevLink);
            return;
        }

        auto pp = &uv->firstUse;
        for(auto u = uv->firstUse; u;)
        {
            assert(u->prevLink == pp);

            pp = &u->nextUse;
            u = u->nextUse;
        }
#endif
    }

    void IRUse::init(IRInst* u, IRInst* v)
    {
        clear();

        user = u;
        usedValue = v;
        if(v)
        {
            nextUse = v->firstUse;
            prevLink = &v->firstUse;

            if(nextUse)
            {
                nextUse->prevLink = &this->nextUse;
            }

            v->firstUse = this;
        }

        debugValidate();
    }

    void IRUse::set(IRInst* uv)
    {
        init(user, uv);
    }

    void IRUse::clear()
    {
        // This `IRUse` is part of the linked list
        // of uses for  `usedValue`.

        debugValidate();

        if (usedValue)
        {
            auto uv = usedValue;

            *prevLink = nextUse;
            if(nextUse)
            {
                nextUse->prevLink = prevLink;
            }

            user        = nullptr;
            usedValue   = nullptr;
            nextUse     = nullptr;
            prevLink    = nullptr;

            if(uv->firstUse)
                uv->firstUse->debugValidate();
        }
    }

    // IRInstListBase

    void IRInstListBase::Iterator::operator++()
    {
        if (inst)
        {
            inst = inst->next;
        }
    }

    IRInstListBase::Iterator IRInstListBase::begin() { return Iterator(first); }
    IRInstListBase::Iterator IRInstListBase::end() { return Iterator(last ? last->next : nullptr); }

    //

    IRUse* IRInst::getOperands()
    {
        // We assume that *all* instructions are laid out
        // in memory such that their arguments come right
        // after the first `sizeof(IRInst)` bytes.
        //
        // TODO: we probably need to be careful and make
        // this more robust.

        return (IRUse*)(this + 1);
    }

    IRDecoration* IRInst::findDecorationImpl(IROp decorationOp)
    {
        for(auto dd : getDecorations())
        {
            if(dd->op == decorationOp)
                return dd;
        }
        return nullptr;
    }

    // IRConstant

    IRIntegerValue GetIntVal(IRInst* inst)
    {
        switch (inst->op)
        {
        default:
            SLANG_UNEXPECTED("needed a known integer value");
            UNREACHABLE_RETURN(0);

        case kIROp_IntLit:
            return static_cast<IRConstant*>(inst)->value.intVal;
            break;
        }
    }

    // IRParam

    IRParam* IRParam::getNextParam()
    {
        return as<IRParam>(getNextInst());
    }

    // IRArrayTypeBase

    IRInst* IRArrayTypeBase::getElementCount()
    {
        if (auto arrayType = as<IRArrayType>(this))
            return arrayType->getElementCount();

        return nullptr;
    }


    // IRBlock

    IRParam* IRBlock::getLastParam()
    {
        IRParam* param = getFirstParam();
        if (!param) return nullptr;

        while (auto nextParam = param->getNextParam())
            param = nextParam;

        return param;
    }

    void IRBlock::addParam(IRParam* param)
    {
        // If there are any existing parameters,
        // then insert after the last of them.
        //
        if (auto lastParam = getLastParam())
        {
            param->insertAfter(lastParam);
        }
        //
        // Otherwise, if there are any existing
        // "ordinary" instructions, insert before
        // the first of them.
        //
        else if(auto firstOrdinary = getFirstOrdinaryInst())
        {
            param->insertBefore(firstOrdinary);
        }
        //
        // Otherwise the block currently has neither
        // parameters nor orindary instructions,
        // so we can safely insert at the end of
        // the list of (raw) children.
        //
        else
        {
            param->insertAtEnd(this);
        }
    }

    IRInst* IRBlock::getFirstOrdinaryInst()
    {
        // Find the last parameter (if any) of the block
        auto lastParam = getLastParam();
        if (lastParam)
        {
            // If there is a last parameter, then the
            // instructions after it are the ordinary
            // instructions.
            return lastParam->getNextInst();
        }
        else
        {
            // If there isn't a last parameter, then
            // there must not have been *any* parameters,
            // and so the first instruction in the block
            // is also the first ordinary one.
            return getFirstInst();
        }
    }

    IRInst* IRBlock::getLastOrdinaryInst()
    {
        // Under normal circumstances, the last instruction
        // in the block is also the last ordinary instruction.
        // However, there is the special case of a block with
        // only parameters (which might happen as a temporary
        // state while we are building IR).
        auto inst = getLastInst();

        // If the last instruction is a parameter, then
        // there are no ordinary instructions, so the last
        // one is a null pointer.
        if (as<IRParam>(inst))
            return nullptr;

        // Otherwise the last instruction is the last "ordinary"
        // instruction as well.
        return inst;
    }


    // The predecessors of a block should all show up as users
    // of its value, so rather than explicitly store the CFG,
    // we will recover it on demand from the use-def information.
    //
    // Note: we are really iterating over incoming/outgoing *edges*
    // for a block, because there might be multiple uses of a block,
    // if more than one way of an N-way branch targets the same block.

    // Get the list of successor blocks for an instruction,
    // which we expect to be the last instruction in a block.
    static IRBlock::SuccessorList getSuccessors(IRInst* terminator)
    {
        // If the block somehow isn't terminated, then
        // there is no way to read its successors, so
        // we return an empty list.
        if (!terminator || !as<IRTerminatorInst>(terminator))
            return IRBlock::SuccessorList(nullptr, nullptr);

        // Otherwise, based on the opcode of the terminator
        // instruction, we will build up our list of uses.
        IRUse* begin = nullptr;
        IRUse* end = nullptr;
        UInt stride = 1;

        auto operands = terminator->getOperands();
        switch (terminator->op)
        {
        case kIROp_ReturnVal:
        case kIROp_ReturnVoid:
        case kIROp_Unreachable:
        case kIROp_MissingReturn:
        case kIROp_discard:
            break;

        case kIROp_unconditionalBranch:
        case kIROp_loop:
            // unconditonalBranch <block>
            begin = operands + 0;
            end = begin + 1;
            break;

        case kIROp_conditionalBranch:
        case kIROp_ifElse:
            // conditionalBranch <condition> <trueBlock> <falseBlock>
            begin = operands + 1;
            end = begin + 2;
            break;

        case kIROp_Switch:
            // switch <val> <break> <default> <caseVal1> <caseBlock1> ...
            begin = operands + 2;

            // TODO: this ends up point one *after* the "one after the end"
            // location, so we should really change the representation
            // so that we don't need to form this pointer...
            end = operands + terminator->getOperandCount() + 1;
            stride = 2;
            break;

        default:
            SLANG_UNEXPECTED("unhandled terminator instruction");
            UNREACHABLE_RETURN(IRBlock::SuccessorList(nullptr, nullptr));
        }

        return IRBlock::SuccessorList(begin, end, stride);
    }

    static IRUse* adjustPredecessorUse(IRUse* use)
    {
        // We will search until we either find a
        // suitable use, or run out of uses.
        for (;use; use = use->nextUse)
        {
            // We only want to deal with uses that represent
            // a "sucessor" operand to some terminator instruction.
            // We will re-use the logic for getting the successor
            // list from such an instruction.

            auto successorList = getSuccessors((IRInst*) use->getUser());

            if(use >= successorList.begin_
                && use < successorList.end_)
            {
                UInt index = (use - successorList.begin_);
                if ((index % successorList.stride) == 0)
                {
                    // This use is in the range of the sucessor list,
                    // and so it represents a real edge between
                    // blocks.
                    return use;
                }
            }
        }

        // If we ran out of uses, then we are at the end
        // of the list of incoming edges.
        return nullptr;
    }

    IRBlock::PredecessorList IRBlock::getPredecessors()
    {
        // We want to iterate over the predecessors of this block.
        // First, we resign ourselves to iterating over the
        // incoming edges, rather than the blocks themselves.
        // This might sound like a trival distinction, but it is
        // possible for there to be multiple edges between two
        // blocks (as for a `switch` with multiple cases that
        // map to the same code). Any client that wants just
        // the unique predecessor blocks needs to deal with
        // the deduplication themselves.
        //
        // Next, we note that for any predecessor edge, there will
        // be a use of this block in the terminator instruction of
        // the predecessor. We basically just want to iterate over
        // the users of this block, then, but we need to be careful
        // to rule out anything that doesn't actually represent
        // an edge. The `adjustPredecessorUse` function will be
        // used to search for a use that actually represents an edge.

        return PredecessorList(
            adjustPredecessorUse(firstUse));
    }

    UInt IRBlock::PredecessorList::getCount()
    {
        UInt count = 0;
        for (auto ii : *this)
        {
            (void)ii;
            count++;
        }
        return count;
    }

    bool IRBlock::PredecessorList::isEmpty()
    {
        return !(begin() != end());
    }


    void IRBlock::PredecessorList::Iterator::operator++()
    {
        if (!use) return;
        use = adjustPredecessorUse(use->nextUse);
    }

    IRBlock* IRBlock::PredecessorList::Iterator::operator*()
    {
        if (!use) return nullptr;
        return (IRBlock*)use->getUser()->parent;
    }

    IRBlock::SuccessorList IRBlock::getSuccessors()
    {
        // The successors of a block will all be listed
        // as operands of its terminator instruction.
        // Depending on the terminator, we might have
        // different numbers of operands to deal with.
        //
        // (We might also have to deal with a "stride"
        // in the case where the basic-block operands
        // are mixed up with non-block operands)

        auto terminator = getLastInst();
        return Slang::getSuccessors(terminator);
    }

    UInt IRBlock::SuccessorList::getCount()
    {
        UInt count = 0;
        for (auto ii : *this)
        {
            (void)ii;
            count++;
        }
        return count;
    }

    void IRBlock::SuccessorList::Iterator::operator++()
    {
        use += stride;
    }

    IRBlock* IRBlock::SuccessorList::Iterator::operator*()
    {
        return (IRBlock*)use->get();
    }

    UInt IRUnconditionalBranch::getArgCount()
    {
        switch(op)
        {
        case kIROp_unconditionalBranch:
            return getOperandCount() - 1;

        case kIROp_loop:
            return getOperandCount() - 3;

        default:
            SLANG_UNEXPECTED("unhandled unconditional branch opcode");
            UNREACHABLE_RETURN(0);
        }
    }

    IRUse* IRUnconditionalBranch::getArgs()
    {
        switch(op)
        {
        case kIROp_unconditionalBranch:
            return getOperands() + 1;

        case kIROp_loop:
            return getOperands() + 3;

        default:
            SLANG_UNEXPECTED("unhandled unconditional branch opcode");
            UNREACHABLE_RETURN(0);
        }
    }

    IRInst* IRUnconditionalBranch::getArg(UInt index)
    {
        return getArgs()[index].usedValue;
    }

    IRParam* IRGlobalValueWithParams::getFirstParam()
    {
        auto entryBlock = getFirstBlock();
        if(!entryBlock) return nullptr;

        return entryBlock->getFirstParam();
    }

    IRParam* IRGlobalValueWithParams::getLastParam()
    {
        auto entryBlock = getFirstBlock();
        if(!entryBlock) return nullptr;

        return entryBlock->getLastParam();
    }

    IRInstList<IRParam> IRGlobalValueWithParams::getParams()
    {
        auto entryBlock = getFirstBlock();
        if(!entryBlock) return IRInstList<IRParam>();

        return entryBlock->getParams();
    }


    // IRFunc

    IRType* IRFunc::getResultType() { return getDataType()->getResultType(); }
    UInt IRFunc::getParamCount() { return getDataType()->getParamCount(); }
    IRType* IRFunc::getParamType(UInt index) { return getDataType()->getParamType(index); }

    void IRGlobalValueWithCode::addBlock(IRBlock* block)
    {
        block->insertAtEnd(this);
    }

    //

    bool isTerminatorInst(IROp op)
    {
        switch (op)
        {
        default:
            return false;

        case kIROp_ReturnVal:
        case kIROp_ReturnVoid:
        case kIROp_unconditionalBranch:
        case kIROp_conditionalBranch:
        case kIROp_loop:
        case kIROp_ifElse:
        case kIROp_discard:
        case kIROp_Switch:
        case kIROp_Unreachable:
        case kIROp_MissingReturn:
            return true;
        }
    }

    bool isTerminatorInst(IRInst* inst)
    {
        if (!inst) return false;
        return isTerminatorInst(inst->op);
    }

    //

    IRBlock* IRBuilder::getBlock()
    {
        return as<IRBlock>(insertIntoParent);
    }

    // Get the current function (or other value with code)
    // that we are inserting into (if any).
    IRGlobalValueWithCode* IRBuilder::getFunc()
    {
        auto pp = insertIntoParent;
        if (auto block = as<IRBlock>(pp))
        {
            pp = pp->getParent();
        }
        return as<IRGlobalValueWithCode>(pp);
    }


    void IRBuilder::setInsertInto(IRInst* insertInto)
    {
        insertIntoParent = insertInto;
        insertBeforeInst = nullptr;
    }

    void IRBuilder::setInsertBefore(IRInst* insertBefore)
    {
        SLANG_ASSERT(insertBefore);
        insertIntoParent = insertBefore->parent;
        insertBeforeInst = insertBefore;
    }


    // Add an instruction into the current scope
    void IRBuilder::addInst(
        IRInst*     inst)
    {
        if(insertBeforeInst)
        {
            inst->insertBefore(insertBeforeInst);
        }
        else if (insertIntoParent)
        {
            inst->insertAtEnd(insertIntoParent);
        }
        else
        {
            // Don't append the instruction anywhere
        }
    }

    // Given two parent instructions, pick the better one to use as as
    // insertion location for a "hoistable" instruction.
    //
    IRInst* mergeCandidateParentsForHoistableInst(IRInst* left, IRInst* right)
    {
        // If the candidates are both the same, then who cares?
        if(left == right) return left;

        // If either `left` or `right` is a block, then we need to be
        // a bit careful, because blocks can see other values just using
        // the dominance relationship, without a direct parent-child relationship.
        //
        // First, check if each of `left` and `right` is a block.
        //
        auto leftBlock = as<IRBlock>(left);
        auto rightBlock = as<IRBlock>(right);
        //
        // As a special case, if both of these are blocks in the same parent,
        // then we need to pick between them based on dominance.
        //
        if (leftBlock && rightBlock && (leftBlock->getParent() == rightBlock->getParent()))
        {
            // We assume that the order of basic blocks in a function is compatible
            // with the dominance relationship (that is, if A dominates B, then
            // A comes before B in the list of blocks), so it suffices to pick
            // the *later* of the two blocks.
            //
            // There are ways we could try to speed up this search, but no matter
            // what it will be O(n) in the number of blocks, unless we build
            // an explicit dominator tree, which is infeasible during IR building.
            // Thus we just do a simple linear walk here.
            //
            // We will start at `leftBlock` and walk forward, until either...
            //
            for (auto ll = leftBlock; ll; ll = ll->getNextBlock())
            {
                // ... we see `rightBlock` (in which case `rightBlock` came later), or ...
                //
                if (ll == rightBlock) return rightBlock;
            }
            //
            // ... we run out of blocks (in which case `leftBlock` came later).
            //
            return leftBlock;
        }

        //
        // If the special case above doesn't apply, then `left` or `right` might
        // still be a block, but they aren't blocks nested in the same function.
        // We will find the first non-block ancestor of `left` and/or `right`.
        // This will either be the inst itself (it is isn't a block), or
        // its immediate parent (if it *is* a block).
        //
        auto leftNonBlock = leftBlock ? leftBlock->getParent() : left;
        auto rightNonBlock = rightBlock ? rightBlock->getParent() : right;

        // If either side is null, then take the non-null one.
        //
        if (!leftNonBlock) return right;
        if (!rightNonBlock) return left;

        // If the non-block on the left or right is a descendent of
        // the other, then that is what we should use.
        //
        IRInst* parentNonBlock = nullptr;
        for (auto ll = leftNonBlock; ll; ll = ll->getParent())
        {
            if (ll == rightNonBlock)
            {
                parentNonBlock = leftNonBlock;
                break;
            }
        }
        for (auto rr = rightNonBlock; rr; rr = rr->getParent())
        {
            if (rr == leftNonBlock)
            {
                SLANG_ASSERT(!parentNonBlock || parentNonBlock == leftNonBlock);
                parentNonBlock = rightNonBlock;
                break;
            }
        }

        // As a matter of validity in the IR, we expect one
        // of the two to be an ancestor (in the non-block case),
        // because otherwise we'd be violating the basic dominance
        // assumptions.
        //
        SLANG_ASSERT(parentNonBlock);

        // As a fallback, try to use the left parent as a default
        // in case things go badly.
        //
        if (!parentNonBlock)
        {
            parentNonBlock = leftNonBlock;
        }

        IRInst* parent = parentNonBlock;

        // At this point we've found a non-block parent where we
        // could stick things, but we have to fix things up in
        // case we should be inserting into a block beneath
        // that non-block parent.
        if (leftBlock && (parentNonBlock == leftNonBlock))
        {
            // We have a left block, and have picked its parent.

            // It cannot be the case that there is a right block
            // with the same parent, or else our special case
            // would have triggered at the start.
            SLANG_ASSERT(!rightBlock || (parentNonBlock != rightNonBlock));

            parent = leftBlock;
        }
        else if (rightBlock && (parentNonBlock == rightNonBlock))
        {
            // We have a right block, and have picked its parent.

            // We already tested above, so we know there isn't a
            // matching situation on the left side.

            parent = rightBlock;
        }

        // Okay, we've picked the parent we want to insert into,
        // *but* one last special case arises, because an `IRGlobalValueWithCode`
        // is not actually a suitable place to insert instructions.
        // Furthermore, there is no actual need to insert instructions at
        // that scope, because any parameters, etc. are actually attached
        // to the block(s) within the function.
        if (auto parentFunc = as<IRGlobalValueWithCode>(parent))
        {
            // Insert in the parent of the function (or other value with code).
            // We know that the parent must be able to hold ordinary instructions,
            // because it was able to hold this `IRGlobalValueWithCode`
            parent = parentFunc->getParent();
        }

        return parent;
    }

    IRInst* createEmptyInst(
        IRModule*   module,
        IROp        op,
        int         totalArgCount)
    {
        size_t size = sizeof(IRInst) + (totalArgCount) * sizeof(IRUse);

        SLANG_ASSERT(module);
        IRInst* inst = (IRInst*)module->memoryArena.allocateAndZero(size);

        inst->operandCount = uint32_t(totalArgCount);
        inst->op = op;

        return inst;
    }

    IRInst* createEmptyInstWithSize(
        IRModule*   module,
        IROp        op,
        size_t      totalSizeInBytes)
    {
        SLANG_ASSERT(totalSizeInBytes >= sizeof(IRInst));

        SLANG_ASSERT(module);
        IRInst* inst = (IRInst*)module->memoryArena.allocateAndZero(totalSizeInBytes);

        inst->operandCount = 0;
        inst->op = op;

        return inst;
    }

    // Given an instruction that represents a constant, a type, etc.
    // Try to "hoist" it as far toward the global scope as possible
    // to insert it at a location where it will be maximally visible.
    //
    void addHoistableInst(
        IRBuilder*  builder,
        IRInst*     inst)
    {
        // Start with the assumption that we would insert this instruction
        // into the global scope (the instruction that represents the module)
        IRInst* parent = builder->getModule()->getModuleInst();

        // The above decision might be invalid, because there might be
        // one or more operands of the instruction that are defined in
        // more deeply nested parents than the global scope.
        //
        // Therefore, we will scan the operands of the instruction, and
        // look at the parents that define them.
        //
        UInt operandCount = inst->getOperandCount();
        for (UInt ii = 0; ii < operandCount; ++ii)
        {
            auto operand = inst->getOperand(ii);
            if (!operand)
                continue;

            auto operandParent = operand->getParent();

            parent = mergeCandidateParentsForHoistableInst(parent, operandParent);
        }

        // We better have ended up with a place to insert.
        SLANG_ASSERT(parent);

        // If we have chosen to insert into the same parent that the
        // IRBuilder is configured to use, then respect its `insertBeforeInst`
        // setting.
        if (parent == builder->insertIntoParent)
        {
            builder->addInst(inst);
            return;
        }

        // Otherwise, we just want to insert at the end of the chosen parent.
        //
        // TODO: be careful about inserting after the terminator of a block...

        inst->insertAtEnd(parent);
    }

    static void maybeSetSourceLoc(
        IRBuilder*  builder,
        IRInst*     value)
    {
        if(!builder)
            return;

        auto sourceLocInfo = builder->sourceLocInfo;
        if(!sourceLocInfo)
            return;

        // Try to find something with usable location info
        for(;;)
        {
            if(sourceLocInfo->sourceLoc.getRaw())
                break;

            if(!sourceLocInfo->next)
                break;

            sourceLocInfo = sourceLocInfo->next;
        }

        value->sourceLoc = sourceLocInfo->sourceLoc;
    }

    // Create an IR instruction/value and initialize it.
    //
    // In this case `argCount` and `args` represent the
    // arguments *after* the type (which is a mandatory
    // argument for all instructions).
    template<typename T>
    static T* createInstImpl(
        IRModule*       module,
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        UInt            fixedArgCount,
        IRInst* const* fixedArgs,
        UInt                    varArgListCount,
        UInt const*             listArgCounts,
        IRInst* const* const*   listArgs)
    {
        UInt varArgCount = 0;
        for (UInt ii = 0; ii < varArgListCount; ++ii)
        {
            varArgCount += listArgCounts[ii];
        }

        UInt size = sizeof(IRInst) + (fixedArgCount + varArgCount) * sizeof(IRUse);
        if (sizeof(T) > size)
        {
            size = sizeof(T);
        }

        SLANG_ASSERT(module);
        T* inst = (T*)module->memoryArena.allocateAndZero(size);

        // TODO: Do we need to run ctor after zeroing?
        new(inst)T();

        inst->operandCount = (uint32_t)(fixedArgCount + varArgCount);

        inst->op = op;

        if (type)
        {
            inst->typeUse.init(inst, type);
        }

        maybeSetSourceLoc(builder, inst);

        auto operand = inst->getOperands();

        for( UInt aa = 0; aa < fixedArgCount; ++aa )
        {
            if (fixedArgs)
            {
                operand->init(inst, fixedArgs[aa]);
            }
            operand++;
        }

        for (UInt ii = 0; ii < varArgListCount; ++ii)
        {
            UInt listArgCount = listArgCounts[ii];
            for (UInt jj = 0; jj < listArgCount; ++jj)
            {
                if (listArgs[ii])
                {
                    operand->init(inst, listArgs[ii][jj]);
                }
                else
                {
                    operand->init(inst, nullptr);
                }
                operand++;
            }
        }
        return inst;
    }

    static IRInst* createInstWithSizeImpl(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        size_t          sizeInBytes)
    {
        auto module = builder->getModule();
        IRInst* inst = (IRInst*)module->memoryArena.allocate(sizeInBytes);
        // Zero only the 'type'
        memset(inst, 0, sizeof(IRInst));
        // TODO: Do we need to run ctor after zeroing?
        new (inst) IRInst;

        inst->op = op;
        if (type)
        {
            inst->typeUse.init(inst, type);
        }
        maybeSetSourceLoc(builder, inst);
        return inst; 
    }

    template<typename T>
    static T* createInstImpl(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        UInt            fixedArgCount,
        IRInst* const* fixedArgs,
        UInt           varArgCount = 0,
        IRInst* const* varArgs = nullptr)
    {
        return createInstImpl<T>(
            builder->getModule(),
            builder,
            op,
            type,
            fixedArgCount,
            fixedArgs,
            1,
            &varArgCount,
            &varArgs);
    }

    template<typename T>
    static T* createInstImpl(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        UInt            fixedArgCount,
        IRInst* const*  fixedArgs,
        UInt                    varArgListCount,
        UInt const*             listArgCount,
        IRInst* const* const*   listArgs)
    {
        return createInstImpl<T>(
            builder->getModule(),
            builder,
            op,
            type,
            fixedArgCount,
            fixedArgs,
            varArgListCount,
            listArgCount,
            listArgs);
    }

    template<typename T>
    static T* createInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        UInt            argCount,
        IRInst* const* args)
    {
        return createInstImpl<T>(
            builder,
            op,
            type,
            argCount,
            args);
    }

    template<typename T>
    static T* createInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type)
    {
        return createInstImpl<T>(
            builder,
            op,
            type,
            0,
            nullptr);
    }

    template<typename T>
    static T* createInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        IRInst*         arg)
    {
        return createInstImpl<T>(
            builder,
            op,
            type,
            1,
            &arg);
    }

    template<typename T>
    static T* createInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        IRInst*         arg1,
        IRInst*         arg2)
    {
        IRInst* args[] = { arg1, arg2 };
        return createInstImpl<T>(
            builder,
            op,
            type,
            2,
            &args[0]);
    }

    template<typename T>
    static T* createInstWithTrailingArgs(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        UInt            argCount,
        IRInst* const* args)
    {
        return createInstImpl<T>(
            builder,
            op,
            type,
            argCount,
            args);
    }

    template<typename T>
    static T* createInstWithTrailingArgs(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        UInt            fixedArgCount,
        IRInst* const*  fixedArgs,
        UInt            varArgCount,
        IRInst* const*  varArgs)
    {
        return createInstImpl<T>(
            builder,
            op,
            type,
            fixedArgCount,
            fixedArgs,
            varArgCount,
            varArgs);
    }

    template<typename T>
    static T* createInstWithTrailingArgs(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        IRInst*         arg1,
        UInt            varArgCount,
        IRInst* const*  varArgs)
    {
        IRInst* fixedArgs[] = { arg1 };
        UInt fixedArgCount = sizeof(fixedArgs) / sizeof(fixedArgs[0]);

        return createInstImpl<T>(
            builder,
            op,
            type,
            fixedArgCount,
            fixedArgs,
            varArgCount,
            varArgs);
    }
    //

    bool operator==(IRInstKey const& left, IRInstKey const& right)
    {
        if(left.inst->op != right.inst->op) return false;
        if(left.inst->getFullType() != right.inst->getFullType()) return false;
        if(left.inst->operandCount != right.inst->operandCount) return false;

        auto argCount = left.inst->operandCount;
        auto leftArgs = left.inst->getOperands();
        auto rightArgs = right.inst->getOperands();
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            if(leftArgs[aa].get() != rightArgs[aa].get())
                return false;
        }

        return true;
    }

    int IRInstKey::GetHashCode()
    {
        auto code = Slang::GetHashCode(inst->op);
        code = combineHash(code, Slang::GetHashCode(inst->getFullType()));
        code = combineHash(code, Slang::GetHashCode(inst->getOperandCount()));

        auto argCount = inst->getOperandCount();
        auto args = inst->getOperands();
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            code = combineHash(code, Slang::GetHashCode(args[aa].get()));
        }
        return code;
    }

    UnownedStringSlice IRConstant::getStringSlice()
    {
        assert(op == kIROp_StringLit);
        // If the transitory decoration is set, then this is uses the transitoryStringVal for the text storage.
        // This is typically used when we are using a transitory IRInst held on the stack (such that it can be looked up in cached), 
        // that just points to a string elsewhere, and NOT the typical normal style, where the string is held after the instruction in memory.
        //
        if(findDecorationImpl(kIROp_TransitoryDecoration))
        {
            return UnownedStringSlice(value.transitoryStringVal.chars, value.transitoryStringVal.numChars);
        }
        else
        {
            return UnownedStringSlice(value.stringVal.chars, value.stringVal.numChars);
        }
    }

    /// True if constants are equal
    bool IRConstant::equal(IRConstant& rhs)
    {
        // If they are literally the same thing.. 
        if (this == &rhs)
        {
            return true;
        }
        // Check the type and they are the same op
        if (op != rhs.op || 
           getFullType() != rhs.getFullType())
        {
            return false;
        }
        switch (op)
        {
            case kIROp_BoolLit:
            case kIROp_FloatLit:
            case kIROp_IntLit:
            {
                SLANG_COMPILE_TIME_ASSERT(sizeof(IRFloatingPointValue) == sizeof(IRIntegerValue));
                // ... we can just compare as bits
                return value.intVal == rhs.value.intVal;
            }
            case kIROp_PtrLit:
            {
                return value.ptrVal == rhs.value.ptrVal;
            }
            case kIROp_StringLit:
            {
                return getStringSlice() == rhs.getStringSlice();
            }
            default: break;
        }

        SLANG_ASSERT(!"Unhandled type");
        return false;
    }

    int IRConstant::getHashCode()
    {
        auto code = Slang::GetHashCode(op);
        code = combineHash(code, Slang::GetHashCode(getFullType()));

        switch (op)
        {
            case kIROp_BoolLit:
            case kIROp_FloatLit:
            case kIROp_IntLit:
            {
                SLANG_COMPILE_TIME_ASSERT(sizeof(IRFloatingPointValue) == sizeof(IRIntegerValue));
                // ... we can just compare as bits
                return combineHash(code, Slang::GetHashCode(value.intVal));
            }
            case kIROp_PtrLit:
            {
                return combineHash(code, Slang::GetHashCode(value.ptrVal));
            }
            case kIROp_StringLit:
            {
                const UnownedStringSlice slice = getStringSlice();
                return combineHash(code, Slang::GetHashCode(slice.begin(), slice.size()));
            }
            default:
            {
                SLANG_ASSERT(!"Invalid type");
                return 0;
            }
        }
    }

    static IRConstant* findOrEmitConstant(
        IRBuilder*      builder,
        IRConstant&     keyInst)
    {
        // We now know where we want to insert, but there might
        // already be an equivalent instruction in that block.
        //
        // We will check for such an instruction in a slightly hacky
        // way: we will construct a temporary instruction and
        // then use it to look up in a cache of instructions.
        // The 'fake' instruction is passed in as keyInst.

        IRConstantKey key;
        key.inst = &keyInst;

        IRConstant* irValue = nullptr;
        if( builder->sharedBuilder->constantMap.TryGetValue(key, irValue) )
        {
            // We found a match, so just use that.
            return irValue;
        }
    
        // Calculate the minimum object size (ie not including the payload of value)    
        const size_t prefixSize = SLANG_OFFSET_OF(IRConstant, value);

        switch (keyInst.op)
        {
        default:
            SLANG_UNEXPECTED("missing case for IR constant");
            break;

            case kIROp_BoolLit:
            case kIROp_IntLit:
            {
                irValue = static_cast<IRConstant*>(createInstWithSizeImpl(builder, keyInst.op, keyInst.getFullType(), prefixSize + sizeof(IRIntegerValue)));
                irValue->value.intVal = keyInst.value.intVal;
                break; 
            }
            case kIROp_FloatLit:
            {
                irValue = static_cast<IRConstant*>(createInstWithSizeImpl(builder, keyInst.op, keyInst.getFullType(), prefixSize + sizeof(IRFloatingPointValue)));
                irValue->value.floatVal = keyInst.value.floatVal;
                break;
            }
            case kIROp_PtrLit:
            {
                irValue = static_cast<IRConstant*>(createInstWithSizeImpl(builder, keyInst.op, keyInst.getFullType(), prefixSize + sizeof(void*)));
                irValue->value.ptrVal = keyInst.value.ptrVal;
                break;
            }
            case kIROp_StringLit:
            {
                const UnownedStringSlice slice = keyInst.getStringSlice();

                const size_t sliceSize = slice.size();
                const size_t instSize = prefixSize + offsetof(IRConstant::StringValue, chars) + sliceSize; 

                irValue = static_cast<IRConstant*>(createInstWithSizeImpl(builder, keyInst.op, keyInst.getFullType(), instSize));

                IRConstant::StringValue& dstString = irValue->value.stringVal;

                dstString.numChars = uint32_t(sliceSize);
                // Turn into pointer to avoid warning of array overrun
                char* dstChars = dstString.chars;
                // Copy the chars
                memcpy(dstChars, slice.begin(), sliceSize); 

                break;
            }
        }

        key.inst = irValue;
        builder->sharedBuilder->constantMap.Add(key, irValue);

        addHoistableInst(builder, irValue);

        return irValue;
    }

    //

    IRInst* IRBuilder::getBoolValue(bool inValue)
    {
        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.op = kIROp_BoolLit;
        keyInst.typeUse.usedValue = getBoolType();
        keyInst.value.intVal = IRIntegerValue(inValue);
        return findOrEmitConstant(this, keyInst);
    }

    IRInst* IRBuilder::getIntValue(IRType* type, IRIntegerValue inValue)
    {
        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.op = kIROp_IntLit;
        keyInst.typeUse.usedValue = type;
        keyInst.value.intVal = inValue;
        return findOrEmitConstant(this, keyInst);
    }

    IRInst* IRBuilder::getFloatValue(IRType* type, IRFloatingPointValue inValue)
    {
        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.op = kIROp_FloatLit;
        keyInst.typeUse.usedValue = type;
        keyInst.value.floatVal = inValue;
        return findOrEmitConstant(this, keyInst);
    }

    IRStringLit* IRBuilder::getStringValue(const UnownedStringSlice& inSlice)
    {
        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        
        // Mark that this is on the stack...
        IRDecoration stackDecoration;
        memset(&stackDecoration, 0, sizeof(stackDecoration));
        stackDecoration.op = kIROp_TransitoryDecoration;
        stackDecoration.insertAtEnd(&keyInst);
            
        keyInst.op = kIROp_StringLit;
        keyInst.typeUse.usedValue = getStringType();
        
        IRConstant::StringSliceValue& dstSlice = keyInst.value.transitoryStringVal;
        dstSlice.chars = const_cast<char*>(inSlice.begin());
        dstSlice.numChars = uint32_t(inSlice.size());

        return static_cast<IRStringLit*>(findOrEmitConstant(this, keyInst));
    }

    IRPtrLit* IRBuilder::getPtrValue(void* value)
    {
        IRType* type = getPtrType(getVoidType());

        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.op = kIROp_PtrLit;
        keyInst.typeUse.usedValue = type;
        keyInst.value.ptrVal = value;
        return (IRPtrLit*) findOrEmitConstant(this, keyInst);
    }

 
    IRInst* findOrEmitHoistableInst(
        IRBuilder*              builder,
        IRType*                 type,
        IROp                    op,
        UInt                    operandListCount,
        UInt const*             listOperandCounts,
        IRInst* const* const*   listOperands)
    {
        UInt operandCount = 0;
        for (UInt ii = 0; ii < operandListCount; ++ii)
        {
            operandCount += listOperandCounts[ii];
        }

        // We are going to create a dummy instruction on the stack,
        // which will be used as a key for lookup, so see if we
        // already have an equivalent instruction available to use.

        size_t keySize = sizeof(IRInst) + operandCount * sizeof(IRUse);
        IRInst* keyInst = (IRInst*) malloc(keySize);
        memset(keyInst, 0, keySize);

        new(keyInst) IRInst();
        keyInst->op = op;
        keyInst->typeUse.usedValue = type;
        keyInst->operandCount = (uint32_t) operandCount;

        IRUse* operand = keyInst->getOperands();
        for (UInt ii = 0; ii < operandListCount; ++ii)
        {
            UInt listOperandCount = listOperandCounts[ii];
            for (UInt jj = 0; jj < listOperandCount; ++jj)
            {
                operand->usedValue = listOperands[ii][jj];
                operand++;
            }
        }

        IRInstKey key;
        key.inst = keyInst;

        IRInst* foundInst = nullptr;
        bool found = builder->sharedBuilder->globalValueNumberingMap.TryGetValue(key, foundInst);

        free((void*)keyInst);

        if (found)
        {
            return foundInst;
        }

        // If no instruction was found, then we need to emit it.

        IRInst* inst = createInstImpl<IRInst>(
            builder,
            op,
            type,
            0,
            nullptr,
            operandListCount,
            listOperandCounts,
            listOperands);
        addHoistableInst(builder, inst);

        key.inst = inst;
        builder->sharedBuilder->globalValueNumberingMap.Add(key, inst);

        return inst;
    }

    IRInst* findOrEmitHoistableInst(
        IRBuilder*      builder,
        IRType*         type,
        IROp            op,
        UInt            operandCount,
        IRInst* const*  operands)
    {
        return findOrEmitHoistableInst(
            builder,
            type,
            op,
            1,
            &operandCount,
            &operands);
    }

    IRInst* findOrEmitHoistableInst(
        IRBuilder*      builder,
        IRType*         type,
        IROp            op,
        IRInst*         operand,
        UInt            operandCount,
        IRInst* const*  operands)
    {
        UInt counts[] = { 1, operandCount };
        IRInst* const* lists[] = { &operand, operands };

        return findOrEmitHoistableInst(
            builder,
            type,
            op,
            2,
            counts,
            lists);
    }


    IRType* IRBuilder::getType(
        IROp            op,
        UInt            operandCount,
        IRInst* const*  operands)
    {
        return (IRType*) findOrEmitHoistableInst(
            this,
            nullptr,
            op,
            operandCount,
            operands);
    }

    IRType* IRBuilder::getType(
        IROp            op)
    {
        return getType(op, 0, nullptr);
    }

    IRBasicType* IRBuilder::getBasicType(BaseType baseType)
    {
        return (IRBasicType*)getType(
            IROp((UInt)kIROp_FirstBasicType + (UInt)baseType));
    }

    IRBasicType* IRBuilder::getVoidType()
    {
        return (IRVoidType*)getType(kIROp_VoidType);
    }

    IRBasicType* IRBuilder::getBoolType()
    {
        return (IRBoolType*)getType(kIROp_BoolType);
    }

    IRBasicType* IRBuilder::getIntType()
    {
        return (IRBasicType*)getType(kIROp_IntType);
    }

    IRStringType* IRBuilder::getStringType()
    {
        return (IRStringType*)getType(kIROp_StringType);
    }

    IRBasicBlockType*   IRBuilder::getBasicBlockType()
    {
        return (IRBasicBlockType*)getType(kIROp_BasicBlockType);
    }

    IRTypeKind* IRBuilder::getTypeKind()
    {
        return (IRTypeKind*)getType(kIROp_TypeKind);
    }

    IRGenericKind* IRBuilder::getGenericKind()
    {
        return (IRGenericKind*)getType(kIROp_GenericKind);
    }

    IRPtrType*  IRBuilder::getPtrType(IRType* valueType)
    {
        return (IRPtrType*) getPtrType(kIROp_PtrType, valueType);
    }

    IROutType* IRBuilder::getOutType(IRType* valueType)
    {
        return (IROutType*) getPtrType(kIROp_OutType, valueType);
    }

    IRInOutType* IRBuilder::getInOutType(IRType* valueType)
    {
        return (IRInOutType*) getPtrType(kIROp_InOutType, valueType);
    }

    IRRefType* IRBuilder::getRefType(IRType* valueType)
    {
        return (IRRefType*) getPtrType(kIROp_RefType, valueType);
    }

    IRPtrTypeBase* IRBuilder::getPtrType(IROp op, IRType* valueType)
    {
        IRInst* operands[] = { valueType };
        return (IRPtrTypeBase*) getType(
            op,
            1,
            operands);
    }

    IRArrayTypeBase* IRBuilder::getArrayTypeBase(
        IROp    op,
        IRType* elementType,
        IRInst* elementCount)
    {
        IRInst* operands[] = { elementType, elementCount };
        return (IRArrayTypeBase*)getType(
            op,
            op == kIROp_ArrayType ? 2 : 1,
            operands);
    }

    IRArrayType* IRBuilder::getArrayType(
        IRType* elementType,
        IRInst* elementCount)
    {
        IRInst* operands[] = { elementType, elementCount };
        return (IRArrayType*)getType(
            kIROp_ArrayType,
            sizeof(operands) / sizeof(operands[0]),
            operands);
    }

    IRUnsizedArrayType* IRBuilder::getUnsizedArrayType(
        IRType* elementType)
    {
        IRInst* operands[] = { elementType };
        return (IRUnsizedArrayType*)getType(
            kIROp_UnsizedArrayType,
            sizeof(operands) / sizeof(operands[0]),
            operands);
    }

    IRVectorType* IRBuilder::getVectorType(
        IRType* elementType,
        IRInst* elementCount)
    {
        IRInst* operands[] = { elementType, elementCount };
        return (IRVectorType*)getType(
            kIROp_VectorType,
            sizeof(operands) / sizeof(operands[0]),
            operands);
    }

    IRMatrixType* IRBuilder::getMatrixType(
        IRType* elementType,
        IRInst* rowCount,
        IRInst* columnCount)
    {
        IRInst* operands[] = { elementType, rowCount, columnCount };
        return (IRMatrixType*)getType(
            kIROp_MatrixType,
            sizeof(operands) / sizeof(operands[0]),
            operands);
    }

    IRFuncType* IRBuilder::getFuncType(
        UInt            paramCount,
        IRType* const*  paramTypes,
        IRType*         resultType)
    {
        return (IRFuncType*) findOrEmitHoistableInst(
            this,
            nullptr,
            kIROp_FuncType,
            resultType,
            paramCount,
            (IRInst* const*) paramTypes);
    }

    IRConstExprRate* IRBuilder::getConstExprRate()
    {
        return (IRConstExprRate*)getType(kIROp_ConstExprRate);
    }

    IRGroupSharedRate* IRBuilder::getGroupSharedRate()
    {
        return (IRGroupSharedRate*)getType(kIROp_GroupSharedRate);
    }

    IRRateQualifiedType* IRBuilder::getRateQualifiedType(
        IRRate* rate,
        IRType* dataType)
    {
        IRInst* operands[] = { rate, dataType };
        return (IRRateQualifiedType*)getType(
            kIROp_RateQualifiedType,
            sizeof(operands) / sizeof(operands[0]),
            operands);
    }

    void IRBuilder::setDataType(IRInst* inst, IRType* dataType)
    {
        if (auto oldRateQualifiedType = as<IRRateQualifiedType>(inst->getFullType()))
        {
            // Construct a new rate-qualified type using the same rate.

            auto newRateQualifiedType = getRateQualifiedType(
                oldRateQualifiedType->getRate(),
                dataType);

            inst->setFullType(newRateQualifiedType);
        }
        else
        {
            // No rate? Just clobber the data type.
            inst->setFullType(dataType);
        }
    }


    IRUndefined* IRBuilder::emitUndefined(IRType* type)
    {
        auto inst = createInst<IRUndefined>(
            this,
            kIROp_undefined,
            type);

        addInst(inst);

        return inst;
    }

    IRInst* IRBuilder::emitExtractExistentialValue(
        IRType* type,
        IRInst* existentialValue)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_ExtractExistentialValue,
            type,
            1,
            &existentialValue);
        addInst(inst);
        return inst;
    }

    IRType* IRBuilder::emitExtractExistentialType(
        IRInst* existentialValue)
    {
        auto type = getTypeKind();
        auto inst = createInst<IRInst>(
            this,
            kIROp_ExtractExistentialType,
            type,
            1,
            &existentialValue);
        addInst(inst);
        return (IRType*) inst;
    }

    IRInst* IRBuilder::emitExtractExistentialWitnessTable(
        IRInst* existentialValue)
    {
        auto type = getWitnessTableType();
        auto inst = createInst<IRInst>(
            this,
            kIROp_ExtractExistentialWitnessTable,
            type,
            1,
            &existentialValue);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitSpecializeInst(
        IRType*         type,
        IRInst*         genericVal,
        UInt            argCount,
        IRInst* const*  args)
    {
        auto inst = createInstWithTrailingArgs<IRSpecialize>(
            this,
            kIROp_Specialize,
            type,
            1,
            &genericVal,
            argCount,
            args);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitLookupInterfaceMethodInst(
        IRType* type,
        IRInst* witnessTableVal,
        IRInst* interfaceMethodVal)
    {
        auto inst = createInst<IRLookupWitnessMethod>(
            this,
            kIROp_lookup_interface_method,
            type,
            witnessTableVal,
            interfaceMethodVal);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitCallInst(
        IRType*         type,
        IRInst*        pFunc,
        UInt            argCount,
        IRInst* const* args)
    {
        auto inst = createInstWithTrailingArgs<IRCall>(
            this,
            kIROp_Call,
            type,
            1,
            &pFunc,
            argCount,
            args);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::createIntrinsicInst(
        IRType*         type,
        IROp            op,
        UInt            argCount,
        IRInst* const* args)
    {
        return createInstWithTrailingArgs<IRInst>(
            this,
            op,
            type,
            argCount,
            args);
    }


    IRInst* IRBuilder::emitIntrinsicInst(
        IRType*         type,
        IROp            op,
        UInt            argCount,
        IRInst* const* args)
    {
        auto inst = createIntrinsicInst(
            type,
            op,
            argCount,
            args);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitConstructorInst(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args)
    {
        auto inst = createInstWithTrailingArgs<IRInst>(
            this,
            kIROp_Construct,
            type,
            argCount,
            args);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitMakeVector(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args)
    {
        return emitIntrinsicInst(type, kIROp_makeVector, argCount, args);
    }

    IRInst* IRBuilder::emitMakeArray(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args)
    {
        return emitIntrinsicInst(type, kIROp_makeArray, argCount, args);
    }

    IRInst* IRBuilder::emitMakeStruct(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args)
    {
        return emitIntrinsicInst(type, kIROp_makeStruct, argCount, args);
    }

    IRInst* IRBuilder::emitMakeExistential(
        IRType* type,
        IRInst* value,
        IRInst* witnessTable)
    {
        IRInst* args[] = {value, witnessTable};
        return emitIntrinsicInst(type, kIROp_MakeExistential, SLANG_COUNT_OF(args), args);
    }

    IRModule* IRBuilder::createModule()
    {
        auto module = new IRModule();
        module->session = getSession();

        auto moduleInst = createInstImpl<IRModuleInst>(
            module,
            this,
            kIROp_Module,
            nullptr,
            0,
            nullptr,
            0,
            nullptr,
            nullptr);
        module->moduleInst = moduleInst;
        moduleInst->module = module;

        return module;
    }

    void addGlobalValue(
        IRBuilder*  builder,
        IRInst*     value)
    {
        // Try to find a suitable parent for the
        // global value we are emitting.
        //
        // We will start out search at the current
        // parent instruction for the builder, and
        // possibly work our way up.
        //
        auto parent = builder->insertIntoParent;
        while(parent)
        {
            // Inserting into the top level of a module?
            // That is fine, and we can stop searching.
            if (as<IRModuleInst>(parent))
                break;

            // Inserting into a basic block inside of
            // a generic? That is okay too.
            if (auto block = as<IRBlock>(parent))
            {
                if (as<IRGeneric>(block->parent))
                    break;
            }

            // Otherwise, move up the chain.
            parent = parent->parent;
        }

        // If we somehow ran out of parents (possibly
        // because an instruction wasn't linked into
        // the full hierarchy yet), then we will
        // fall back to inserting into the overall module.
        if (!parent)
        {
            parent = builder->getModule()->getModuleInst();
        }

        // If it turns out that we are inserting into the
        // current "insert into" parent for the builder, then
        // we need to respect its "insert before" setting
        // as well.
        if (parent == builder->insertIntoParent
            && builder->insertBeforeInst)
        {
            value->insertBefore(builder->insertBeforeInst);
        }
        else
        {
            value->insertAtEnd(parent);
        }
    }

    IRFunc* IRBuilder::createFunc()
    {
        IRFunc* rsFunc = createInst<IRFunc>(
            this,
            kIROp_Func,
            nullptr);
        maybeSetSourceLoc(this, rsFunc);
        addGlobalValue(this, rsFunc);
        return rsFunc;
    }

    IRGlobalVar* IRBuilder::createGlobalVar(
        IRType* valueType)
    {
        auto ptrType = getPtrType(valueType);
        IRGlobalVar* globalVar = createInst<IRGlobalVar>(
            this,
            kIROp_GlobalVar,
            ptrType);
        maybeSetSourceLoc(this, globalVar);
        addGlobalValue(this, globalVar);
        return globalVar;
    }

    IRGlobalConstant* IRBuilder::createGlobalConstant(
        IRType* valueType)
    {
        IRGlobalConstant* globalConstant = createInst<IRGlobalConstant>(
            this,
            kIROp_GlobalConstant,
            valueType);
        maybeSetSourceLoc(this, globalConstant);
        addGlobalValue(this, globalConstant);
        return globalConstant;
    }

    IRGlobalParam* IRBuilder::createGlobalParam(
        IRType* valueType)
    {
        IRGlobalParam* inst = createInst<IRGlobalParam>(
            this,
            kIROp_GlobalParam,
            valueType);
        maybeSetSourceLoc(this, inst);
        addGlobalValue(this, inst);
        return inst;
    }

    IRWitnessTable* IRBuilder::createWitnessTable()
    {
        IRWitnessTable* witnessTable = createInst<IRWitnessTable>(
            this,
            kIROp_WitnessTable,
            nullptr);
        addGlobalValue(this, witnessTable);
        return witnessTable;
    }

    IRWitnessTableEntry* IRBuilder::createWitnessTableEntry(
        IRWitnessTable* witnessTable,
        IRInst*         requirementKey,
        IRInst*         satisfyingVal)
    {
        IRWitnessTableEntry* entry = createInst<IRWitnessTableEntry>(
            this,
            kIROp_WitnessTableEntry,
            nullptr,
            requirementKey,
            satisfyingVal);

        if (witnessTable)
        {
            entry->insertAtEnd(witnessTable);
        }

        return entry;
    }

    IRStructType* IRBuilder::createStructType()
    {
        IRStructType* structType = createInst<IRStructType>(
            this,
            kIROp_StructType,
            nullptr);
        addGlobalValue(this, structType);
        return structType;
    }

    IRInterfaceType* IRBuilder::createInterfaceType()
    {
        IRInterfaceType* interfaceType = createInst<IRInterfaceType>(
            this,
            kIROp_InterfaceType,
            nullptr);
        addGlobalValue(this, interfaceType);
        return interfaceType;
    }

    IRStructKey* IRBuilder::createStructKey()
    {
        IRStructKey* structKey = createInst<IRStructKey>(
            this,
            kIROp_StructKey,
            nullptr);
        addGlobalValue(this, structKey);
        return structKey;
    }

    // Create a field nested in a struct type, declaring that
    // the specified field key maps to a field with the specified type.
    IRStructField*  IRBuilder::createStructField(
        IRStructType*   structType,
        IRStructKey*    fieldKey,
        IRType*         fieldType)
    {
        IRInst* operands[] = { fieldKey, fieldType };
        IRStructField* field = (IRStructField*) createInstWithTrailingArgs<IRInst>(
            this,
            kIROp_StructField,
            nullptr,
            0,
            nullptr,
            2,
            operands);

        if (structType)
        {
            field->insertAtEnd(structType);
        }

        return field;
    }

    IRGeneric* IRBuilder::createGeneric()
    {
        IRGeneric* irGeneric = createInst<IRGeneric>(
            this,
            kIROp_Generic,
            nullptr);
        return irGeneric;
    }

    IRGeneric* IRBuilder::emitGeneric()
    {
        auto irGeneric = createGeneric();
        addGlobalValue(this, irGeneric);
        return irGeneric;
    }

    IRBlock* IRBuilder::createBlock()
    {
        return createInst<IRBlock>(
            this,
            kIROp_Block,
            getBasicBlockType());
    }

    void IRBuilder::insertBlock(IRBlock* block)
    {
        // If we are emitting into a function
        // (or another value with code), then
        // append the block to the function and
        // set this block as the new parent for
        // subsequent instructions we insert.
        //
        // TODO: This should probably insert the block
        // after the current "insert into" block if
        // there is one. Right now we are always
        // adding the block to the end of the list,
        // which is technically valid (the ordering
        // of blocks doesn't affect the CFG topology),
        // but some later passes might assume the ordering
        // is significant in representing the intent
        // of the original code.
        //
        auto f = getFunc();
        if (f)
        {
            f->addBlock(block);
            setInsertInto(block);
        }
    }

    IRBlock* IRBuilder::emitBlock()
    {
        auto block = createBlock();
        insertBlock(block);
        return block;
    }

    IRParam* IRBuilder::createParam(
        IRType* type)
    {
        auto param = createInst<IRParam>(
            this,
            kIROp_Param,
            type);
        return param;
    }

    IRParam* IRBuilder::emitParam(
        IRType* type)
    {
        auto param = createParam(type);
        if (auto bb = getBlock())
        {
            bb->addParam(param);
        }
        return param;
    }

    IRVar* IRBuilder::emitVar(
        IRType*         type)
    {
        auto allocatedType = getPtrType(type);
        auto inst = createInst<IRVar>(
            this,
            kIROp_Var,
            allocatedType);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitLoad(
        IRInst*    ptr)
    {
        // Note: a `load` operation does not consider the rate
        // (if any) attached to its operand (see the use of `getDataType`
        // below). This means that a load from a rate-qualified
        // variable will still conceptually execute (and return
        // results) at the "default" rate of the parent function,
        // unless a subsequent analysis pass constraints it.

        IRType* valueType = nullptr;
        if(auto ptrType = as<IRPtrTypeBase>(ptr->getDataType()))
        {
            valueType = ptrType->getValueType();
        }
        else if(auto ptrLikeType = as<IRPointerLikeType>(ptr->getDataType()))
        {
            valueType = ptrLikeType->getElementType();
        }
        else
        {
            // Bad!
            SLANG_ASSERT(ptrType);
            return nullptr;
        }

        // Ugly special case: if the front-end created a variable with
        // type `Ptr<@R T>` instead of `@R Ptr<T>`, then the above
        // logic will yield `@R T` instead of `T`, and we need to
        // try and fix that up here.
        //
        // TODO: Lowering to the IR should be fixed to never create
        // that case: rate-qualified types should only be allowed
        // to appear as the type of an instruction, and should not
        // be allowed as operands to type constructors (except
        // in special cases we decide to allow).
        //
        if(auto rateType = as<IRRateQualifiedType>(valueType))
        {
            valueType = rateType->getValueType();
        }

        auto inst = createInst<IRLoad>(
            this,
            kIROp_Load,
            valueType,
            ptr);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitStore(
        IRInst* dstPtr,
        IRInst* srcVal)
    {
        auto inst = createInst<IRStore>(
            this,
            kIROp_Store,
            nullptr,
            dstPtr,
            srcVal);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitFieldExtract(
        IRType* type,
        IRInst* base,
        IRInst* field)
    {
        auto inst = createInst<IRFieldExtract>(
            this,
            kIROp_FieldExtract,
            type,
            base,
            field);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitFieldAddress(
        IRType* type,
        IRInst* base,
        IRInst* field)
    {
        auto inst = createInst<IRFieldAddress>(
            this,
            kIROp_FieldAddress,
            type,
            base,
            field);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitElementExtract(
        IRType* type,
        IRInst* base,
        IRInst* index)
    {
        auto inst = createInst<IRFieldAddress>(
            this,
            kIROp_getElement,
            type,
            base,
            index);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitElementAddress(
        IRType*     type,
        IRInst*    basePtr,
        IRInst*    index)
    {
        auto inst = createInst<IRFieldAddress>(
            this,
            kIROp_getElementPtr,
            type,
            basePtr,
            index);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitSwizzle(
        IRType*         type,
        IRInst*         base,
        UInt            elementCount,
        IRInst* const*  elementIndices)
    {
        auto inst = createInstWithTrailingArgs<IRSwizzle>(
            this,
            kIROp_swizzle,
            type,
            base,
            elementCount,
            elementIndices);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitSwizzle(
        IRType*         type,
        IRInst*         base,
        UInt            elementCount,
        UInt const*     elementIndices)
    {
        auto intType = getBasicType(BaseType::Int);

        IRInst* irElementIndices[4];
        for (UInt ii = 0; ii < elementCount; ++ii)
        {
            irElementIndices[ii] = getIntValue(intType, elementIndices[ii]);
        }

        return emitSwizzle(type, base, elementCount, irElementIndices);
    }


    IRInst* IRBuilder::emitSwizzleSet(
        IRType*         type,
        IRInst*         base,
        IRInst*         source,
        UInt            elementCount,
        IRInst* const*  elementIndices)
    {
        IRInst* fixedArgs[] = { base, source };
        UInt fixedArgCount = sizeof(fixedArgs) / sizeof(fixedArgs[0]);

        auto inst = createInstWithTrailingArgs<IRSwizzleSet>(
            this,
            kIROp_swizzleSet,
            type,
            fixedArgCount,
            fixedArgs,
            elementCount,
            elementIndices);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitSwizzleSet(
        IRType*         type,
        IRInst*         base,
        IRInst*         source,
        UInt            elementCount,
        UInt const*     elementIndices)
    {
        auto intType = getBasicType(BaseType::Int);

        IRInst* irElementIndices[4];
        for (UInt ii = 0; ii < elementCount; ++ii)
        {
            irElementIndices[ii] = getIntValue(intType, elementIndices[ii]);
        }

        return emitSwizzleSet(type, base, source, elementCount, irElementIndices);
    }

    IRInst* IRBuilder::emitSwizzledStore(
        IRInst*         dest,
        IRInst*         source,
        UInt            elementCount,
        IRInst* const*  elementIndices)
    {
        IRInst* fixedArgs[] = { dest, source };
        UInt fixedArgCount = sizeof(fixedArgs) / sizeof(fixedArgs[0]);

        auto inst = createInstImpl<IRSwizzledStore>(
            this,
            kIROp_SwizzledStore,
            nullptr,
            fixedArgCount,
            fixedArgs,
            elementCount,
            elementIndices);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitSwizzledStore(
        IRInst*         dest,
        IRInst*         source,
        UInt            elementCount,
        UInt const*     elementIndices)
    {
        auto intType = getBasicType(BaseType::Int);

        IRInst* irElementIndices[4];
        for (UInt ii = 0; ii < elementCount; ++ii)
        {
            irElementIndices[ii] = getIntValue(intType, elementIndices[ii]);
        }

        return emitSwizzledStore(dest, source, elementCount, irElementIndices);
    }

    IRInst* IRBuilder::emitReturn(
        IRInst*    val)
    {
        auto inst = createInst<IRReturnVal>(
            this,
            kIROp_ReturnVal,
            nullptr,
            val);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitReturn()
    {
        auto inst = createInst<IRReturnVoid>(
            this,
            kIROp_ReturnVoid,
            nullptr);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitUnreachable()
    {
        auto inst = createInst<IRUnreachable>(
            this,
            kIROp_Unreachable,
            nullptr);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitMissingReturn()
    {
        auto inst = createInst<IRMissingReturn>(
            this,
            kIROp_MissingReturn,
            nullptr);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitDiscard()
    {
        auto inst = createInst<IRDiscard>(
            this,
            kIROp_discard,
            nullptr);
        addInst(inst);
        return inst;
    }


    IRInst* IRBuilder::emitBranch(
        IRBlock*    pBlock)
    {
        auto inst = createInst<IRUnconditionalBranch>(
            this,
            kIROp_unconditionalBranch,
            nullptr,
            pBlock);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitBreak(
        IRBlock*    target)
    {
        return emitBranch(target);
    }

    IRInst* IRBuilder::emitContinue(
        IRBlock*    target)
    {
        return emitBranch(target);
    }

    IRInst* IRBuilder::emitLoop(
        IRBlock*    target,
        IRBlock*    breakBlock,
        IRBlock*    continueBlock)
    {
        IRInst* args[] = { target, breakBlock, continueBlock };
        UInt argCount = sizeof(args) / sizeof(args[0]);

        auto inst = createInst<IRLoop>(
            this,
            kIROp_loop,
            nullptr,
            argCount,
            args);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitBranch(
        IRInst*     val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock)
    {
        IRInst* args[] = { val, trueBlock, falseBlock };
        UInt argCount = sizeof(args) / sizeof(args[0]);

        auto inst = createInst<IRConditionalBranch>(
            this,
            kIROp_conditionalBranch,
            nullptr,
            argCount,
            args);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitIfElse(
        IRInst*     val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock,
        IRBlock*    afterBlock)
    {
        IRInst* args[] = { val, trueBlock, falseBlock, afterBlock };
        UInt argCount = sizeof(args) / sizeof(args[0]);

        auto inst = createInst<IRIfElse>(
            this,
            kIROp_ifElse,
            nullptr,
            argCount,
            args);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitIf(
        IRInst*    val,
        IRBlock*    trueBlock,
        IRBlock*    afterBlock)
    {
        return emitIfElse(val, trueBlock, afterBlock, afterBlock);
    }

    IRInst* IRBuilder::emitLoopTest(
        IRInst*     val,
        IRBlock*    bodyBlock,
        IRBlock*    breakBlock)
    {
        return emitIfElse(val, bodyBlock, breakBlock, bodyBlock);
    }

    IRInst* IRBuilder::emitSwitch(
        IRInst*         val,
        IRBlock*        breakLabel,
        IRBlock*        defaultLabel,
        UInt            caseArgCount,
        IRInst* const*  caseArgs)
    {
        IRInst* fixedArgs[] = { val, breakLabel, defaultLabel };
        UInt fixedArgCount = sizeof(fixedArgs) / sizeof(fixedArgs[0]);

        auto inst = createInstWithTrailingArgs<IRSwitch>(
            this,
            kIROp_Switch,
            nullptr,
            fixedArgCount,
            fixedArgs,
            caseArgCount,
            caseArgs);
        addInst(inst);
        return inst;
    }

    IRGlobalGenericParam* IRBuilder::emitGlobalGenericParam()
    {
        IRGlobalGenericParam* irGenericParam = createInst<IRGlobalGenericParam>(
            this,
            kIROp_GlobalGenericParam,
            nullptr);
        addGlobalValue(this, irGenericParam);
        return irGenericParam;
    }

    IRBindGlobalGenericParam* IRBuilder::emitBindGlobalGenericParam(
        IRInst* param,
        IRInst* val)
    {
        auto inst = createInst<IRBindGlobalGenericParam>(
            this,
            kIROp_BindGlobalGenericParam,
            nullptr,
            param,
            val);
        addInst(inst);
        return inst;
    }

    IRDecoration* IRBuilder::addDecoration(IRInst* value, IROp op, IRInst* const* operands, Int operandCount)
    {
        auto decoration = createInstWithTrailingArgs<IRDecoration>(
            this,
            op,
            getVoidType(),
            operandCount,
            operands);

        // Decoration order should not, in general, be semantically
        // meaningful, so we will elect to insert a new decoration
        // at the start of an instruction (constant time) rather
        // than at the end of any existing list of deocrations
        // (which would take time linear in the number of decorations).
        //
        // TODO: revisit this if maintaining decoration ordering
        // from input source code is desirable.
        //
        decoration->insertAtStart(value);

        return decoration;
    }


    void IRBuilder::addHighLevelDeclDecoration(IRInst* inst, Decl* decl)
    {
        auto ptrConst = getPtrValue(addRefObjectToFree(decl));
        addDecoration(inst, kIROp_HighLevelDeclDecoration, ptrConst);
    }

    void IRBuilder::addLayoutDecoration(IRInst* inst, Layout* layout)
    {
        auto ptrConst = getPtrValue(addRefObjectToFree(layout));
        addDecoration(inst, kIROp_LayoutDecoration, ptrConst);
    }

    //


    struct IRDumpContext
    {
        StringBuilder*  builder = nullptr;
        int             indent  = 0;
        IRDumpMode      mode    = IRDumpMode::Simplified;

        Dictionary<IRInst*, String> mapValueToName;
        Dictionary<String, UInt>    uniqueNameCounters;
        UInt                        uniqueIDCounter = 1;
    };

    static void dump(
        IRDumpContext*  context,
        char const*     text)
    {
        context->builder->append(text);
    }

    static void dump(
        IRDumpContext*  context,
        String const&   text)
    {
        context->builder->append(text);
    }

    /*
    static void dump(
        IRDumpContext*  context,
        UInt            val)
    {
        context->builder->append(UnambigousUInt(val));
    }
    */

    static void dump(
        IRDumpContext*          context,
        IntegerLiteralValue     val)
    {
        context->builder->append(val);
    }

    static void dump(
        IRDumpContext*              context,
        FloatingPointLiteralValue   val)
    {
        context->builder->append(val);
    }

    static void dumpIndent(
        IRDumpContext*  context)
    {
        for (int ii = 0; ii < context->indent; ++ii)
        {
            dump(context, "\t");
        }
    }

    bool opHasResult(IRInst* inst)
    {
        auto type = inst->getDataType();
        if (!type) return false;

        // As a bit of a hack right now, we need to check whether
        // the function returns the distinguished `Void` type,
        // since that is conceptually the same as "not returning
        // a value."
        if(type->op == kIROp_VoidType)
            return false;

        return true;
    }

    bool instHasUses(IRInst* inst)
    {
        return inst->firstUse != nullptr;
    }

    static void scrubName(
        String const& name,
        StringBuilder&  sb)
    {
        // Note: this function duplicates a lot of the logic
        // in `EmitVisitor::scrubName`, so we should consider
        // whether they can share code at some point.
        //
        // There is no requirement that assembly dumps and output
        // code follow the same model, though, so this is just
        // a nice-to-have rather than a maintenance problem
        // waiting to happen.

        // Allow an empty nam
        // Special case a name that is the empty string, just in case.
        if(name.Length() == 0)
        {
            sb.append('_');
        }

        int prevChar = -1;
        for(auto c : name)
        {
            if(c == '.')
            {
                c = '_';
            }

            if(((c >= 'a') && (c <= 'z'))
                || ((c >= 'A') && (c <= 'Z')))
            {
                // Ordinary ASCII alphabetic characters are assumed
                // to always be okay.
            }
            else if((c >= '0') && (c <= '9'))
            {
                // We don't want to allow a digit as the first
                // byte in a name.
                if(prevChar == -1)
                {
                    sb.append('_');
                }
            }
            else
            {
                // If we run into a character that wouldn't normally
                // be allowed in an identifier, we need to translate
                // it into something that *is* valid.
                //
                // Our solution for now will be very clumsy: we will
                // emit `x` and then the hexadecimal version of
                // the byte we were given.
                sb.append("x");
                sb.append(uint32_t((unsigned char) c), 16);

                // We don't want to apply the default handling below,
                // so skip to the top of the loop now.
                prevChar = c;
                continue;
            }

            sb.append(c);
            prevChar = c;
        }

        // If the whole thing ended with a digit, then add
        // a final `_` just to make sure that we can append
        // a unique ID suffix without risk of collisions.
        if(('0' <= prevChar) && (prevChar <= '9'))
        {
            sb.append('_');
        }
    }

    static String createName(
        IRDumpContext*  context,
        IRInst*         value)
    {
        if(auto nameHintDecoration = value->findDecoration<IRNameHintDecoration>())
        {
            String nameHint = nameHintDecoration->getName();

            StringBuilder sb;
            scrubName(nameHint, sb);

            String key = sb.ProduceString();
            UInt count = 0;
            context->uniqueNameCounters.TryGetValue(key, count);

            context->uniqueNameCounters[key] = count+1;

            if(count)
            {
                sb.append(count);
            }
            return sb.ProduceString();
        }
        else
        {
            StringBuilder sb;
            auto id = context->uniqueIDCounter++;
            sb.append(id);
            return sb.ProduceString();
        }
    }

    static String getName(
        IRDumpContext*  context,
        IRInst*         value)
    {
        String name;
        if (context->mapValueToName.TryGetValue(value, name))
            return name;

        name = createName(context, value);
        context->mapValueToName.Add(value, name);
        return name;
    }

    static void dumpID(
        IRDumpContext* context,
        IRInst*        inst)
    {
        if (!inst)
        {
            dump(context, "<null>");
            return;
        }

        if( opHasResult(inst) || instHasUses(inst) )
        {
            dump(context, "%");
            dump(context, getName(context, inst));
        }
        else
        {
            dump(context, "_");
        }
    }


    
    struct StringEncoder
    {
        static char getHexChar(int v)
        {
            return (v <= 9) ? char(v + '0') : char(v - 10 + 'A');
        }

        void flush(const char* pos)
        {
            if (pos > m_runStart)
            {
                m_builder->append(m_runStart, pos);
            }
            m_runStart = pos + 1;
        }

        void appendEscapedChar(const char* pos, char encodeChar)
        {
            flush(pos);
            const char chars[] = { '\\', encodeChar };
            m_builder->Append(chars, 2);
        }
        
        void appendAsHex(const char* pos)
        {
            flush(pos);

            const int v = *(const uint8_t*)pos;

            char buf[5];
            buf[0] = '\\';
            buf[1] = 'x';
            buf[2] = '0';

            buf[3] = getHexChar(v >> 4);
            buf[4] = getHexChar(v & 0xf);

            m_builder->Append(buf, 5);
        }

        StringEncoder(StringBuilder* builder, const char* start):
            m_runStart(start),
            m_builder(builder)
        {}

        StringBuilder* m_builder;
        const char* m_runStart;
    };

    static void dumpEncodeString(
        IRDumpContext*  context, 
        const UnownedStringSlice& slice)
    {
        // https://msdn.microsoft.com/en-us/library/69ze775t.aspx

        StringBuilder& builder = *context->builder;
        builder.Append('"');
        
        {
            const char* cur = slice.begin();
            StringEncoder encoder(&builder, cur);
            const char* end = slice.end();

            for (; cur < end; cur++)
            {
                const int8_t c = uint8_t(*cur);
                switch (c)
                {
                    case '\\':
                        encoder.appendEscapedChar(cur, '\\');
                        break;
                    case '"':
                        encoder.appendEscapedChar(cur, '"');
                        break;
                    case '\n': 
                        encoder.appendEscapedChar(cur, 'n');
                        break;
                    case '\t':
                        encoder.appendEscapedChar(cur, 't');
                        break;
                    case '\r':
                        encoder.appendEscapedChar(cur, 'r');
                        break;
                    case '\0':
                        encoder.appendEscapedChar(cur, '0');
                        break;
                    default:
                    {
                        if (c < 32)
                        {
                            encoder.appendAsHex(cur);
                        }
                        break;
                    }
                }
            }
            encoder.flush(end);
        }
        
        builder.Append('"');
    }

    static void dumpType(
        IRDumpContext*  context,
        IRType*         type);

    static bool shouldFoldInstIntoUses(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        // Never fold an instruction into its use site
        // in the "detailed" mode, so that we always
        // accurately reflect the structure of the IR.
        //
        if(context->mode == IRDumpMode::Detailed)
            return false;

        if(as<IRConstant>(inst))
            return true;

        if(as<IRType>(inst))
            return true;

        return false;
    }

    static void dumpInst(
        IRDumpContext*  context,
        IRInst*         inst);

    static void dumpInstBody(
        IRDumpContext*  context,
        IRInst*         inst);

    static void dumpInstExpr(
        IRDumpContext*  context,
        IRInst*         inst);

    static void dumpOperand(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        // TODO: we should have a dedicated value for the `undef` case
        if (!inst)
        {
            dumpID(context, inst);
            return;
        }

        if(shouldFoldInstIntoUses(context, inst))
        {
            dumpInstExpr(context, inst);
            return;
        }

        dumpID(context, inst);
    }

    static void dumpType(
        IRDumpContext*  context,
        IRType*         type)
    {
        if (!type)
        {
            dump(context, "_");
            return;
        }

        // TODO: we should consider some special-case printing
        // for types, so that the IR doesn't get too hard to read
        // (always having to back-reference for what a type expands to)
        dumpOperand(context, type);
    }

    static void dumpInstTypeClause(
        IRDumpContext*  context,
        IRType*         type)
    {
        dump(context, "\t: ");
        dumpType(context, type);

    }

    void dumpIRDecorations(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        for(auto dd : inst->getDecorations())
        {
            // Certain decorations aren't helpful to appear
            // in output dumps, so we will only include them
            // in the "detailed" dumping mode.
            //
            // For all other modes, we will check the opcode
            // and skip selected decorations.
            //
            if(context->mode != IRDumpMode::Detailed)
            {
                switch(dd->op)
                {
                default:
                    break;

                case kIROp_HighLevelDeclDecoration:
                case kIROp_LayoutDecoration:
                    continue;
                }
            }

            dump(context, "[");
            dumpInstBody(context, dd);
            dump(context, "]\n");

            dumpIndent(context);
        }
    }

    static void dumpBlock(
        IRDumpContext*  context,
        IRBlock*        block)
    {
        context->indent--;
        dump(context, "block ");
        dumpID(context, block);

        IRInst* inst = block->getFirstInst();

        // First walk through any `param` instructions,
        // so that we can format them nicely
        if (auto firstParam = as<IRParam>(inst))
        {
            dump(context, "(\n");
            context->indent += 2;

            for(;;)
            {
                auto param = as<IRParam>(inst);
                if (!param)
                    break;

                if (param != firstParam)
                    dump(context, ",\n");

                inst = inst->getNextInst();

                dumpIndent(context);
                dumpIRDecorations(context, param);
                dump(context, "param ");
                dumpID(context, param);
                dumpInstTypeClause(context, param->getFullType());
            }
            context->indent -= 2;
            dump(context, ")");
        }
        dump(context, ":\n");
        context->indent++;

        for(; inst; inst = inst->getNextInst())
        {
            dumpInst(context, inst);
        }
    }

    void dumpIRGlobalValueWithCode(
        IRDumpContext*          context,
        IRGlobalValueWithCode*  code)
    {
        auto opInfo = getIROpInfo(code->op);

        dumpIndent(context);
        dump(context, opInfo.name);
        dump(context, " ");
        dumpID(context, code);

        dumpInstTypeClause(context, code->getFullType());

        if (!code->getFirstBlock())
        {
            // Just a declaration.
            dump(context, ";\n");
            return;
        }

        dump(context, "\n");

        dumpIndent(context);
        dump(context, "{\n");
        context->indent++;

        for (auto bb = code->getFirstBlock(); bb; bb = bb->getNextBlock())
        {
            if (bb != code->getFirstBlock())
                dump(context, "\n");
            dumpBlock(context, bb);
        }

        context->indent--;
        dump(context, "}");
    }


    void dumpIRWitnessTableEntry(
        IRDumpContext*          context,
        IRWitnessTableEntry*    entry)
    {
        dump(context, "witness_table_entry(");
        dumpOperand(context, entry->requirementKey.get());
        dump(context, ",");
        dumpOperand(context, entry->satisfyingVal.get());
        dump(context, ")\n");
    }

    void dumpIRParentInst(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        auto opInfo = getIROpInfo(inst->op);

        dumpIndent(context);
        dump(context, opInfo.name);
        dump(context, " ");
        dumpID(context, inst);

        dumpInstTypeClause(context, inst->getFullType());

        if (!inst->getFirstChild())
        {
            // Empty.
            dump(context, ";\n");
            return;
        }

        dump(context, "\n");

        dumpIndent(context);
        dump(context, "{\n");
        context->indent++;

        for(auto child : inst->getChildren())
        {
            dumpInst(context, child);
        }

        context->indent--;
        dump(context, "}\n");
    }

    void dumpIRGeneric(
        IRDumpContext*  context,
        IRGeneric* witnessTable)
    {
        dump(context, "\n");
        dumpIndent(context);
        dump(context, "ir_witness_table ");
        dumpID(context, witnessTable);
        dump(context, "\n{\n");
        context->indent++;

        for (auto ii : witnessTable->getChildren())
        {
            dumpInst(context, ii);
        }

        context->indent--;
        dump(context, "}\n");
    }

    static void dumpInstExpr(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        if (!inst)
        {
            dump(context, "<null>");
            return;
        }

        auto op = inst->op;
        auto opInfo = getIROpInfo(op);

        // Special-case the literal instructions.
        if(auto irConst = as<IRConstant>(inst))
        {
            switch (op)
            {
            case kIROp_IntLit:
                dump(context, irConst->value.intVal);
                return;

            case kIROp_FloatLit:
                dump(context, irConst->value.floatVal);
                return;

            case kIROp_BoolLit:
                dump(context, irConst->value.intVal ? "true" : "false");
                return;

            case kIROp_StringLit:
                dumpEncodeString(context, irConst->getStringSlice());
                return;

            case kIROp_PtrLit:
                dump(context, "<ptr>");
                return;

            default:
                break;
            }
        }

        dump(context, opInfo.name);

        UInt argCount = inst->getOperandCount();

        if(argCount == 0)
            return;

        UInt ii = 0;

        // Special case: make printing of `call` a bit
        // nicer to look at
        if (inst->op == kIROp_Call && argCount > 0)
        {
            dump(context, " ");
            auto argVal = inst->getOperand(ii++);
            dumpOperand(context, argVal);
        }

        bool first = true;
        dump(context, "(");
        for (; ii < argCount; ++ii)
        {
            if (!first)
                dump(context, ", ");

            auto argVal = inst->getOperand(ii);

            dumpOperand(context, argVal);

            first = false;
        }

        dump(context, ")");

    }

    static void dumpInstBody(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        if (!inst)
        {
            dump(context, "<null>");
            return;
        }

        auto op = inst->op;

        dumpIRDecorations(context, inst);

        // There are several ops we want to special-case here,
        // so that they will be more pleasant to look at.
        //
        switch (op)
        {
        case kIROp_Func:
        case kIROp_GlobalVar:
        case kIROp_GlobalConstant:
        case kIROp_Generic:
            dumpIRGlobalValueWithCode(context, (IRGlobalValueWithCode*)inst);
            return;

        case kIROp_WitnessTable:
        case kIROp_StructType:
            dumpIRParentInst(context, inst);
            return;

        case kIROp_WitnessTableEntry:
            dumpIRWitnessTableEntry(context, (IRWitnessTableEntry*)inst);
            return;

        default:
            break;
        }

        // Okay, we have a seemingly "ordinary" op now
        auto dataType = inst->getDataType();
        auto rate = inst->getRate();

        if(rate)
        {
            dump(context, "@");
            dumpOperand(context, rate);
            dump(context, " ");
        }

        if(opHasResult(inst) || instHasUses(inst))
        {
            dump(context, "let  ");
            dumpID(context, inst);
            dumpInstTypeClause(context, dataType);
            dump(context, "\t= ");
        }
        else
        {
            // No result, okay...
        }

        dumpInstExpr(context, inst);
    }

    static void dumpInst(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        if(shouldFoldInstIntoUses(context, inst))
            return;

        dumpIndent(context);
        dumpInstBody(context, inst);
        dump(context, "\n");
    }

    void dumpIRModule(
        IRDumpContext*  context,
        IRModule*       module)
    {
        for(auto ii : module->getGlobalInsts())
        {
            dumpInst(context, ii);
        }
    }

    void printSlangIRAssembly(StringBuilder& builder, IRModule* module, IRDumpMode mode)
    {
        IRDumpContext context;
        context.builder = &builder;
        context.indent = 0;
        context.mode = mode;

        dumpIRModule(&context, module);
    }

    void dumpIR(IRInst* globalVal, ISlangWriter* writer, IRDumpMode mode)
    {
        StringBuilder sb;

        IRDumpContext context;
        context.builder = &sb;
        context.indent = 0;
        context.mode = mode;

        dumpInst(&context, globalVal);

        writer->write(sb.Buffer(), sb.Length());
        writer->flush();
    }

    String getSlangIRAssembly(IRModule* module, IRDumpMode mode)
    {
        StringBuilder sb;
        printSlangIRAssembly(sb, module, mode);
        return sb;
    }

    void dumpIR(IRModule* module, ISlangWriter* writer, IRDumpMode mode)
    {
        String ir = getSlangIRAssembly(module, mode);
        writer->write(ir.Buffer(), ir.Length());
        writer->flush();
    }

    //
    //
    //

    IRDecoration* IRInst::getFirstDecoration()
    {
        return as<IRDecoration>(getFirstDecorationOrChild());
    }

    IRDecoration* IRInst::getLastDecoration()
    {
        IRDecoration* decoration = getFirstDecoration();
        if (!decoration) return nullptr;

        while (auto nextDecoration = decoration->getNextDecoration())
            decoration = nextDecoration;

        return decoration;
    }

    IRInstList<IRDecoration> IRInst::getDecorations()
    {
        return IRInstList<IRDecoration>(
            getFirstDecoration(),
            getLastDecoration());
    }

    IRInst* IRInst::getFirstChild()
    {
        // The children come after any decorations,
        // so if there are any decorations, then the
        // first child is right after the last decoration.
        //
        if(auto lastDecoration = getLastDecoration())
            return lastDecoration->getNextInst();
        //
        // Otherwise, there must be no decorations, so
        // that the first "child or decoration" is a child.
        //
        return getFirstDecorationOrChild();
    }

    IRInst* IRInst::getLastChild()
    {
        // The children come after any decorations, so
        // that the last item in the list of children
        // and decorations is the last child *unless*
        // it is a decoration, in which case there are
        // no children.
        //
        auto lastChild = getLastDecorationOrChild();
        return as<IRDecoration>(lastChild) ? nullptr : lastChild;
    }


    IRRate* IRInst::getRate()
    {
        if(auto rateQualifiedType = as<IRRateQualifiedType>(getFullType()))
            return rateQualifiedType->getRate();

        return nullptr;
    }

    IRType* IRInst::getDataType()
    {
        auto type = getFullType();
        if(auto rateQualifiedType = as<IRRateQualifiedType>(type))
            return rateQualifiedType->getValueType();

        return type;
    }

    void IRInst::replaceUsesWith(IRInst* other)
    {
        // Safety check: don't try to replace something with itself.
        if(other == this)
            return;

        // We will walk through the list of uses for the current
        // instruction, and make them point to the other inst.
        IRUse* ff = firstUse;

        // No uses? Nothing to do.
        if(!ff)
            return;

        ff->debugValidate();

        IRUse* uu = ff;
        for(;;)
        {
            // The uses had better all be uses of this
            // instruction, or invariants are broken.
            SLANG_ASSERT(uu->get() == this);

            // Swap this use over to use the other value.
            uu->usedValue = other;

            // Try to move to the next use, but bail
            // out if we are at the last one.
            IRUse* nn = uu->nextUse;
            if( !nn )
                break;

            uu = nn;
        }

        // We are at the last use (and there must
        // be at least one, because we handled
        // the case of an empty list earlier).
        SLANG_ASSERT(uu);

        // Our job at this point is to splice
        // our list of uses onto the other
        // value's uses.
        //
        // If the value already had uses, then
        // we need to patch our new list onto
        // the front.
        if( auto nn = other->firstUse )
        {
            uu->nextUse = nn;
            nn->prevLink = &uu->nextUse;
        }

        // No matter what, our list of
        // uses will become the start
        // of the list of uses for
        // `other`
        other->firstUse = ff;
        ff->prevLink = &other->firstUse;

        // And `this` will have no uses any more.
        this->firstUse = nullptr;

        ff->debugValidate();
    }

    // Insert this instruction into the same basic block
    // as `other`, right before it.
    void IRInst::insertBefore(IRInst* other)
    {
        SLANG_ASSERT(other);
        _insertAt(other->getPrevInst(), other, other->getParent());
    }

    void IRInst::insertAtStart(IRInst* newParent)
    {
        SLANG_ASSERT(newParent);
        _insertAt(nullptr, newParent->getFirstDecorationOrChild(), newParent);
    }

    void IRInst::moveToStart()
    {
        auto p = parent;
        removeFromParent();
        insertAtStart(p);
    }

    void IRInst::_insertAt(IRInst* inPrev, IRInst* inNext, IRInst* inParent)
    {
        // Make sure this instruction has been removed from any previous parent
        this->removeFromParent();

        SLANG_ASSERT(inParent);
        SLANG_ASSERT(!inPrev || (inPrev->getNextInst() == inNext) && (inPrev->getParent() == inParent));
        SLANG_ASSERT(!inNext || (inNext->getPrevInst() == inPrev) && (inNext->getParent() == inParent));

        if( inPrev )
        {
            inPrev->next = this;
        }
        else
        {
            inParent->m_decorationsAndChildren.first = this;
        }

        if (inNext)
        {
            inNext->prev = this;
        }
        else
        {
            inParent->m_decorationsAndChildren.last = this;
        }

        this->prev = inPrev;
        this->next = inNext;
        this->parent = inParent;
    }

    void IRInst::insertAfter(IRInst* other)
    {
        SLANG_ASSERT(other);

        _insertAt(other, other->getNextInst(), other->getParent());
    }

    void IRInst::insertAtEnd(IRInst* newParent)
    {
        SLANG_ASSERT(newParent);
        _insertAt(newParent->getLastDecorationOrChild(), nullptr, newParent);
    }

    void IRInst::moveToEnd()
    {
        auto p = parent;
        removeFromParent();
        insertAtEnd(p);
    }

    // Remove this instruction from its parent block,
    // and then destroy it (it had better have no uses!)
    void IRInst::removeFromParent()
    {
        auto oldParent = getParent();

        // If we don't currently have a parent, then
        // we are doing fine.
        if(!oldParent)
            return;

        auto pp = getPrevInst();
        auto nn = getNextInst();

        if(pp)
        {
            SLANG_ASSERT(pp->getParent() == oldParent);
            pp->next = nn;
        }
        else
        {
            oldParent->m_decorationsAndChildren.first = nn;
        }

        if(nn)
        {
            SLANG_ASSERT(nn->getParent() == oldParent);
            nn->prev = pp;
        }
        else
        {
            oldParent->m_decorationsAndChildren.last = pp;
        }

        prev = nullptr;
        next = nullptr;
        parent = nullptr;
    }

    void IRInst::removeArguments()
    {
        typeUse.clear();
        for( UInt aa = 0; aa < operandCount; ++aa )
        {
            IRUse& use = getOperands()[aa];
            use.clear();
        }
    }

    // Remove this instruction from its parent block,
    // and then destroy it (it had better have no uses!)
    void IRInst::removeAndDeallocate()
    {
        removeFromParent();
        removeArguments();
        removeAndDeallocateAllDecorationsAndChildren();

        // Run destructor to be sure...
        this->~IRInst();
    }

    void IRInst::removeAndDeallocateAllDecorationsAndChildren()
    {
        IRInst* nextChild = nullptr;
        for( IRInst* child = getFirstDecorationOrChild(); child; child = nextChild )
        {
            nextChild = child->getNextInst();
            child->removeAndDeallocate();
        }
    }


    bool IRInst::mightHaveSideEffects()
    {
        // TODO: We should drive this based on flags specified
        // in `ir-inst-defs.h` isntead of hard-coding things here,
        // but this is good enough for now if we are conservative:

        if(as<IRType>(this))
            return false;

        if(as<IRConstant>(this))
            return false;

        switch(op)
        {
        // By default, assume that we might have side effects,
        // to safely cover all the instructions we haven't had time to think about.
        default:
            return true;

        case kIROp_Call:
            {
                // In the general case, a function call must be assumed to
                // have almost arbitrary side effects.
                //
                // However, it is possible that the callee can be identified,
                // and it may be a function with an attribute that explicitly
                // limits the side effects it is allowed to have.
                //
                // For now, we will explicitly check for the `[__readNone]`
                // attribute, which was used to mark functions that compute
                // their result strictly as a function of the arguments (and
                // not anything they point to, or other non-argument state).
                // Calls to such functions cannot have side effects (except
                // for things like stack overflow that abstract language models
                // tend to ignore), and can be subject to dead code elimination,
                // common subexpression elimination, etc.
                //
                auto call = cast<IRCall>(this);
                auto callee = getResolvedInstForDecorations(call->getCallee());
                if(callee->findDecoration<IRReadNoneDecoration>())
                {
                    return false;
                }
            }
            return true;

            // All of the cases for "global values" are side-effect-free.
        case kIROp_StructType:
        case kIROp_StructField:
        case kIROp_Func:
        case kIROp_Generic:
        case kIROp_GlobalVar:
        case kIROp_GlobalConstant:
        case kIROp_GlobalParam:
        case kIROp_StructKey:
        case kIROp_GlobalGenericParam:
        case kIROp_WitnessTable:
        case kIROp_WitnessTableEntry:
        case kIROp_Block:
            return false;

        case kIROp_Nop:
        case kIROp_Specialize:
        case kIROp_lookup_interface_method:
        case kIROp_Construct:
        case kIROp_makeVector:
        case kIROp_makeMatrix:
        case kIROp_makeArray:
        case kIROp_makeStruct:
        case kIROp_Load:    // We are ignoring the possibility of loads from bad addresses, or `volatile` loads
        case kIROp_FieldExtract:
        case kIROp_FieldAddress:
        case kIROp_getElement:
        case kIROp_getElementPtr:
        case kIROp_constructVectorFromScalar:
        case kIROp_swizzle:
        case kIROp_swizzleSet:  // Doesn't actually "set" anything - just returns the resulting vector
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        //case kIROp_Div:   // TODO: We could split out integer vs. floating-point div/mod and assume the floating-point cases have no side effects
        //case kIROp_Mod:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Greater:
        case kIROp_Less:
        case kIROp_Geq:
        case kIROp_Leq:
        case kIROp_BitAnd:
        case kIROp_BitXor:
        case kIROp_BitOr:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_Neg:
        case kIROp_Not:
        case kIROp_BitNot:
        case kIROp_Select:
        case kIROp_Dot:
        case kIROp_Mul_Vector_Matrix:
        case kIROp_Mul_Matrix_Vector:
        case kIROp_Mul_Matrix_Matrix:
            return false;
        }
    }

    IRModule* IRInst::getModule()
    {
        IRInst* ii = this;
        while(ii)
        {
            if(auto moduleInst = as<IRModuleInst>(ii))
                return moduleInst->module;

            ii = ii->getParent();
        }
        return nullptr;
    }

    //
    // IRType
    //

    IRType* unwrapArray(IRType* type)
    {
        IRType* t = type;
        while( auto arrayType = as<IRArrayTypeBase>(t) )
        {
            t = arrayType->getElementType();
        }
        return t;
    }

    IRTargetIntrinsicDecoration* findTargetIntrinsicDecoration(
        IRInst*        val,
        String const&   targetName)
    {
        for(auto dd : val->getDecorations())
        {
            if(dd->op != kIROp_TargetIntrinsicDecoration)
                continue;

            auto decoration = (IRTargetIntrinsicDecoration*) dd;
            if(String(decoration->getTargetName()) == targetName)
                return decoration;
        }

        return nullptr;
    }

#if 0
    IRFunc* cloneSimpleFuncWithoutRegistering(IRSpecContextBase* context, IRFunc* originalFunc)
    {
        auto clonedFunc = context->builder->createFunc();
        cloneFunctionCommon(context, clonedFunc, originalFunc, false);
        return clonedFunc;
    }
#endif

    IRInst* findGenericReturnVal(IRGeneric* generic)
    {
        auto lastBlock = generic->getLastBlock();
        if (!lastBlock)
            return nullptr;

        auto returnInst = as<IRReturnVal>(lastBlock->getTerminator());
        if (!returnInst)
            return nullptr;

        auto val = returnInst->getVal();
        return val;
    }

    IRInst* getResolvedInstForDecorations(IRInst* inst)
    {
        IRInst* candidate = inst;
        while(auto specInst = as<IRSpecialize>(candidate))
        {
            auto genericInst = as<IRGeneric>(specInst->getBase());
            if(!genericInst)
                break;

            auto returnVal = findGenericReturnVal(genericInst);
            if(!returnVal)
                break;

            candidate = returnVal;
        }
        return candidate;
    }

    bool isDefinition(
        IRInst* inVal)
    {
        IRInst* val = inVal;
        // unwrap any generic declarations to see
        // the value they return.
        for(;;)
        {
            auto genericInst = as<IRGeneric>(val);
            if(!genericInst)
                break;

            auto returnVal = findGenericReturnVal(genericInst);
            if(!returnVal)
                break;

            val = returnVal;
        }

        // TODO: the logic here should probably
        // be that anything with an `IRImportDecoration`
        // is considered to be a declaration rather than definition.

        switch (val->op)
        {
        case kIROp_WitnessTable:
        case kIROp_GlobalConstant:
        case kIROp_Func:
        case kIROp_Generic:
            return val->getFirstChild() != nullptr;

        case kIROp_StructType:
        case kIROp_GlobalVar:
        case kIROp_GlobalParam:
            return true;

        default:
            return false;
        }
    }

    void markConstExpr(
        IRBuilder*  builder,
        IRInst*     irValue)
    {
        // We will take an IR value with type `T`,
        // and turn it into one with type `@ConstExpr T`.

        // TODO: need to be careful if the value already has a rate
        // qualifier set.

        irValue->setFullType(
            builder->getRateQualifiedType(
                builder->getConstExprRate(),
                irValue->getDataType()));
    }
}
