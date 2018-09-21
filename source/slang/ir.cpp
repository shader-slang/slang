// ir.cpp
#include "ir.h"
#include "ir-insts.h"

#include "../core/basic.h"
#include "mangle.h"

namespace Slang
{
    struct IRSpecContext;

    IRGlobalValue* cloneGlobalValueWithMangledName(
        IRSpecContext*  context,
        Name*   mangledName,
        IRGlobalValue*  originalVal);

    struct IROpMapEntry
    {
        IROp        op;
        IROpInfo    info;
    };

    // TODO: We should ideally be speeding up the name->inst
    // mapping by using a dictionary, or even by pre-computing
    // a hash table to be stored as a `static const` array.
    static const IROpMapEntry kIROps[] =
    {
        
    // Main ops in order
#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    { kIROp_##ID, { #MNEMONIC, ARG_COUNT, FLAGS, } },

    // Pseudo ops
    { kIROp_Invalid,{ "invalid", 0, 0 } },
#define PSEUDO_INST(ID)  \
    { kIRPseudoOp_##ID, { #ID, 0, 0 } },
#include "ir-inst-defs.h"

    };

    IROpInfo getIROpInfo(IROp opIn)
    {
        int op = opIn & kIROp_Mask_OpMask;
        if (op & kIROp_Mask_IsPseudoOp)
        {
            SLANG_ASSERT(op < kIRPseudoOp_LastPlusOne);
            const int index = op - kIRPseudoOp_First;

            return kIROps[kIROpCount + index].info;


        }

        for (auto ee : kIROps)
        {
            if (ee.op == op)
                return ee.info;
        }

        return kIROps[0].info;
    }

    //

    IROp findIROp(char const* name)
    {
        for (auto ee : kIROps)
        {
            if (strcmp(name, ee.info.name) == 0)
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

    IRDecoration* IRInst::findDecorationImpl(IRDecorationOp decorationOp)
    {
        for( auto dd = firstDecoration; dd; dd = dd->next )
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
        auto lastParam = getLastParam();
        if (lastParam)
        {
            param->insertAfter(lastParam);
        }
        else
        {
            param->insertAtStart(this);
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
        case kIROp_unreachable:
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

        case kIROp_switch:
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

    IRParam* IRGlobalValueWithParams::getFirstParam()
    {
        auto entryBlock = getFirstBlock();
        if(!entryBlock) return nullptr;

        return entryBlock->getFirstParam();
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
        case kIROp_switch:
        case kIROp_unreachable:
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


    void IRBuilder::setInsertInto(IRParentInst* insertInto)
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
    IRParentInst* mergeCandidateParentsForHoistableInst(IRParentInst* left, IRParentInst* right)
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
        IRParentInst* parentNonBlock = nullptr;
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

        IRParentInst* parent = parentNonBlock;

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
        IRParentInst* parent = builder->getModule()->getModuleInst();

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

    UnownedStringSlice IRConstant::getStringSlice() const
    {
        assert(op == kIROp_StringLit);
        // If the transitory decoration is set, then this is uses the transitoryStringVal for the text storage.
        // This is typically used when we are using a transitory IRInst held on the stack (such that it can be looked up in cached), 
        // that just points to a string elsewhere, and NOT the typical normal style, where the string is held after the instruction in memory.
        if (firstDecoration && firstDecoration->op == kIRDecorationOp_Transitory)
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
            case kIROp_boolConst:
            case kIROp_FloatLit:
            case kIROp_IntLit:
            {
                SLANG_COMPILE_TIME_ASSERT(sizeof(IRFloatingPointValue) == sizeof(IRIntegerValue));
                // ... we can just compare as bits
                return value.intVal == rhs.value.intVal;
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
            case kIROp_boolConst:
            case kIROp_FloatLit:
            case kIROp_IntLit:
            {
                SLANG_COMPILE_TIME_ASSERT(sizeof(IRFloatingPointValue) == sizeof(IRIntegerValue));
                // ... we can just compare as bits
                return combineHash(code, Slang::GetHashCode(value.intVal));
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
        const size_t prefixSize = offsetof(IRConstant, value);

        switch (keyInst.op)
        {
            case kIROp_boolConst:
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
        keyInst.op = kIROp_boolConst;
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
        static IRDecoration stackDecoration = IRDecoration::make(kIRDecorationOp_Transitory);
        keyInst.firstDecoration = &stackDecoration;
            
        keyInst.op = kIROp_StringLit;
        keyInst.typeUse.usedValue = getStringType();
        
        IRConstant::StringSliceValue& dstSlice = keyInst.value.transitoryStringVal;
        dstSlice.chars = const_cast<char*>(inSlice.begin());
        dstSlice.numChars = uint32_t(inSlice.size());

        return static_cast<IRStringLit*>(findOrEmitConstant(this, keyInst));
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

    IRInst* IRBuilder::emitIntrinsicInst(
        IRType*         type,
        IROp            op,
        UInt            argCount,
        IRInst* const* args)
    {
        auto inst = createInstWithTrailingArgs<IRInst>(
            this,
            op,
            type,
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
        IRBuilder*      builder,
        IRGlobalValue*  value)
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


    IRWitnessTable * IRBuilder::lookupWitnessTable(Name* mangledName)
    {
        IRWitnessTable * result;
        if (sharedBuilder->witnessTableMap.TryGetValue(mangledName, result))
            return result;
        return nullptr;
    }


    void IRBuilder::registerWitnessTable(IRWitnessTable * table)
    {
        sharedBuilder->witnessTableMap[table->mangledName] = table;
    }

    IRBlock* IRBuilder::createBlock()
    {
        return createInst<IRBlock>(
            this,
            kIROp_Block,
            getBasicBlockType());
    }

    IRBlock* IRBuilder::emitBlock()
    {
        // Create a block
        auto bb = createBlock();

        // If we are emitting into a function
        // (or another value with code), then
        // append the block to the function and
        // set this block as the new parent for
        // subsequent instructions we insert.
        auto f = getFunc();
        if (f)
        {
            f->addBlock(bb);
            setInsertInto(bb);
        }
        return bb;
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

    IRNotePatchConstantFunc* IRBuilder::emitNotePatchConstantFunc(
        IRInst* func)
    {
        auto inst = createInst<IRNotePatchConstantFunc>(
            this,
            kIROp_NotePatchConstantFunc,
            nullptr,
            func);

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
            kIROp_unreachable,
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
            kIROp_switch,
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

    IRHighLevelDeclDecoration* IRBuilder::addHighLevelDeclDecoration(IRInst* inst, Decl* decl)
    {
        auto decoration = addDecoration<IRHighLevelDeclDecoration>(inst, kIRDecorationOp_HighLevelDecl);
        decoration->decl = decl;
        return decoration;
    }

    IRLayoutDecoration* IRBuilder::addLayoutDecoration(IRInst* inst, Layout* layout)
    {
        auto decoration = addDecoration<IRLayoutDecoration>(inst);
        decoration->layout = addRefObjectToFree(layout);
        return decoration;
    }

    //


    struct IRDumpContext
    {
        StringBuilder* builder;
        int     indent = 0;

        UInt                        idCounter = 1;
        Dictionary<IRInst*, UInt>  mapValueToID;
    };

    static void dump(
        IRDumpContext*  context,
        char const*     text)
    {
        context->builder->append(text);
    }

    static void dump(
        IRDumpContext*  context,
        UInt            val)
    {
        context->builder->append(val);

//        fprintf(context->file, "%llu", (unsigned long long)val);
    }

    static void dump(
        IRDumpContext*          context,
        IntegerLiteralValue     val)
    {
        context->builder->append(val);

//        fprintf(context->file, "%llu", (unsigned long long)val);
    }

    static void dump(
        IRDumpContext*              context,
        FloatingPointLiteralValue   val)
    {
        context->builder->append(val);

//        fprintf(context->file, "%llu", (unsigned long long)val);
    }

    static void dumpIndent(
        IRDumpContext*  context)
    {
        for (int ii = 0; ii < context->indent; ++ii)
        {
            dump(context, "\t");
        }
    }

    bool opHasResult(IRInst* inst);

    bool instHasUses(IRInst* inst)
    {
        return inst->firstUse != nullptr;
    }

    static UInt getID(
        IRDumpContext*  context,
        IRInst*         value)
    {
        UInt id = 0;
        if (context->mapValueToID.TryGetValue(value, id))
            return id;

        if (opHasResult(value) || instHasUses(value))
        {
            id = context->idCounter++;
        }

        context->mapValueToID.Add(value, id);
        return id;
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

        if (auto globalValue = as<IRGlobalValue>(inst))
        {
            auto mangledName = globalValue->mangledName;
            if(mangledName)
            {
                auto mangledNameText = getText(mangledName);
                if (mangledNameText.Length() > 0)
                {
                    dump(context, "@");
                    dump(context, mangledNameText.Buffer());
                    return;
                }
            }
        }

        UInt id = getID(context, inst);
        if (id)
        {
            dump(context, "%");
            dump(context, id);
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

    static void dumpDeclRef(
        IRDumpContext*          context,
        DeclRef<Decl> const&    declRef);

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

        switch (inst->op)
        {
        case kIROp_IntLit:
            dump(context, ((IRConstant*)inst)->value.intVal);
            return;

        case kIROp_FloatLit:
            dump(context, ((IRConstant*)inst)->value.floatVal);
            return;

        case kIROp_boolConst:
            dump(context, ((IRConstant*)inst)->value.intVal ? "true" : "false");
            return;
        case kIROp_StringLit:
            dumpEncodeString(context, static_cast<IRConstant*>(inst)->getStringSlice());
            return;

        default:
            break;
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

    static void dumpInst(
        IRDumpContext*  context,
        IRInst*         inst);

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

    void dumpIRDecorations(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        for( auto dd = inst->firstDecoration; dd; dd = dd->next )
        {
            switch( dd->op )
            {
            case kIRDecorationOp_Target:
                {
                    auto decoration = (IRTargetDecoration*) dd;

                    dump(context, "\n");
                    dumpIndent(context);
                    dump(context, "[target(");
                    dump(context, StringRepresentation::getData(decoration->targetName));
                    dump(context, ")]");
                }
                break;

            }
        }
    }

    void dumpIRGlobalValueWithCode(
        IRDumpContext*          context,
        IRGlobalValueWithCode*  code)
    {
        // TODO: should apply this to all instructions
        dumpIRDecorations(context, code);

        auto opInfo = getIROpInfo(code->op);

        dump(context, "\n");
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
        dump(context, "}\n");
    }


    String dumpIRFunc(IRFunc* func)
    {
        IRDumpContext dumpContext;
        StringBuilder sbDump;
        dumpContext.builder = &sbDump;
        dumpIRGlobalValueWithCode(&dumpContext, func);
        auto strFunc = sbDump.ToString();
        return strFunc;
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
        IRParentInst*   inst)
    {
        // TODO: should apply this to all instructions
        dumpIRDecorations(context, inst);

        auto opInfo = getIROpInfo(inst->op);

        dump(context, "\n");
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

        for (auto child = inst->getFirstChild(); child; child = child->getNextInst())
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

    static void dumpInst(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        if (!inst)
        {
            dumpIndent(context);
            dump(context, "<null>");
            return;
        }

        auto op = inst->op;

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
            dumpIRParentInst(context, (IRWitnessTable*)inst);
            return;

        case kIROp_WitnessTableEntry:
            dumpIRWitnessTableEntry(context, (IRWitnessTableEntry*)inst);
            return;

        default:
            break;
        }

        // Okay, we have a seemingly "ordinary" op now
        dumpIndent(context);

        auto opInfo = getIROpInfo(op);
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

        dump(context, opInfo.name);

        UInt argCount = inst->getOperandCount();
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

        // Special cases: literals and other instructions with no real operands
        switch (inst->op)
        {
        case kIROp_IntLit:
        case kIROp_FloatLit:
        case kIROp_boolConst:
        case kIROp_StringLit:
            dumpOperand(context, inst);
            break;

        default:
            break;
        }

        dump(context, ")");

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

    void printSlangIRAssembly(StringBuilder& builder, IRModule* module)
    {
        IRDumpContext context;
        context.builder = &builder;
        context.indent = 0;

        dumpIRModule(&context, module);
    }

    void dumpIR(IRGlobalValue* globalVal)
    {
        StringBuilder sb;

        IRDumpContext context;
        context.builder = &sb;
        context.indent = 0;

        dumpInst(&context, globalVal);

        fprintf(stderr, "%s\n", sb.Buffer());
        fflush(stderr);
    }

    String getSlangIRAssembly(IRModule* module)
    {
        StringBuilder sb;
        printSlangIRAssembly(sb, module);
        return sb;
    }

    void dumpIR(IRModule* module)
    {
        String ir = getSlangIRAssembly(module);
        fprintf(stderr, "%s\n", ir.Buffer());
        fflush(stderr);
    }


    //
    //
    //

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
        insertBefore(other, other->parent);
    }

    void IRInst::insertAtStart(IRParentInst* newParent)
    {
        SLANG_ASSERT(newParent);
        insertBefore(newParent->children.first, newParent);
    }

    void IRInst::moveToStart()
    {
        auto p = parent;
        removeFromParent();
        insertAtStart(p);
    }

    void IRInst::insertBefore(IRInst* other, IRParentInst* newParent)
    {
        // Make sure this instruction has been removed from any previous parent
        this->removeFromParent();

        SLANG_ASSERT(other || newParent);
        if (!other) other = newParent->children.first;
        if (!newParent) newParent = other->parent;
        SLANG_ASSERT(newParent);

        auto nn = other;
        auto pp = other ? other->getPrevInst() : nullptr;

        if( pp )
        {
            pp->next = this;
        }
        else
        {
            newParent->children.first = this;
        }

        if (nn)
        {
            nn->prev = this;
        }
        else
        {
            newParent->children.last = this;
        }

        this->prev = pp;
        this->next = nn;
        this->parent = newParent;
    }

    void IRInst::insertAfter(IRInst* other)
    {
        SLANG_ASSERT(other);
        insertAfter(other, other->parent);
    }

    void IRInst::insertAtEnd(IRParentInst* newParent)
    {
        SLANG_ASSERT(newParent);
        insertAfter(newParent->children.last, newParent);
    }

    void IRInst::moveToEnd()
    {
        auto p = parent;
        removeFromParent();
        insertAtEnd(p);
    }

    void IRInst::insertAfter(IRInst* other, IRParentInst* newParent)
    {
        // Make sure this instruction has been removed from any previous parent
        this->removeFromParent();

        SLANG_ASSERT(other || newParent);
        if (!other) other = newParent->children.last;
        if (!newParent) newParent = other->parent;
        SLANG_ASSERT(newParent);

        auto pp = other;
        auto nn = other ? other->next : nullptr;

        if (pp)
        {
            pp->next = this;
        }
        else
        {
            newParent->children.first = this;
        }

        if (nn)
        {
            nn->prev = this;
        }
        else
        {
            newParent->children.last = this;
        }

        this->prev = pp;
        this->next = nn;
        this->parent = newParent;
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
            oldParent->children.first = nn;
        }

        if(nn)
        {
            SLANG_ASSERT(nn->getParent() == oldParent);
            nn->prev = pp;
        }
        else
        {
            oldParent->children.last = pp;
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

        // Run destructor to be sure...
        this->~IRInst();
    }

    bool IRInst::mightHaveSideEffects()
    {
        // TODO: We should drive this based on flags specified
        // in `ir-inst-defs.h` isntead of hard-coding things here,
        // but this is good enough for now if we are conservative:

        if(as<IRType>(this))
            return false;

        if(as<IRGlobalValue>(this))
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
            // This is the most interesting.
            return true;

        case kIROp_Nop:
        case kIROp_Specialize:
        case kIROp_lookup_interface_method:
        case kIROp_Construct:
        case kIROp_makeVector:
        case kIROp_makeMatrix:
        case kIROp_makeArray:
        case kIROp_makeStruct:
        case kIROp_Load:    // We are ignoring the possibility of loads from bad addresses, or `volatile` loads
        case kIROp_BufferLoad:
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
    // Legalization of entry points for GLSL:
    //

    IRGlobalVar* addGlobalVariable(
        IRModule*   module,
        IRType*     valueType)
    {
        auto session = module->session;

        SharedIRBuilder shared;
        shared.module = module;
        shared.session = session;

        IRBuilder builder;
        builder.sharedBuilder = &shared;
        return builder.createGlobalVar(valueType);
    }

    void moveValueBefore(
        IRGlobalValue*  valueToMove,
        IRGlobalValue*  placeBefore)
    {
        valueToMove->removeFromParent();
        valueToMove->insertBefore(placeBefore);
    }

    // When scalarizing shader inputs/outputs for GLSL, we need a way
    // to refer to a conceptual "value" that might comprise multiple
    // IR-level values. We could in principle introduce tuple types
    // into the IR so that everything stays at the IR level, but
    // it seems easier to just layer it over the top for now.
    //
    // The `ScalarizedVal` type deals with the "tuple or single value?"
    // question, and also the "l-value or r-value?" question.
    struct ScalarizedValImpl : RefObject
    {};
    struct ScalarizedTupleValImpl;
    struct ScalarizedTypeAdapterValImpl;
    struct ScalarizedVal
    {
        enum class Flavor
        {
            // no value (null pointer)
            none,

            // A simple `IRInst*` that represents the actual value
            value,

            // An `IRInst*` that represents the address of the actual value
            address,

            // A `TupleValImpl` that represents zero or more `ScalarizedVal`s
            tuple,

            // A `TypeAdapterValImpl` that wraps a single `ScalarizedVal` and
            // represents an implicit type conversion applied to it on read
            // or write.
            typeAdapter,
        };

        // Create a value representing a simple value
        static ScalarizedVal value(IRInst* irValue)
        {
            ScalarizedVal result;
            result.flavor = Flavor::value;
            result.irValue = irValue;
            return result;
        }


        // Create a value representing an address
        static ScalarizedVal address(IRInst* irValue)
        {
            ScalarizedVal result;
            result.flavor = Flavor::address;
            result.irValue = irValue;
            return result;
        }

        static ScalarizedVal tuple(ScalarizedTupleValImpl* impl)
        {
            ScalarizedVal result;
            result.flavor = Flavor::tuple;
            result.impl = (ScalarizedValImpl*)impl;
            return result;
        }

        static ScalarizedVal typeAdapter(ScalarizedTypeAdapterValImpl* impl)
        {
            ScalarizedVal result;
            result.flavor = Flavor::typeAdapter;
            result.impl = (ScalarizedValImpl*)impl;
            return result;
        }

        Flavor                      flavor = Flavor::none;
        IRInst*                     irValue = nullptr;
        RefPtr<ScalarizedValImpl>   impl;
    };

    // This is the case for a value that is a "tuple" of other values
    struct ScalarizedTupleValImpl : ScalarizedValImpl
    {
        struct Element
        {
            IRStructKey*    key;
            ScalarizedVal   val;
        };

        IRType*         type;
        List<Element>   elements;
    };

    // This is the case for a value that is stored with one type,
    // but needs to present itself as having a different type
    struct ScalarizedTypeAdapterValImpl : ScalarizedValImpl
    {
        ScalarizedVal   val;
        IRType*         actualType;   // the actual type of `val`
        IRType*         pretendType;     // the type this value pretends to have
    };

    struct GlobalVaryingDeclarator
    {
        enum class Flavor
        {
            array,
        };

        Flavor                      flavor;
        IRInst*                     elementCount;
        GlobalVaryingDeclarator*    next;
    };

    struct GLSLSystemValueInfo
    {
        // The name of the built-in GLSL variable
        char const* name;

        // The name of an outer array that wraps
        // the variable, in the case of a GS input
        char const*     outerArrayName;

        // The required type of the built-in variable
        IRType*     requiredType;
    };

    void requireGLSLVersionImpl(
        ExtensionUsageTracker*  tracker,
        ProfileVersion          version);

    void requireGLSLExtension(
        ExtensionUsageTracker*  tracker,
        String const&           name);

    struct GLSLLegalizationContext
    {
        Session*                session;
        ExtensionUsageTracker*  extensionUsageTracker;
        DiagnosticSink*         sink;
        Stage                   stage;

        void requireGLSLExtension(String const& name)
        {
            Slang::requireGLSLExtension(extensionUsageTracker, name);
        }

        void requireGLSLVersion(ProfileVersion version)
        {
            Slang::requireGLSLVersionImpl(extensionUsageTracker, version);
        }

        Stage getStage()
        {
            return stage;
        }

        DiagnosticSink* getSink()
        {
            return sink;
        }

        IRBuilder* builder;
        IRBuilder* getBuilder() { return builder; }
    };

    GLSLSystemValueInfo* getGLSLSystemValueInfo(
        GLSLLegalizationContext*    context,
        VarLayout*                  varLayout,
        LayoutResourceKind          kind,
        Stage                       stage,
        GLSLSystemValueInfo*        inStorage)
    {
        char const* name = nullptr;
        char const* outerArrayName = nullptr;

        auto semanticNameSpelling = varLayout->systemValueSemantic;
        if(semanticNameSpelling.Length() == 0)
            return nullptr;

        auto semanticName = semanticNameSpelling.ToLower();

        IRType* requiredType = nullptr;

        if(semanticName == "sv_position")
        {
            // This semantic can either work like `gl_FragCoord`
            // when it is used as a fragment shader input, or
            // like `gl_Position` when used in other stages.
            //
            // Note: This isn't as simple as testing input-vs-output,
            // because a user might have a VS output `SV_Position`,
            // and then pass it along to a GS that reads it as input.
            //
            if( stage == Stage::Fragment
                && kind == LayoutResourceKind::VaryingInput )
            {
                name = "gl_FragCoord";
            }
            else if( stage == Stage::Geometry
                && kind == LayoutResourceKind::VaryingInput )
            {
                // As a GS input, the correct syntax is `gl_in[...].gl_Position`,
                // but that is not compatible with picking the array dimension later,
                // of course.
                outerArrayName = "gl_in";
                name = "gl_Position";
            }
            else
            {
                name = "gl_Position";
            }
        }
        else if(semanticName == "sv_target")
        {
            // Note: we do *not* need to generate some kind of `gl_`
            // builtin for fragment-shader outputs: they are just
            // ordinary `out` variables, with ordinary `location`s,
            // as far as GLSL is concerned.
            return nullptr;
        }
        else if(semanticName == "sv_clipdistance")
        {
            // TODO: type conversion is required here.
            name = "gl_ClipDistance";
        }
        else if(semanticName == "sv_culldistance")
        {
            context->requireGLSLExtension("ARB_cull_distance");

            // TODO: type conversion is required here.
            name = "gl_CullDistance";
        }
        else if(semanticName == "sv_coverage")
        {
            // TODO: deal with `gl_SampleMaskIn` when used as an input.

            // TODO: type conversion is required here.
            name = "gl_SampleMask";
        }
        else if(semanticName == "sv_depth")
        {
            name = "gl_FragDepth";
        }
        else if(semanticName == "sv_depthgreaterequal")
        {
            // TODO: layout(depth_greater) out float gl_FragDepth;
            name = "gl_FragDepth";
        }
        else if(semanticName == "sv_depthlessequal")
        {
            // TODO: layout(depth_greater) out float gl_FragDepth;
            name = "gl_FragDepth";
        }
        else if(semanticName == "sv_dispatchthreadid")
        {
            name = "gl_GlobalInvocationID";
        }
        else if(semanticName == "sv_domainlocation")
        {
            name = "gl_TessCoord";
        }
        else if(semanticName == "sv_groupid")
        {
            name = "gl_WorkGroupID";
        }
        else if(semanticName == "sv_groupindex")
        {
            name = "gl_LocalInvocationIndex";
        }
        else if(semanticName == "sv_groupthreadid")
        {
            name = "gl_LocalInvocationID";
        }
        else if(semanticName == "sv_gsinstanceid")
        {
            name = "gl_InvocationID";
        }
        else if(semanticName == "sv_instanceid")
        {
            name = "gl_InstanceIndex";
        }
        else if(semanticName == "sv_isfrontface")
        {
            name = "gl_FrontFacing";
        }
        else if(semanticName == "sv_outputcontrolpointid")
        {
            name = "gl_InvocationID";
        }
        else if(semanticName == "sv_primitiveid")
        {
            name = "gl_PrimitiveID";
        }
        else if (semanticName == "sv_rendertargetarrayindex")
        {
            switch (context->getStage())
            {
            case Stage::Geometry:
                context->requireGLSLVersion(ProfileVersion::GLSL_150);
                break;

            case Stage::Fragment:
                context->requireGLSLVersion(ProfileVersion::GLSL_430);
                break;

            default:
                context->requireGLSLVersion(ProfileVersion::GLSL_450);
                context->requireGLSLExtension("GL_ARB_shader_viewport_layer_array");
                break;
            }

            name = "gl_Layer";
            requiredType = context->getBuilder()->getBasicType(BaseType::Int);
        }
        else if (semanticName == "sv_sampleindex")
        {
            name = "gl_SampleID";
        }
        else if (semanticName == "sv_stencilref")
        {
            context->requireGLSLExtension("ARB_shader_stencil_export");
            name = "gl_FragStencilRef";
        }
        else if (semanticName == "sv_tessfactor")
        {
            name = "gl_TessLevelOuter";
        }
        else if (semanticName == "sv_vertexid")
        {
            name = "gl_VertexIndex";
        }
        else if (semanticName == "sv_viewportarrayindex")
        {
            name = "gl_ViewportIndex";
        }
        else if (semanticName == "nv_x_right")
        {
            context->requireGLSLVersion(ProfileVersion::GLSL_450);
            context->requireGLSLExtension("GL_NVX_multiview_per_view_attributes");

            // The actual output in GLSL is:
            //
            //    vec4 gl_PositionPerViewNV[];
            //
            // and is meant to support an arbitrary number of views,
            // while the HLSL case just defines a second position
            // output.
            //
            // For now we will hack this by:
            //   1. Mapping an `NV_X_Right` output to `gl_PositionPerViewNV[1]`
            //      (that is, just one element of the output array)
            //   2. Adding logic to copy the traditional `gl_Position` output
            //      over to `gl_PositionPerViewNV[0]`
            //

            name = "gl_PositionPerViewNV[1]";

//            shared->requiresCopyGLPositionToPositionPerView = true;
        }
        else if (semanticName == "nv_viewport_mask")
        {
            context->requireGLSLVersion(ProfileVersion::GLSL_450);
            context->requireGLSLExtension("GL_NVX_multiview_per_view_attributes");

            name = "gl_ViewportMaskPerViewNV";
//            globalVarExpr = createGLSLBuiltinRef("gl_ViewportMaskPerViewNV",
//                getUnsizedArrayType(getIntType()));
        }

        if( name )
        {
            inStorage->name = name;
            inStorage->outerArrayName = outerArrayName;
            inStorage->requiredType = requiredType;
            return inStorage;
        }

        context->getSink()->diagnose(varLayout->varDecl.getDecl()->loc, Diagnostics::unknownSystemValueSemantic, semanticNameSpelling);
        return nullptr;
    }

    ScalarizedVal createSimpleGLSLGlobalVarying(
        GLSLLegalizationContext*    context,
        IRBuilder*                  builder,
        IRType*                     inType,
        VarLayout*                  inVarLayout,
        TypeLayout*                 inTypeLayout,
        LayoutResourceKind          kind,
        Stage                       stage,
        UInt                        bindingIndex,
        GlobalVaryingDeclarator*    declarator)
    {
        // Check if we have a system value on our hands.
        GLSLSystemValueInfo systemValueInfoStorage;
        auto systemValueInfo = getGLSLSystemValueInfo(
            context,
            inVarLayout,
            kind,
            stage,
            &systemValueInfoStorage);

        IRType* type = inType;

        // A system-value semantic might end up needing to override the type
        // that the user specified.
        if( systemValueInfo && systemValueInfo->requiredType )
        {
            type = systemValueInfo->requiredType;
        }

        // Construct the actual type and type-layout for the global variable
        //
        RefPtr<TypeLayout> typeLayout = inTypeLayout;
        for( auto dd = declarator; dd; dd = dd->next )
        {
            // We only have one declarator case right now...
            SLANG_ASSERT(dd->flavor == GlobalVaryingDeclarator::Flavor::array);

            auto arrayType = builder->getArrayType(
                type,
                dd->elementCount);

            RefPtr<ArrayTypeLayout> arrayTypeLayout = new ArrayTypeLayout();
//            arrayTypeLayout->type = arrayType;
            arrayTypeLayout->rules = typeLayout->rules;
            arrayTypeLayout->originalElementTypeLayout =  typeLayout;
            arrayTypeLayout->elementTypeLayout = typeLayout;
            arrayTypeLayout->uniformStride = 0;

            if( auto resInfo = inTypeLayout->FindResourceInfo(kind) )
            {
                // TODO: it is kind of gross to be re-running some
                // of the type layout logic here.

                UInt elementCount = (UInt) GetIntVal(dd->elementCount);
                arrayTypeLayout->addResourceUsage(
                    kind,
                    resInfo->count * elementCount);
            }

            type = arrayType;
            typeLayout = arrayTypeLayout;
        }

        // We need to construct a fresh layout for the variable, even
        // if the original had its own layout, because it might be
        // an `inout` parameter, and we only want to deal with the case
        // described by our `kind` parameter.
        RefPtr<VarLayout> varLayout = new VarLayout();
        varLayout->varDecl = inVarLayout->varDecl;
        varLayout->typeLayout = typeLayout;
        varLayout->flags = inVarLayout->flags;
        varLayout->systemValueSemantic = inVarLayout->systemValueSemantic;
        varLayout->systemValueSemanticIndex = inVarLayout->systemValueSemanticIndex;
        varLayout->semanticName = inVarLayout->semanticName;
        varLayout->semanticIndex = inVarLayout->semanticIndex;
        varLayout->stage = inVarLayout->stage;
        varLayout->AddResourceInfo(kind)->index = bindingIndex;

        // Simple case: just create a global variable of the matching type,
        // and then use the value of the global as a replacement for the
        // value of the original parameter.
        //
        auto globalVariable = addGlobalVariable(builder->getModule(), type);
        moveValueBefore(globalVariable, builder->getFunc());

        ScalarizedVal val = ScalarizedVal::address(globalVariable);

        if( systemValueInfo )
        {
            globalVariable->mangledName = builder->getSession()->getNameObj(systemValueInfo->name);

            if( auto fromType = systemValueInfo->requiredType )
            {
                // We may need to adapt from the declared type to/from
                // the actual type of the GLSL global.
                auto toType = inType;

                if( fromType != toType )
                {
                    RefPtr<ScalarizedTypeAdapterValImpl> typeAdapter = new ScalarizedTypeAdapterValImpl;
                    typeAdapter->actualType = systemValueInfo->requiredType;
                    typeAdapter->pretendType = inType;
                    typeAdapter->val = val;

                    val = ScalarizedVal::typeAdapter(typeAdapter);
                }
            }

            if(auto outerArrayName = systemValueInfo->outerArrayName)
            {
                auto decoration = builder->addDecoration<IRGLSLOuterArrayDecoration>(globalVariable);
                decoration->outerArrayName = outerArrayName;
            }
        }

        builder->addLayoutDecoration(globalVariable, varLayout);

        return val;
    }

    ScalarizedVal createGLSLGlobalVaryingsImpl(
        GLSLLegalizationContext*    context,
        IRBuilder*                  builder,
        IRType*                     type,
        VarLayout*                  varLayout,
        TypeLayout*                 typeLayout,
        LayoutResourceKind          kind,
        Stage                       stage,
        UInt                        bindingIndex,
        GlobalVaryingDeclarator*    declarator)
    {
        if( as<IRBasicType>(type) )
        {
            return createSimpleGLSLGlobalVarying(
                context,
                builder, type, varLayout, typeLayout, kind, stage, bindingIndex, declarator);
        }
        else if( as<IRVectorType>(type) )
        {
            return createSimpleGLSLGlobalVarying(
                context,
                builder, type, varLayout, typeLayout, kind, stage, bindingIndex, declarator);
        }
        else if( as<IRMatrixType>(type) )
        {
            // TODO: a matrix-type varying should probably be handled like an array of rows
            return createSimpleGLSLGlobalVarying(
                context,
                builder, type, varLayout, typeLayout, kind, stage, bindingIndex, declarator);
        }
        else if( auto arrayType = as<IRArrayType>(type) )
        {
            // We will need to SOA-ize any nested types.

            auto elementType = arrayType->getElementType();
            auto elementCount = arrayType->getElementCount();
            auto arrayLayout = dynamic_cast<ArrayTypeLayout*>(typeLayout);
            SLANG_ASSERT(arrayLayout);
            auto elementTypeLayout = arrayLayout->elementTypeLayout;

            GlobalVaryingDeclarator arrayDeclarator;
            arrayDeclarator.flavor = GlobalVaryingDeclarator::Flavor::array;
            arrayDeclarator.elementCount = elementCount;
            arrayDeclarator.next = declarator;

            return createGLSLGlobalVaryingsImpl(
                context,
                builder,
                elementType,
                varLayout,
                elementTypeLayout,
                kind,
                stage,
                bindingIndex,
                &arrayDeclarator);
        }
        else if( auto streamType = as<IRHLSLStreamOutputType>(type))
        {
            auto elementType = streamType->getElementType();
            auto streamLayout = dynamic_cast<StreamOutputTypeLayout*>(typeLayout);
            SLANG_ASSERT(streamLayout);
            auto elementTypeLayout = streamLayout->elementTypeLayout;

            return createGLSLGlobalVaryingsImpl(
                context,
                builder,
                elementType,
                varLayout,
                elementTypeLayout,
                kind,
                stage,
                bindingIndex,
                declarator);
        }
        else if(auto structType = as<IRStructType>(type))
        {
            // We need to recurse down into the individual fields,
            // and generate a variable for each of them.

            auto structTypeLayout = dynamic_cast<StructTypeLayout*>(typeLayout);
            SLANG_ASSERT(structTypeLayout);
            RefPtr<ScalarizedTupleValImpl> tupleValImpl = new ScalarizedTupleValImpl();


            // Construct the actual type for the tuple (including any outer arrays)
            IRType* fullType = type;
            for( auto dd = declarator; dd; dd = dd->next )
            {
                SLANG_ASSERT(dd->flavor == GlobalVaryingDeclarator::Flavor::array);
                fullType = builder->getArrayType(
                    fullType,
                    dd->elementCount);
            }

            tupleValImpl->type = fullType;

            // Okay, we want to walk through the fields here, and
            // generate one variable for each.
            UInt fieldCounter = 0;
            for(auto field : structType->getFields())
            {
                UInt fieldIndex = fieldCounter++;

                auto fieldLayout = structTypeLayout->fields[fieldIndex];

                UInt fieldBindingIndex = bindingIndex;
                if(auto fieldResInfo = fieldLayout->FindResourceInfo(kind))
                    fieldBindingIndex += fieldResInfo->index;

                auto fieldVal = createGLSLGlobalVaryingsImpl(
                    context,
                    builder,
                    field->getFieldType(),
                    fieldLayout,
                    fieldLayout->typeLayout,
                    kind,
                    stage,
                    fieldBindingIndex,
                    declarator);

                ScalarizedTupleValImpl::Element element;
                element.val = fieldVal;
                element.key = field->getKey();

                tupleValImpl->elements.Add(element);
            }

            return ScalarizedVal::tuple(tupleValImpl);
        }

        // Default case is to fall back on the simple behavior
        return createSimpleGLSLGlobalVarying(
            context,
            builder, type, varLayout, typeLayout, kind, stage, bindingIndex, declarator);
    }

    ScalarizedVal createGLSLGlobalVaryings(
        GLSLLegalizationContext*    context,
        IRBuilder*                  builder,
        IRType*                     type,
        VarLayout*                  layout,
        LayoutResourceKind          kind,
        Stage                       stage)
    {
        UInt bindingIndex = 0;
        if(auto rr = layout->FindResourceInfo(kind))
            bindingIndex = rr->index;
        return createGLSLGlobalVaryingsImpl(
            context,
            builder, type, layout, layout->typeLayout, kind, stage, bindingIndex, nullptr);
    }

    IRType* getFieldType(
        IRType*         baseType,
        IRStructKey*    fieldKey)
    {
        if(auto structType = as<IRStructType>(baseType))
        {
            for(auto ff : structType->getFields())
            {
                if(ff->getKey() == fieldKey)
                    return ff->getFieldType();
            }
        }

        SLANG_UNEXPECTED("no such field");
        UNREACHABLE_RETURN(nullptr);
    }

    ScalarizedVal extractField(
        IRBuilder*              builder,
        ScalarizedVal const&    val,
        UInt                    fieldIndex,
        IRStructKey*            fieldKey)
    {
        switch( val.flavor )
        {
        case ScalarizedVal::Flavor::value:
            return ScalarizedVal::value(
                builder->emitFieldExtract(
                    getFieldType(val.irValue->getDataType(), fieldKey),
                    val.irValue,
                    fieldKey));

        case ScalarizedVal::Flavor::address:
            return ScalarizedVal::address(
                builder->emitFieldAddress(
                    getFieldType(val.irValue->getDataType(), fieldKey),
                    val.irValue,
                    fieldKey));

        case ScalarizedVal::Flavor::tuple:
            {
                auto tupleVal = val.impl.As<ScalarizedTupleValImpl>();
                return tupleVal->elements[fieldIndex].val;
            }

        default:
            SLANG_UNEXPECTED("unimplemented");
            UNREACHABLE_RETURN(ScalarizedVal());
        }

    }

    ScalarizedVal adaptType(
        IRBuilder*              builder,
        IRInst*                 val,
        IRType*                 toType,
        IRType*                 /*fromType*/)
    {
        // TODO: actually consider what needs to go on here...
        return ScalarizedVal::value(builder->emitConstructorInst(
            toType,
            1,
            &val));
    }

    ScalarizedVal adaptType(
        IRBuilder*              builder,
        ScalarizedVal const&    val,
        IRType*                 toType,
        IRType*                 fromType)
    {
        switch( val.flavor )
        {
        case ScalarizedVal::Flavor::value:
            return adaptType(builder, val.irValue, toType, fromType);
            break;

        case ScalarizedVal::Flavor::address:
            {
                auto loaded = builder->emitLoad(val.irValue);
                return adaptType(builder, loaded, toType, fromType);
            }
            break;

        default:
            SLANG_UNEXPECTED("unimplemented");
            UNREACHABLE_RETURN(ScalarizedVal());
        }
    }

    void assign(
        IRBuilder*              builder,
        ScalarizedVal const&    left,
        ScalarizedVal const&    right)
    {
        switch( left.flavor )
        {
        case ScalarizedVal::Flavor::address:
            switch( right.flavor )
            {
            case ScalarizedVal::Flavor::value:
                {
                    builder->emitStore(left.irValue, right.irValue);
                }
                break;

            case ScalarizedVal::Flavor::address:
                {
                    auto val = builder->emitLoad(right.irValue);
                    builder->emitStore(left.irValue, val);
                }
                break;

            case ScalarizedVal::Flavor::tuple:
                {
                    // We are assigning from a tuple to a destination
                    // that is not a tuple. We will perform assignment
                    // element-by-element.
                    auto rightTupleVal = right.impl.As<ScalarizedTupleValImpl>();
                    UInt elementCount = rightTupleVal->elements.Count();

                    for( UInt ee = 0; ee < elementCount; ++ee )
                    {
                        auto rightElement = rightTupleVal->elements[ee];
                        auto leftElementVal = extractField(
                            builder,
                            left,
                            ee,
                            rightElement.key);
                        assign(builder, leftElementVal, rightElement.val);
                    }
                }
                break;

            default:
                SLANG_UNEXPECTED("unimplemented");
                break;
            }
            break;

        case ScalarizedVal::Flavor::tuple:
            {
                // We have a tuple, so we are going to need to try and assign
                // to each of its constituent fields.
                auto leftTupleVal = left.impl.As<ScalarizedTupleValImpl>();
                UInt elementCount = leftTupleVal->elements.Count();

                for( UInt ee = 0; ee < elementCount; ++ee )
                {
                    auto rightElementVal = extractField(
                        builder,
                        right,
                        ee,
                        leftTupleVal->elements[ee].key);
                    assign(builder, leftTupleVal->elements[ee].val, rightElementVal);
                }
            }
            break;

        case ScalarizedVal::Flavor::typeAdapter:
            {
                // We are trying to assign to something that had its type adjusted,
                // so we will need to adjust the type of the right-hand side first.
                //
                // In this case we are converting to the actual type of the GLSL variable,
                // from the "pretend" type that it had in the IR before.
                auto typeAdapter = left.impl.As<ScalarizedTypeAdapterValImpl>();
                auto adaptedRight = adaptType(builder, right, typeAdapter->actualType, typeAdapter->pretendType);
                assign(builder, typeAdapter->val, adaptedRight);
            }
            break;

        default:
            SLANG_UNEXPECTED("unimplemented");
            break;
        }
    }

    ScalarizedVal getSubscriptVal(
        IRBuilder*      builder,
        IRType*         elementType,
        ScalarizedVal   val,
        IRInst*         indexVal)
    {
        switch( val.flavor )
        {
        case ScalarizedVal::Flavor::value:
            return ScalarizedVal::value(
                builder->emitElementExtract(
                    elementType,
                    val.irValue,
                    indexVal));

        case ScalarizedVal::Flavor::address:
            return ScalarizedVal::address(
                builder->emitElementAddress(
                    builder->getPtrType(elementType),
                    val.irValue,
                    indexVal));

        case ScalarizedVal::Flavor::tuple:
            {
                auto inputTuple = val.impl.As<ScalarizedTupleValImpl>();

                RefPtr<ScalarizedTupleValImpl> resultTuple = new ScalarizedTupleValImpl();
                resultTuple->type = elementType;

                UInt elementCount = inputTuple->elements.Count();
                UInt elementCounter = 0;

                auto structType = as<IRStructType>(elementType);
                for(auto field : structType->getFields())
                {
                    auto tupleElementType = field->getFieldType();

                    UInt elementIndex = elementCounter++;

                    SLANG_RELEASE_ASSERT(elementIndex < elementCount);
                    auto inputElement = inputTuple->elements[elementIndex];

                    ScalarizedTupleValImpl::Element resultElement;
                    resultElement.key = inputElement.key;
                    resultElement.val = getSubscriptVal(
                        builder,
                        tupleElementType,
                        inputElement.val,
                        indexVal);

                    resultTuple->elements.Add(resultElement);
                }
                SLANG_RELEASE_ASSERT(elementCounter == elementCount);

                return ScalarizedVal::tuple(resultTuple);
            }

        default:
            SLANG_UNEXPECTED("unimplemented");
            UNREACHABLE_RETURN(ScalarizedVal());
        }
    }

    ScalarizedVal getSubscriptVal(
        IRBuilder*      builder,
        IRType*         elementType,
        ScalarizedVal   val,
        UInt            index)
    {
        return getSubscriptVal(
            builder,
            elementType,
            val,
            builder->getIntValue(
                builder->getIntType(),
                index));
    }

    IRInst* materializeValue(
        IRBuilder*              builder,
        ScalarizedVal const&    val);

    IRInst* materializeTupleValue(
        IRBuilder*      builder,
        ScalarizedVal   val)
    {
        auto tupleVal = val.impl.As<ScalarizedTupleValImpl>();
        SLANG_ASSERT(tupleVal);

        UInt elementCount = tupleVal->elements.Count();
        auto type = tupleVal->type;

        if( auto arrayType = as<IRArrayType>(type))
        {
            // The tuple represent an array, which means that the
            // individual elements are expected to yield arrays as well.
            //
            // We will extract a value for each array element, and
            // then use these to construct our result.

            List<IRInst*> arrayElementVals;
            UInt arrayElementCount = (UInt) GetIntVal(arrayType->getElementCount());

            for( UInt ii = 0; ii < arrayElementCount; ++ii )
            {
                auto arrayElementPseudoVal = getSubscriptVal(
                    builder,
                    arrayType->getElementType(),
                    val,
                    ii);

                auto arrayElementVal = materializeValue(
                    builder,
                    arrayElementPseudoVal);

                arrayElementVals.Add(arrayElementVal);
            }

            return builder->emitMakeArray(
                arrayType,
                arrayElementVals.Count(),
                arrayElementVals.Buffer());
        }
        else
        {
            // The tuple represents a value of some aggregate type,
            // so we can simply materialize the elements and then
            // construct a value of that type.
            //
            // TODO: this should be using a `makeStruct` instruction.

            List<IRInst*> elementVals;
            for( UInt ee = 0; ee < elementCount; ++ee )
            {
                auto elementVal = materializeValue(builder, tupleVal->elements[ee].val);
                elementVals.Add(elementVal);
            }

            return builder->emitConstructorInst(
                tupleVal->type,
                elementVals.Count(),
                elementVals.Buffer());
        }
    }

    IRInst* materializeValue(
        IRBuilder*              builder,
        ScalarizedVal const&    val)
    {
        switch( val.flavor )
        {
        case ScalarizedVal::Flavor::value:
            return val.irValue;

        case ScalarizedVal::Flavor::address:
            {
                auto loadInst = builder->emitLoad(val.irValue);
                return loadInst;
            }
            break;

        case ScalarizedVal::Flavor::tuple:
            {
                auto tupleVal = val.impl.As<ScalarizedTupleValImpl>();
                return materializeTupleValue(builder, val);
            }
            break;

        case ScalarizedVal::Flavor::typeAdapter:
            {
                // Somebody is trying to use a value where its actual type
                // doesn't match the type it pretends to have. To make this
                // work we need to adapt the type from its actual type over
                // to its pretend type.
                auto typeAdapter = val.impl.As<ScalarizedTypeAdapterValImpl>();
                auto adapted = adaptType(builder, typeAdapter->val, typeAdapter->pretendType, typeAdapter->actualType);
                return materializeValue(builder, adapted);
            }
            break;

        default:
            SLANG_UNEXPECTED("unimplemented");
            break;
        }
    }

    IRTargetIntrinsicDecoration* findTargetIntrinsicDecoration(
        IRInst*        val,
        String const&   targetName)
    {
        for( auto dd = val->firstDecoration; dd; dd = dd->next )
        {
            if(dd->op != kIRDecorationOp_TargetIntrinsic)
                continue;

            auto decoration = (IRTargetIntrinsicDecoration*) dd;
            if(String(decoration->targetName) == targetName)
                return decoration;
        }

        return nullptr;
    }

    void legalizeEntryPointForGLSL(
        Session*                session,
        IRModule*               module,
        IRFunc*                 func,
        EntryPointLayout*       entryPointLayout,
        DiagnosticSink*         sink,
        ExtensionUsageTracker*  extensionUsageTracker)
    {
        GLSLLegalizationContext context;
        context.session = session;
        context.stage = entryPointLayout->profile.GetStage();
        context.sink = sink;
        context.extensionUsageTracker = extensionUsageTracker;

        Stage stage = entryPointLayout->profile.GetStage();

        // We require that the entry-point function has no uses,
        // because otherwise we'd invalidate the signature
        // at all existing call sites.
        //
        // TODO: the right thing to do here is to split any
        // function that both gets called as an entry point
        // and as an ordinary function.
        SLANG_ASSERT(!func->firstUse);

        // We create a dummy IR builder, since some of
        // the functions require it.
        //
        // TODO: make some of these free functions...
        //
        SharedIRBuilder shared;
        shared.module = module;
        shared.session = session;
        IRBuilder builder;
        builder.sharedBuilder = &shared;
        builder.setInsertInto(func);

        context.builder = &builder;

        // We will start by looking at the return type of the
        // function, because that will enable us to do an
        // early-out check to avoid more work.
        //
        // Specifically, we need to check if the function has
        // a `void` return type, because there is no work
        // to be done on its return value in that case.
        auto resultType = func->getResultType();
        if(as<IRVoidType>(resultType))
        {
            // In this case, the function doesn't return a value
            // so we don't need to transform its `return` sites.
            //
            // We can also use this opportunity to quickly
            // check if the function has any parameters, and if
            // it doesn't use the chance to bail out immediately.
            if( func->getParamCount() == 0 )
            {
                // This function is already legal for GLSL
                // (at least in terms of parameter/result signature),
                // so we won't bother doing anything at all.
                return;
            }

            // If the function does have parameters, then we need
            // to let the logic later in this function handle them.
        }
        else
        {
            // Function returns a value, so we need
            // to introduce a new global variable
            // to hold that value, and then replace
            // any `returnVal` instructions with
            // code to write to that variable.

            auto resultGlobal = createGLSLGlobalVaryings(
                &context,
                &builder,
                resultType,
                entryPointLayout->resultLayout,
                LayoutResourceKind::VaryingOutput,
                stage);

            for( auto bb = func->getFirstBlock(); bb; bb = bb->getNextBlock() )
            {
                // TODO: This is silly, because we are looking at every instruction,
                // when we know that a `returnVal` should only ever appear as a
                // terminator...
                for( auto ii = bb->getFirstInst(); ii; ii = ii->getNextInst() )
                {
                    if(ii->op != kIROp_ReturnVal)
                        continue;

                    IRReturnVal* returnInst = (IRReturnVal*) ii;
                    IRInst* returnValue = returnInst->getVal();

                    // Make sure we add these instructions to the right block
                    builder.setInsertInto(bb);

                    // Write to our global variable(s) from the value being returned.
                    assign(&builder, resultGlobal, ScalarizedVal::value(returnValue));

                    // Emit a `returnVoid` to end the block
                    auto returnVoid = builder.emitReturn();

                    // Remove the old `returnVal` instruction.
                    returnInst->removeAndDeallocate();

                    // Make sure to resume our iteration at an
                    // appropriate instruciton, since we deleted
                    // the one we had been using.
                    ii = returnVoid;
                }
            }
        }

        // Next we will walk through any parameters of the entry-point function,
        // and turn them into global variables.
        if( auto firstBlock = func->getFirstBlock() )
        {
            // Any initialization code we insert for parameters needs
            // to be at the start of the "ordinary" instructions in the block:
            builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());

            UInt paramCounter = 0;
            for( auto pp = firstBlock->getFirstParam(); pp; pp = pp->getNextParam() )
            {
                UInt paramIndex = paramCounter++;

                // We assume that the entry-point layout includes information
                // on each parameter, and that these arrays are kept aligned.
                // Note that this means that any transformations that mess
                // with function signatures will need to also update layout info...
                //
                SLANG_ASSERT(entryPointLayout->fields.Count() > paramIndex);
                auto paramLayout = entryPointLayout->fields[paramIndex];

                // We need to create a global variable that will replace the parameter.
                // It seems superficially obvious that the variable should have
                // the same type as the parameter.
                // However, if the parameter was a pointer, in order to
                // support `out` or `in out` parameter passing, we need
                // to be sure to allocate a variable of the pointed-to
                // type instead.
                //
                // We also need to replace uses of the parameter with
                // uses of the variable, and the exact logic there
                // will differ a bit between the pointer and non-pointer
                // cases.
                auto paramType = pp->getDataType();

                // First we will special-case stage input/outputs that
                // don't fit into the standard varying model.
                // For right now we are only doing special-case handling
                // of geometry shader output streams.
                if( auto paramPtrType = as<IROutTypeBase>(paramType) )
                {
                    auto valueType = paramPtrType->getValueType();
                    if( auto gsStreamType = as<IRHLSLStreamOutputType>(valueType) )
                    {
                        // An output stream type like `TriangleStream<Foo>` should
                        // more or less translate into `out Foo` (plus scalarization).

                        auto globalOutputVal = createGLSLGlobalVaryings(
                            &context,
                            &builder,
                            valueType,
                            paramLayout,
                            LayoutResourceKind::VaryingOutput,
                            stage);

                        // TODO: a GS output stream might be passed into other
                        // functions, so that we should really be modifying
                        // any function that has one of these in its parameter
                        // list (and in the limit we should be leagalizing any
                        // type that nests these...).
                        //
                        // For now we will just try to deal with `Append` calls
                        // directly in this function.



                        for( auto bb = func->getFirstBlock(); bb; bb = bb->getNextBlock() )
                        {
                            for( auto ii = bb->getFirstInst(); ii; ii = ii->getNextInst() )
                            {
                                // Is it a call?
                                if(ii->op != kIROp_Call)
                                    continue;

                                // Is it calling the append operation?
                                auto callee = ii->getOperand(0);
                                for(;;)
                                {
                                    // If the instruction is `specialize(X,...)` then
                                    // we want to look at `X`, and if it is `generic { ... return R; }`
                                    // then we want to look at `R`. We handle this
                                    // iteratively here.
                                    //
                                    // TODO: This idiom seems to come up enough that we
                                    // should probably have a dedicated convenience routine
                                    // for this.
                                    //
                                    // Alternatively, we could switch the IR encoding so
                                    // that decorations are added to the generic instead of the
                                    // value it returns.
                                    //
                                    switch(callee->op)
                                    {
                                    case kIROp_Specialize:
                                        {
                                            callee = cast<IRSpecialize>(callee)->getOperand(0);
                                            continue;
                                        }

                                    case kIROp_Generic:
                                        {
                                            auto genericResult = findGenericReturnVal(cast<IRGeneric>(callee));
                                            if(genericResult)
                                            {
                                                callee = genericResult;
                                                continue;
                                            }
                                        }

                                    default:
                                        break;
                                    }
                                    break;
                                }
                                if(callee->op != kIROp_Func)
                                    continue;

                                // HACK: we will identify the operation based
                                // on the target-intrinsic definition that was
                                // given to it.
                                auto decoration = findTargetIntrinsicDecoration(callee, "glsl");
                                if(!decoration)
                                    continue;

                                if(StringRepresentation::asSlice(decoration->definition) != UnownedStringSlice::fromLiteral("EmitVertex()"))
                                {
                                    continue;
                                }

                                // Okay, we have a declaration, and we want to modify it!

                                builder.setInsertBefore(ii);

                                assign(&builder, globalOutputVal, ScalarizedVal::value(ii->getOperand(2)));
                            }
                        }

                        continue;
                    }
                }


                // Is the parameter type a special pointer type
                // that indicates the parameter is used for `out`
                // or `inout` access?
                if(auto paramPtrType = as<IROutTypeBase>(paramType) )
                {
                    // Okay, we have the more interesting case here,
                    // where the parameter was being passed by reference.
                    // We are going to create a local variable of the appropriate
                    // type, which will replace the parameter, along with
                    // one or more global variables for the actual input/output.

                    auto valueType = paramPtrType->getValueType();

                    auto localVariable = builder.emitVar(valueType);
                    auto localVal = ScalarizedVal::address(localVariable);

                    if( auto inOutType = as<IRInOutType>(paramPtrType) )
                    {
                        // In the `in out` case we need to declare two
                        // sets of global variables: one for the `in`
                        // side and one for the `out` side.
                        auto globalInputVal = createGLSLGlobalVaryings(
                            &context,
                            &builder, valueType, paramLayout, LayoutResourceKind::VaryingInput, stage);

                        assign(&builder, localVal, globalInputVal);
                    }

                    // Any places where the original parameter was used inside
                    // the function body should instead use the new local variable.
                    // Since the parameter was a pointer, we use the variable instruction
                    // itself (which is an `alloca`d pointer) directly:
                    pp->replaceUsesWith(localVariable);

                    // We also need one or more global variabels to write the output to
                    // when the function is done. We create them here.
                    auto globalOutputVal = createGLSLGlobalVaryings(
                            &context,
                            &builder, valueType, paramLayout, LayoutResourceKind::VaryingOutput, stage);

                    // Now we need to iterate over all the blocks in the function looking
                    // for any `return*` instructions, so that we can write to the output variable
                    for( auto bb = func->getFirstBlock(); bb; bb = bb->getNextBlock() )
                    {
                        auto terminatorInst = bb->getLastInst();
                        if(!terminatorInst)
                            continue;

                        switch( terminatorInst->op )
                        {
                        default:
                            continue;

                        case kIROp_ReturnVal:
                        case kIROp_ReturnVoid:
                            break;
                        }

                        // We dont' re-use `builder` here because we don't want to
                        // disrupt the source location it is using for inserting
                        // temporary variables at the top of the function.
                        //
                        IRBuilder terminatorBuilder;
                        terminatorBuilder.sharedBuilder = builder.sharedBuilder;
                        terminatorBuilder.setInsertBefore(terminatorInst);

                        // Assign from the local variabel to the global output
                        // variable before the actual `return` takes place.
                        assign(&terminatorBuilder, globalOutputVal, localVal);
                    }
                }
                else
                {
                    // This is the "easy" case where the parameter wasn't
                    // being passed by reference. We start by just creating
                    // one or more global variables to represent the parameter,
                    // and attach the required layout information to it along
                    // the way.

                    auto globalValue = createGLSLGlobalVaryings(
                        &context,
                        &builder, paramType, paramLayout, LayoutResourceKind::VaryingInput, stage);

                    // Next we need to replace uses of the parameter with
                    // references to the variable(s). We are going to do that
                    // somewhat naively, by simply materializing the
                    // variables at the start.
                    IRInst* materialized = materializeValue(&builder, globalValue);

                    pp->replaceUsesWith(materialized);
                }
            }

            // At this point we should have eliminated all uses of the
            // parameters of the entry block. Also, our control-flow
            // rules mean that the entry block cannot be the target
            // of any branches in the code, so there can't be
            // any control-flow ops that try to match the parameter
            // list.
            //
            // We can safely go through and destroy the parameters
            // themselves, and then clear out the parameter list.

            for( auto pp = firstBlock->getFirstParam(); pp; )
            {
                auto next = pp->getNextParam();
                pp->removeAndDeallocate();
                pp = next;
            }
        }

        // Finally, we need to patch up the type of the entry point,
        // because it is no longer accurate.

        IRFuncType* voidFuncType = builder.getFuncType(
            0,
            nullptr,
            builder.getVoidType());
        func->setFullType(voidFuncType);

        // TODO: we should technically be constructing
        // a new `EntryPointLayout` here to reflect
        // the way that things have been moved around.
    }

    // Needed for lookup up entry-point layouts.
    //
    // TODO: maybe arrange so that codegen is driven from the layout layer
    // instead of the input/request layer.
    EntryPointLayout* findEntryPointLayout(
        ProgramLayout*      programLayout,
        EntryPointRequest*  entryPointRequest);

    struct IRSpecSymbol : RefObject
    {
        IRGlobalValue*          irGlobalValue;
        RefPtr<IRSpecSymbol>    nextWithSameName;
    };

    struct IRSpecEnv
    {
        IRSpecEnv*  parent = nullptr;

        // A map from original values to their cloned equivalents.
        typedef Dictionary<IRInst*, IRInst*> ClonedValueDictionary;
        ClonedValueDictionary clonedValues;
    };

    struct IRSharedSpecContext
    {
        // The code-generation target in use
        CodeGenTarget target;

        // The specialized module we are building
        RefPtr<IRModule>   module;

        // The original, unspecialized module we are copying
        IRModule*   originalModule;

        // A map from mangled symbol names to zero or
        // more global IR values that have that name,
        // in the *original* module.
        typedef Dictionary<Name*, RefPtr<IRSpecSymbol>> SymbolDictionary;
        SymbolDictionary symbols;

        SharedIRBuilder sharedBuilderStorage;
        IRBuilder builderStorage;

        // The "global" specialization environment.
        IRSpecEnv globalEnv;
    };

    struct IRSharedGenericSpecContext : IRSharedSpecContext
    {
        // Instructions to be processed (for generic specialization context)
        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;
        void addToWorkList(IRInst* inst)
        {
            if(!workListSet.Contains(inst))
            {
                workList.Add(inst);
                workListSet.Add(inst);
            }
        }
        IRInst* popWorkList()
        {
            UInt count = workList.Count();
            if(count != 0)
            {
                IRInst* inst = workList[count - 1];
                workList.FastRemoveAt(count - 1);
                workListSet.Remove(inst);
                return inst;
            }
            return nullptr;
        }
    };

    struct IRSpecContextBase
    {
        // A map from the mangled name of a global variable
        // to the layout to use for it.
        Dictionary<Name*, VarLayout*> globalVarLayouts;

        IRSharedSpecContext* shared;

        IRSharedSpecContext* getShared() { return shared; }

        IRModule* getModule() { return getShared()->module; }

        IRModule* getOriginalModule() { return getShared()->originalModule; }

        IRSharedSpecContext::SymbolDictionary& getSymbols() { return getShared()->symbols; }

        // The current specialization environment to use.
        IRSpecEnv* env = nullptr;
        IRSpecEnv* getEnv()
        {
            // TODO: need to actually establish environments on contexts we create.
            //
            // Or more realistically we need to change the whole approach
            // to specialization and cloning so that we don't try to share
            // logic between two very different cases.


            return env;
        }

        // The IR builder to use for creating nodes
        IRBuilder*  builder;

        // A callback to be used when a value that is not registerd in `clonedValues`
        // is needed during cloning. This gives the subtype a chance to intercept
        // the operation and clone (or not) as needed.
        virtual IRInst* maybeCloneValue(IRInst* originalVal)
        {
            return originalVal;
        }
    };

    void registerClonedValue(
        IRSpecContextBase*  context,
        IRInst*    clonedValue,
        IRInst*    originalValue)
    {
        if(!originalValue)
            return;

        // TODO: now that things are scoped using environments, we
        // shouldn't be running into the cases where a value with
        // the same key already exists. This should be changed to
        // an `Add()` call.
        //
        context->getEnv()->clonedValues[originalValue] = clonedValue;
    }

    // Information on values to use when registering a cloned value
    struct IROriginalValuesForClone
    {
        IRInst*        originalVal = nullptr;
        IRSpecSymbol*   sym = nullptr;

        IROriginalValuesForClone() {}

        IROriginalValuesForClone(IRInst* originalValue)
            : originalVal(originalValue)
        {}

        IROriginalValuesForClone(IRSpecSymbol* symbol)
            : sym(symbol)
        {}
    };

    void registerClonedValue(
        IRSpecContextBase*              context,
        IRInst*                        clonedValue,
        IROriginalValuesForClone const& originalValues)
    {
        registerClonedValue(context, clonedValue, originalValues.originalVal);
        for( auto s = originalValues.sym; s; s = s->nextWithSameName )
        {
            registerClonedValue(context, clonedValue, s->irGlobalValue);
        }
    }

    void cloneDecorations(
        IRSpecContextBase*  context,
        IRInst*        clonedValue,
        IRInst*        originalValue)
    {
        for (auto dd = originalValue->firstDecoration; dd; dd = dd->next)
        {
            switch (dd->op)
            {
            case kIRDecorationOp_HighLevelDecl:
                {
                    auto originalDecoration = (IRHighLevelDeclDecoration*)dd;

                    context->builder->addHighLevelDeclDecoration(clonedValue, originalDecoration->decl);
                }
                break;

            case kIRDecorationOp_LoopControl:
                {
                    auto originalDecoration = (IRLoopControlDecoration*)dd;
                    auto newDecoration = context->builder->addDecoration<IRLoopControlDecoration>(clonedValue);
                    newDecoration->mode = originalDecoration->mode;
                }
                break;

            case kIRDecorationOp_TargetIntrinsic:
                {
                    auto originalDecoration = (IRTargetIntrinsicDecoration*)dd;
                    auto newDecoration = context->builder->addDecoration<IRTargetIntrinsicDecoration>(clonedValue);
                    newDecoration->targetName = originalDecoration->targetName;
                    newDecoration->definition = originalDecoration->definition;
                }
                break;

            case kIRDecorationOp_Semantic:
                {
                    auto originalDecoration = (IRSemanticDecoration*)dd;
                    auto newDecoration = context->builder->addDecoration<IRSemanticDecoration>(clonedValue);
                    newDecoration->semanticName = originalDecoration->semanticName;
                }
                break;

            case kIRDecorationOp_InterpolationMode:
                {
                    auto originalDecoration = (IRInterpolationModeDecoration*)dd;
                    auto newDecoration = context->builder->addDecoration<IRInterpolationModeDecoration>(clonedValue);
                    newDecoration->mode = originalDecoration->mode;
                }
                break;

            case kIRDecorationOp_NameHint:
                {
                    auto originalDecoration = (IRNameHintDecoration*)dd;
                    auto newDecoration = context->builder->addDecoration<IRNameHintDecoration>(clonedValue);
                    newDecoration->name = originalDecoration->name;
                }
                break;

            default:
                // Don't clone any decorations we don't understand.
                break;
            }
        }

        // We will also clone the location here, just because this is a convenient bottleneck
        clonedValue->sourceLoc = originalValue->sourceLoc;
    }

    // We use an `IRSpecContext` for the case where we are cloning
    // code from one or more input modules to create a "linked" output
    // module. Along the way, we will resolve profile-specific functions
    // to the best definition for a given target.
    //
    struct IRSpecContext : IRSpecContextBase
    {
        // Override the "maybe clone" logic so that we always clone
        virtual IRInst* maybeCloneValue(IRInst* originalVal) override;
    };


    IRGlobalValue* cloneGlobalValue(IRSpecContext* context, IRGlobalValue* originalVal);

    IRInst* cloneValue(
        IRSpecContextBase*  context,
        IRInst*        originalValue);

    IRType* cloneType(
        IRSpecContextBase*  context,
        IRType*             originalType);

    IRInst* IRSpecContext::maybeCloneValue(IRInst* originalValue)
    {
        if (auto globalValue = as<IRGlobalValue>(originalValue))
        {
            return cloneGlobalValue(this, globalValue);
        }

        switch (originalValue->op)
        {
        case kIROp_boolConst:
            {
                IRConstant* c = (IRConstant*)originalValue;
                return builder->getBoolValue(c->value.intVal != 0);
            }
            break;


        case kIROp_IntLit:
            {
                IRConstant* c = (IRConstant*)originalValue;
                return builder->getIntValue(cloneType(this, c->getDataType()), c->value.intVal);
            }
            break;

        case kIROp_FloatLit:
            {
                IRConstant* c = (IRConstant*)originalValue;
                return builder->getFloatValue(cloneType(this, c->getDataType()), c->value.floatVal);
            }
            break;

        default:
            {
                // In the deafult case, assume that we have some sort of "hoistable"
                // instruction that requires us to create a clone of it.

                UInt argCount = originalValue->getOperandCount();
                IRInst* clonedValue = createInstWithTrailingArgs<IRInst>(
                    builder,
                    originalValue->op,
                    cloneType(this, originalValue->getFullType()),
                    0, nullptr,
                    argCount, nullptr);
                registerClonedValue(this, clonedValue, originalValue);
                for (UInt aa = 0; aa < argCount; ++aa)
                {
                    IRInst* originalArg = originalValue->getOperand(aa);
                    IRInst* clonedArg = cloneValue(this, originalArg);
                    clonedValue->getOperands()[aa].init(clonedValue, clonedArg);
                }
                cloneDecorations(this, clonedValue, originalValue);

                addHoistableInst(builder, clonedValue);

                return clonedValue;
            }
            break;
        }
    }

    IRInst* cloneValue(
        IRSpecContextBase*  context,
        IRInst*        originalValue);

    // Find a pre-existing cloned value, or return null if none is available.
    IRInst* findClonedValue(
        IRSpecContextBase*  context,
        IRInst*        originalValue)
    {
        IRInst* clonedValue = nullptr;
        for (auto env = context->getEnv(); env; env = env->parent)
        {
            if (env->clonedValues.TryGetValue(originalValue, clonedValue))
            {
                return clonedValue;
            }
        }

        return nullptr;
    }

    IRInst* cloneValue(
        IRSpecContextBase*  context,
        IRInst*        originalValue)
    {
        if (!originalValue)
            return nullptr;

        if (IRInst* clonedValue = findClonedValue(context, originalValue))
            return clonedValue;

        return context->maybeCloneValue(originalValue);
    }

    IRType* cloneType(
        IRSpecContextBase*  context,
        IRType*             originalType)
    {
        return (IRType*)cloneValue(context, originalType);
    }

    IRInst* maybeCloneValueWithMangledName(
        IRSpecContextBase*  context,
        IRGlobalValue*      originalValue)
    {
        for(auto ii : context->shared->module->getGlobalInsts())
        {
            auto gv = as<IRGlobalValue>(ii);
            if (!gv)
                continue;

            if (gv->mangledName == originalValue->mangledName)
                return gv;
        }
        return cloneValue(context, originalValue);
    }

    IRInst* cloneInst(
        IRSpecContextBase*              context,
        IRBuilder*                      builder,
        IRInst*                         originalInst,
        IROriginalValuesForClone const& originalValues);

    IRInst* cloneInst(
        IRSpecContextBase*  context,
        IRBuilder*          builder,
        IRInst*             originalInst)
    {
        return cloneInst(context, builder, originalInst, originalInst);
    }

    void cloneGlobalValueWithCodeCommon(
        IRSpecContextBase*      context,
        IRGlobalValueWithCode*  clonedValue,
        IRGlobalValueWithCode*  originalValue);

    IRRate* cloneRate(
        IRSpecContextBase*  context,
        IRRate*             rate)
    {
        return (IRRate*) cloneType(context, rate);
    }

    void maybeSetClonedRate(
        IRSpecContextBase*  context,
        IRBuilder*          builder,
        IRInst*             clonedValue,
        IRInst*             originalValue)
    {
        if(auto rate = originalValue->getRate() )
        {
            clonedValue->setFullType(builder->getRateQualifiedType(
                cloneRate(context, rate),
                clonedValue->getFullType()));
        }
    }

    IRGlobalVar* cloneGlobalVarImpl(
        IRSpecContextBase*              context,
        IRBuilder*                      builder,
        IRGlobalVar*                    originalVar,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedVar = builder->createGlobalVar(
            cloneType(context, originalVar->getDataType()->getValueType()));

        maybeSetClonedRate(context, builder, clonedVar, originalVar);

        registerClonedValue(context, clonedVar, originalValues);

        auto mangledName = originalVar->mangledName;
        clonedVar->mangledName = mangledName;

        cloneDecorations(context, clonedVar, originalVar);

        VarLayout* layout = nullptr;
        if (context->globalVarLayouts.TryGetValue(mangledName, layout))
        {
            builder->addLayoutDecoration(clonedVar, layout);
        }

        // Clone any code in the body of the variable, since this
        // represents the initializer.
        cloneGlobalValueWithCodeCommon(
            context,
            clonedVar,
            originalVar);

        return clonedVar;
    }

    IRGlobalConstant* cloneGlobalConstantImpl(
        IRSpecContextBase*              context,
        IRBuilder*                      builder,
        IRGlobalConstant*               originalVal,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedVal = builder->createGlobalConstant(
            cloneType(context, originalVal->getFullType()));
        registerClonedValue(context, clonedVal, originalValues);

        auto mangledName = originalVal->mangledName;
        clonedVal->mangledName = mangledName;

        cloneDecorations(context, clonedVal, originalVal);

        // Clone any code in the body of the constant, since this
        // represents the initializer.
        cloneGlobalValueWithCodeCommon(
            context,
            clonedVal,
            originalVal);

        return clonedVal;
    }

    IRGeneric* cloneGenericImpl(
        IRSpecContextBase*              context,
        IRBuilder*                      builder,
        IRGeneric*                      originalVal,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedVal = builder->emitGeneric();
        registerClonedValue(context, clonedVal, originalValues);

        auto mangledName = originalVal->mangledName;
        clonedVal->mangledName = mangledName;

        cloneDecorations(context, clonedVal, originalVal);

        // Clone any code in the body of the generic, since this
        // computes its result value.
        cloneGlobalValueWithCodeCommon(
            context,
            clonedVal,
            originalVal);

        return clonedVal;
    }

    void cloneSimpleGlobalValueImpl(
        IRSpecContextBase*              context,
        IRGlobalValue*                  originalInst,
        IROriginalValuesForClone const& originalValues,
        IRGlobalValue*                  clonedInst,
        bool                            registerValue = true)
    {
        if (registerValue)
            registerClonedValue(context, clonedInst, originalValues);

        auto mangledName = originalInst->mangledName;
        clonedInst->mangledName = mangledName;

        cloneDecorations(context, clonedInst, originalInst);

        // Set up an IR builder for inserting into the inst
        IRBuilder builderStorage = *context->builder;
        IRBuilder* builder = &builderStorage;
        builder->setInsertInto(clonedInst);

        // Clone any children of the instruction
        for (auto child : originalInst->getChildren())
        {
            cloneInst(context, builder, child);
        }
    }

    IRStructKey* cloneStructKeyImpl(
        IRSpecContextBase*              context,
        IRBuilder*                      builder,
        IRStructKey*                    originalVal,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedVal = builder->createStructKey();
        cloneSimpleGlobalValueImpl(context, originalVal, originalValues, clonedVal);
        return clonedVal;
    }

    IRGlobalGenericParam* cloneGlobalGenericParamImpl(
        IRSpecContextBase*              context,
        IRBuilder*                      builder,
        IRGlobalGenericParam*           originalVal,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedVal = builder->emitGlobalGenericParam();
        cloneSimpleGlobalValueImpl(context, originalVal, originalValues, clonedVal);
        return clonedVal;
    }


    IRWitnessTable* cloneWitnessTableImpl(
        IRSpecContextBase*  context,
        IRBuilder*          builder,
        IRWitnessTable* originalTable,
        IROriginalValuesForClone const& originalValues,
        IRWitnessTable* dstTable = nullptr,
        bool registerValue = true)
    {
        auto clonedTable = dstTable ? dstTable : builder->createWitnessTable();
        cloneSimpleGlobalValueImpl(context, originalTable, originalValues, clonedTable, registerValue);
        return clonedTable;
    }

    IRWitnessTable* cloneWitnessTableWithoutRegistering(
        IRSpecContextBase*  context,
        IRBuilder*          builder,
        IRWitnessTable* originalTable,
        IRWitnessTable* dstTable = nullptr)
    {
        return cloneWitnessTableImpl(context, builder, originalTable, IROriginalValuesForClone(), dstTable, false);
    }

    IRStructType* cloneStructTypeImpl(
        IRSpecContextBase*              context,
        IRBuilder*                      builder,
        IRStructType*                   originalStruct,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedStruct = builder->createStructType();
        cloneSimpleGlobalValueImpl(context, originalStruct, originalValues, clonedStruct);
        return clonedStruct;
    }

    void cloneGlobalValueWithCodeCommon(
        IRSpecContextBase*      context,
        IRGlobalValueWithCode*  clonedValue,
        IRGlobalValueWithCode*  originalValue)
    {
        // Next we are going to clone the actual code.
        IRBuilder builderStorage = *context->builder;
        IRBuilder* builder = &builderStorage;
        builder->setInsertInto(clonedValue);


        // We will walk through the blocks of the function, and clone each of them.
        //
        // We need to create the cloned blocks first, and then walk through them,
        // because blocks might be forward referenced (this is not possible
        // for other cases of instructions).
        for (auto originalBlock = originalValue->getFirstBlock();
            originalBlock;
            originalBlock = originalBlock->getNextBlock())
        {
            IRBlock* clonedBlock = builder->createBlock();
            clonedValue->addBlock(clonedBlock);
            registerClonedValue(context, clonedBlock, originalBlock);

#if 0
            // We can go ahead and clone parameters here, while we are at it.
            builder->curBlock = clonedBlock;
            for (auto originalParam = originalBlock->getFirstParam();
                originalParam;
                originalParam = originalParam->getNextParam())
            {
                IRParam* clonedParam = builder->emitParam(
                    context->maybeCloneType(
                        originalParam->getFullType()));
                cloneDecorations(context, clonedParam, originalParam);
                registerClonedValue(context, clonedParam, originalParam);
            }
#endif
        }

        // Okay, now we are in a good position to start cloning
        // the instructions inside the blocks.
        {
            IRBlock* ob = originalValue->getFirstBlock();
            IRBlock* cb = clonedValue->getFirstBlock();
            while (ob)
            {
                SLANG_ASSERT(cb);

                builder->setInsertInto(cb);
                for (auto oi = ob->getFirstInst(); oi; oi = oi->getNextInst())
                {
                    cloneInst(context, builder, oi);
                }

                ob = ob->getNextBlock();
                cb = cb->getNextBlock();
            }
        }

    }

    void checkIRDuplicate(IRInst* inst, IRParentInst* moduleInst, Name* mangledName)
    {
#ifdef _DEBUG
        for (auto child : moduleInst->getChildren())
        {
            if (child == inst)
                continue;

            if (child->op == kIROp_Func)
            {
                auto extName = ((IRGlobalValue*)child)->mangledName;
                if (extName == mangledName ||
                    (extName && mangledName &&
                        extName->text == mangledName->text))
                    SLANG_UNEXPECTED("duplicate global var");
            }
        }
#else
        SLANG_UNREFERENCED_PARAMETER(inst);
        SLANG_UNREFERENCED_PARAMETER(moduleInst);
        SLANG_UNREFERENCED_PARAMETER(mangledName);
#endif
    }

    void cloneFunctionCommon(
        IRSpecContextBase*  context,
        IRFunc*         clonedFunc,
        IRFunc*         originalFunc,
        bool checkDuplicate = true)
    {
        // First clone all the simple properties.
        clonedFunc->mangledName = originalFunc->mangledName;
        clonedFunc->setFullType(cloneType(context, originalFunc->getFullType()));

        cloneDecorations(context, clonedFunc, originalFunc);

        cloneGlobalValueWithCodeCommon(
            context,
            clonedFunc,
            originalFunc);

        // Shuffle the function to the end of the list, because
        // it needs to follow its dependencies.
        //
        // TODO: This isn't really a good requirement to place on the IR...
        clonedFunc->moveToEnd();
        if (checkDuplicate)
            checkIRDuplicate(clonedFunc, context->getModule()->getModuleInst(), clonedFunc->mangledName);
    }

    IRFunc* specializeIRForEntryPoint(
        IRSpecContext*  context,
        EntryPointRequest*  entryPointRequest,
        EntryPointLayout*   entryPointLayout)
    {
        // Look up the IR symbol by name
        auto mangledName = context->getModule()->session->getNameObj(getMangledName(entryPointRequest->decl));
        RefPtr<IRSpecSymbol> sym;
        if (!context->getSymbols().TryGetValue(mangledName, sym))
        {
            SLANG_UNEXPECTED("no matching IR symbol");
            return nullptr;
        }

        // TODO: deal with the case where we might
        // have multiple versions...

        auto globalValue = sym->irGlobalValue;
        if (globalValue->op != kIROp_Func)
        {
            SLANG_UNEXPECTED("expected an IR function");
            return nullptr;
        }
        auto originalFunc = (IRFunc*)globalValue;

        // Create a clone for the IR function
        auto clonedFunc = context->builder->createFunc();

        // Note: we do *not* register this cloned declaration
        // as the cloned value for the original symbol.
        // This is kind of a kludge, but it ensures that
        // in the unlikely case that the function is both
        // used as an entry point and a callable function
        // (yes, this would imply recursion...) we actually
        // have two copies, which lets us arbitrarily
        // transform the entry point to meet target requirements.
        //
        // TODO: The above statement is kind of bunk, though,
        // because both versions of the function would have
        // the same mangled name... :(

        // We need to clone all the properties of the original
        // function, including any blocks, their parameters,
        // and their instructions.
        cloneFunctionCommon(context, clonedFunc, originalFunc);

        // We need to attach the layout information for
        // the entry point to this declaration, so that
        // we can use it to inform downstream code emit.
        context->builder->addLayoutDecoration(
            clonedFunc,
            entryPointLayout);

        // We will also go on and attach layout information
        // to the function parameters, so that we have it
        // available directly on the parameters, rather
        // than having to look it up on the original entry-point layout.
        if( auto firstBlock = clonedFunc->getFirstBlock() )
        {
            UInt paramLayoutCount = entryPointLayout->fields.Count();
            UInt paramCounter = 0;
            for( auto pp = firstBlock->getFirstParam(); pp; pp = pp->getNextParam() )
            {
                UInt paramIndex = paramCounter++;
                if( paramIndex < paramLayoutCount )
                {
                    auto paramLayout = entryPointLayout->fields[paramIndex];
                    context->builder->addLayoutDecoration(
                        pp,
                        paramLayout);
                }
                else
                {
                    SLANG_UNEXPECTED("too many parameters");
                }
            }
        }

        return clonedFunc;
    }

    IRFunc* cloneSimpleFuncWithoutRegistering(IRSpecContextBase* context, IRFunc* originalFunc)
    {
        auto clonedFunc = context->builder->createFunc();
        cloneFunctionCommon(context, clonedFunc, originalFunc, false);
        return clonedFunc;
    }

    // Get a string form of the target so that we can
    // use it to match against target-specialization modifiers
    //
    // TODO: We shouldn't be using strings for this.
    String getTargetName(IRSpecContext* context)
    {
        switch( context->shared->target )
        {
        case CodeGenTarget::HLSL:
            return "hlsl";

        case CodeGenTarget::GLSL:
            return "glsl";

        default:
            SLANG_UNEXPECTED("unhandled case");
            UNREACHABLE_RETURN("unknown");
        }
    }

    // How specialized is a given declaration for the chosen target?
    enum class TargetSpecializationLevel
    {
        specializedForOtherTarget = 0,
        notSpecialized,
        specializedForTarget,
    };

    TargetSpecializationLevel getTargetSpecialiationLevel(
        IRGlobalValue*  val,
        String const&   targetName)
    {
        TargetSpecializationLevel result = TargetSpecializationLevel::notSpecialized;
        for( auto dd = val->firstDecoration; dd; dd = dd->next )
        {
            if(dd->op != kIRDecorationOp_Target)
                continue;

            auto decoration = (IRTargetDecoration*) dd;
            if(String(decoration->targetName) == targetName)
                return TargetSpecializationLevel::specializedForTarget;

            result = TargetSpecializationLevel::specializedForOtherTarget;
        }

        return result;
    }

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

    bool isDefinition(
        IRGlobalValue* inVal)
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

        switch (val->op)
        {
        case kIROp_WitnessTable:
        case kIROp_GlobalVar:
        case kIROp_GlobalConstant:
        case kIROp_Func:
        case kIROp_Generic:
            return ((IRParentInst*)val)->getFirstChild() != nullptr;

        case kIROp_StructType:
            return true;

        default:
            return false;
        }
    }

    // Is `newVal` marked as being a better match for our
    // chosen code-generation target?
    //
    // TODO: there is a missing step here where we need
    // to check if things are even available in the first place...
    bool isBetterForTarget(
        IRSpecContext*  context,
        IRGlobalValue*  newVal,
        IRGlobalValue*  oldVal)
    {
        String targetName = getTargetName(context);

        // For right now every declaration might have zero or more
        // modifiers, representing the targets for which it is specialized.
        // Each modifier has a single string "tag" to represent a target.
        // We thus decide that a declaration is "more specialized" by:
        //
        // - Does it have a modifier with a tag with the string for the current target?
        //   If yes, it is the most specialized it can be.
        //
        // - Does it have a no tags? Then it is "unspecialized" and that is okay.
        //
        // - Does it have a modifier with a tag for a *different* target?
        //   If yes, then it shouldn't even be usable on this target.
        //
        // Longer term a better approach is to think of this in terms
        // of a "disjunction of conjunctions" that is:
        //
        //     (A and B and C) or (A and D) or (E) or (F and G) ...
        //
        // A code generation target would then consist of a
        // conjunction of invidual tags:
        //
        //    (HLSL and SM_4_0 and Vertex and ...)
        //
        // A declaration is *applicable* on a target if one of
        // its conjunctions of tags is a subset of the target's.
        //
        // One declaration is *better* than another on a target
        // if it is applicable and its tags are a superset
        // of the other's.

        auto newLevel = getTargetSpecialiationLevel(newVal, targetName);
        auto oldLevel = getTargetSpecialiationLevel(oldVal, targetName);
        if(newLevel != oldLevel)
            return UInt(newLevel) > UInt(oldLevel);

        // All other factors being equal, a definition is
        // better than a declaration.
        auto newIsDef = isDefinition(newVal);
        auto oldIsDef = isDefinition(oldVal);
        if (newIsDef != oldIsDef)
            return newIsDef;

        return false;
    }

    IRFunc* cloneFuncImpl(
        IRSpecContextBase*  context,
        IRBuilder*          builder,
        IRFunc*             originalFunc,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedFunc = builder->createFunc();
        registerClonedValue(context, clonedFunc, originalValues);
        cloneFunctionCommon(context, clonedFunc, originalFunc);
        return clonedFunc;
    }


    IRInst* cloneInst(
        IRSpecContextBase*              context,
        IRBuilder*                      builder,
        IRInst*                         originalInst,
        IROriginalValuesForClone const& originalValues)
    {
        switch (originalInst->op)
        {
            // We need to special-case any instruction that is not
            // allocated like an ordinary `IRInst` with trailing args.
        case kIROp_Func:
            return cloneFuncImpl(context, builder, cast<IRFunc>(originalInst), originalValues);

        case kIROp_GlobalVar:
            return cloneGlobalVarImpl(context, builder, cast<IRGlobalVar>(originalInst), originalValues);

        case kIROp_GlobalConstant:
            return cloneGlobalConstantImpl(context, builder, cast<IRGlobalConstant>(originalInst), originalValues);

        case kIROp_WitnessTable:
            return cloneWitnessTableImpl(context, builder, cast<IRWitnessTable>(originalInst), originalValues);

        case kIROp_StructType:
            return cloneStructTypeImpl(context, builder, cast<IRStructType>(originalInst), originalValues);

        case kIROp_Generic:
            return cloneGenericImpl(context, builder, cast<IRGeneric>(originalInst), originalValues);

        case kIROp_StructKey:
            return cloneStructKeyImpl(context, builder, cast<IRStructKey>(originalInst), originalValues);

        case kIROp_GlobalGenericParam:
            return cloneGlobalGenericParamImpl(context, builder, cast<IRGlobalGenericParam>(originalInst), originalValues);

        default:
            break;
        }

        // The common case is that we just need to construct a cloned
        // instruction with the right number of operands, intialize
        // it, and then add it to the sequence.
        UInt argCount = originalInst->getOperandCount();
        IRInst* clonedInst = createInstWithTrailingArgs<IRInst>(
            builder, originalInst->op,
            cloneType(context, originalInst->getFullType()),
            0, nullptr,
            argCount, nullptr);
        registerClonedValue(context, clonedInst, originalValues);
        auto oldBuilder = context->builder;
        context->builder = builder;
        for (UInt aa = 0; aa < argCount; ++aa)
        {
            IRInst* originalArg = originalInst->getOperand(aa);
            IRInst* clonedArg = cloneValue(context, originalArg);
            clonedInst->getOperands()[aa].init(clonedInst, clonedArg);
        }
        builder->addInst(clonedInst);
        context->builder = oldBuilder;
        cloneDecorations(context, clonedInst, originalInst);

        return clonedInst;
    }

    IRGlobalValue* cloneGlobalValueImpl(
        IRSpecContext*                  context,
        IRGlobalValue*                  originalInst,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedValue = cloneInst(context, &context->shared->builderStorage, originalInst, originalValues);
        clonedValue->moveToEnd();
        return cast<IRGlobalValue>(clonedValue);
    }


    // Clone a global value, which has the given `mangledName`.
    // The `originalVal` is a known global IR value with that name, if one is available.
    // (It is okay for this parameter to be null).
    IRGlobalValue* cloneGlobalValueWithMangledName(
        IRSpecContext*  context,
        Name*           mangledName,
        IRGlobalValue*  originalVal)
    {
        // If the global value being cloned is already in target module, don't clone
        // Why checking this?
        //   When specializing a generic function G (which is already in target module),
        //   where G calls a normal function F (which is already in target module),
        //   then when we are making a copy of G via cloneFuncCommom(), it will recursively clone F,
        //   however we don't want to make a duplicate of F in the target module.
        if (originalVal->getParent() == context->getModule()->getModuleInst())
            return originalVal;

        // Check if we've already cloned this value, for the case where
        // an original value has already been established.
        if (originalVal)
        {
            if (IRInst* clonedVal = findClonedValue(context, originalVal))
            {
                return cast<IRGlobalValue>(clonedVal);
            }
        }

        if(getText(mangledName).Length() == 0)
        {
            // If there is no mangled name, then we assume this is a local symbol,
            // and it can't possibly have multiple declarations.
            return cloneGlobalValueImpl(context, originalVal, IROriginalValuesForClone());
        }

        //
        // We will scan through all of the available declarations
        // with the same mangled name as `originalVal` and try
        // to pick the "best" one for our target.

        RefPtr<IRSpecSymbol> sym;
        if( !context->getSymbols().TryGetValue(mangledName, sym) )
        {
            if(!originalVal)
                return nullptr;

            // This shouldn't happen!
            SLANG_UNEXPECTED("no matching values registered");
            UNREACHABLE_RETURN(cloneGlobalValueImpl(context, originalVal, IROriginalValuesForClone()));
        }

        // We will try to track the "best" declaration we can find.
        //
        // Generally, one declaration wil lbe better than another if it is
        // more specialized for the chosen target. Otherwise, we simply favor
        // definitions over declarations.
        //
        IRGlobalValue* bestVal = sym->irGlobalValue;
        for( auto ss = sym->nextWithSameName; ss; ss = ss->nextWithSameName )
        {
            IRGlobalValue* newVal = ss->irGlobalValue;
            if(isBetterForTarget(context, newVal, bestVal))
                bestVal = newVal;
        }

        // Check if we've already cloned this value, for the case where
        // we didn't have an original value (just a name), but we've
        // now found a representative value.
        if (!originalVal)
        {
            if (IRInst* clonedVal = findClonedValue(context, bestVal))
            {
                return cast<IRGlobalValue>(clonedVal);
            }
        }

        return cloneGlobalValueImpl(context, bestVal, IROriginalValuesForClone(sym));
    }

    IRGlobalValue* cloneGlobalValueWithMangledName(IRSpecContext* context, Name* mangledName)
    {
        return cloneGlobalValueWithMangledName(context, mangledName, nullptr);
    }

    // Clone a global value, where `originalVal` is one declaration/definition, but we might
    // have to consider others, in order to find the "best" version of the symbol.
    IRGlobalValue* cloneGlobalValue(IRSpecContext* context, IRGlobalValue* originalVal)
    {
        // We are being asked to clone a particular global value, but in
        // the IR that comes out of the front-end there could still
        // be multiple, target-specific, declarations of any given
        // global value, all of which share the same mangled name.
        return cloneGlobalValueWithMangledName(
            context,
            originalVal->mangledName,
            originalVal);
    }

    StructTypeLayout* getGlobalStructLayout(
        ProgramLayout*  programLayout);

    void insertGlobalValueSymbol(
        IRSharedSpecContext*    sharedContext,
        IRGlobalValue*          gv)
    {
        auto mangledName = gv->mangledName;

        // Don't try to register a symbol for global values
        // with no mangled name, since these represent symbols
        // that shouldn't get "linkage"
        if (!getText(mangledName).Length())
            return;

        RefPtr<IRSpecSymbol> sym = new IRSpecSymbol();
        sym->irGlobalValue = gv;

        RefPtr<IRSpecSymbol> prev;
        if (sharedContext->symbols.TryGetValue(mangledName, prev))
        {
            sym->nextWithSameName = prev->nextWithSameName;
            prev->nextWithSameName = sym;
        }
        else
        {
            sharedContext->symbols.Add(mangledName, sym);
        }
    }

    void insertGlobalValueSymbols(
        IRSharedSpecContext*    sharedContext,
        IRModule*               originalModule)
    {
        if (!originalModule)
            return;

        for(auto ii : originalModule->getGlobalInsts())
        {
            auto gv = as<IRGlobalValue>(ii);
            if (!gv)
                continue;
            insertGlobalValueSymbol(sharedContext, gv);
        }
    }

    void initializeSharedSpecContext(
        IRSharedSpecContext*    sharedContext,
        Session*                session,
        IRModule*               module,
        IRModule*               originalModule,
        CodeGenTarget           target)
    {

        SharedIRBuilder* sharedBuilder = &sharedContext->sharedBuilderStorage;
        sharedBuilder->module = nullptr;
        sharedBuilder->session = session;

        IRBuilder* builder = &sharedContext->builderStorage;
        builder->sharedBuilder = sharedBuilder;

        if( !module )
        {
            module = builder->createModule();
        }

        sharedBuilder->module = module;
        sharedContext->module = module;
        sharedContext->originalModule = originalModule;
        sharedContext->target = target;
        // We will populate a map with all of the IR values
        // that use the same mangled name, to make lookup easier
        // in other steps.
        insertGlobalValueSymbols(sharedContext, originalModule);
    }

    // implementation provided in parameter-binding.cpp
    RefPtr<ProgramLayout> specializeProgramLayout(
        TargetRequest * targetReq,
        ProgramLayout* programLayout,
        SubstitutionSet typeSubst);

    struct IRSpecializationState
    {
        ProgramLayout*      programLayout;
        CodeGenTarget       target;
        TargetRequest*      targetReq;

        IRModule* irModule = nullptr;
        RefPtr<ProgramLayout> newProgramLayout;

        IRSharedSpecContext sharedContextStorage;
        IRSpecContext contextStorage;

        IRSpecEnv globalEnv;

        IRSharedSpecContext* getSharedContext() { return &sharedContextStorage; }
        IRSpecContext* getContext() { return &contextStorage; }

        IRSpecializationState()
        {
            contextStorage.env = &globalEnv;
        }

        ~IRSpecializationState()
        {
            newProgramLayout = nullptr;
            contextStorage = IRSpecContext();
            sharedContextStorage = IRSharedSpecContext();
        }
    };

    IRSpecializationState* createIRSpecializationState(
        EntryPointRequest*  entryPointRequest,
        ProgramLayout*      programLayout,
        CodeGenTarget       target,
        TargetRequest*      targetReq)
    {
        IRSpecializationState* state = new IRSpecializationState();

        state->programLayout = programLayout;
        state->target = target;
        state->targetReq = targetReq;


        auto compileRequest = entryPointRequest->compileRequest;
        auto translationUnit = entryPointRequest->getTranslationUnit();
        auto originalIRModule = translationUnit->irModule;

        auto sharedContext = state->getSharedContext();
        initializeSharedSpecContext(
            sharedContext,
            compileRequest->mSession,
            nullptr,
            originalIRModule,
            target);

        state->irModule = sharedContext->module;

        // We also need to attach the IR definitions for symbols from
        // any loaded modules:
        for (auto loadedModule : compileRequest->loadedModulesList)
        {
            insertGlobalValueSymbols(sharedContext, loadedModule->irModule);
        }

        auto context = state->getContext();
        context->shared = sharedContext;
        context->builder = &sharedContext->builderStorage;

        // Now specialize the program layout using the substitution
        //
        // TODO: The specialization of the layout is conceptually an AST-level operations,
        // and shouldn't be done here in the IR at all.
        //
        RefPtr<ProgramLayout> newProgramLayout = specializeProgramLayout(
            targetReq,
            programLayout,
            SubstitutionSet(entryPointRequest->globalGenericSubst));

        // TODO: we need to register the (IR-level) arguments of the global generic parameters as the
        // substitutions for the generic parameters in the original IR.

        // applyGlobalGenericParamSubsitution(...);


        state->newProgramLayout = newProgramLayout;

        // Next, we want to optimize lookup for layout infromation
        // associated with global declarations, so that we can
        // look things up based on the IR values (using mangled names)
        auto globalStructLayout = getGlobalStructLayout(newProgramLayout);
        for (auto globalVarLayout : globalStructLayout->fields)
        {
            auto mangledName = compileRequest->mSession->getNameObj(getMangledName(globalVarLayout->varDecl));
            context->globalVarLayouts.AddIfNotExists(mangledName, globalVarLayout);
        }

        // for now, clone all unreferenced witness tables
        for (auto sym :context->getSymbols())
        {
            if (sym.Value->irGlobalValue->op == kIROp_WitnessTable)
                cloneGlobalValue(context, (IRWitnessTable*)sym.Value->irGlobalValue);
        }
        return state;
    }

    void destroyIRSpecializationState(IRSpecializationState* state)
    {
        delete state;
    }

    IRModule* getIRModule(IRSpecializationState* state)
    {
        return state->irModule;
    }

    IRGlobalValue* getSpecializedGlobalValueForDeclRef(
        IRSpecializationState*  state,
        DeclRef<Decl> const&    declRef)
    {
        // We will start be ensuring that we have code for
        // the declaration itself.
        auto decl = declRef.getDecl();
        auto mangledDeclName = getMangledName(decl);

        IRGlobalValue* irDeclVal = cloneGlobalValueWithMangledName(
            state->getContext(),
            state->getContext()->getModule()->session->getNameObj(mangledDeclName));
        if(!irDeclVal)
            return nullptr;

        // Now we need to deal with specializing the given
        // IR value based on the substitutions applied to
        // our declaration reference.

        if(!declRef.substitutions)
            return irDeclVal;

        SLANG_UNEXPECTED("unhandled");
        UNREACHABLE_RETURN(nullptr);
    }

    void specializeIRForEntryPoint(
        IRSpecializationState*  state,
        EntryPointRequest*  entryPointRequest,
        ExtensionUsageTracker*  extensionUsageTracker)
    {
        auto target = state->target;

        auto compileRequest = entryPointRequest->compileRequest;
        auto session = compileRequest->mSession;
        auto translationUnit = entryPointRequest->getTranslationUnit();
        auto originalIRModule = translationUnit->irModule;
        if (!originalIRModule)
        {
            // We should already have emitted IR for the original
            // translation unit, and it we don't have it, then
            // we are now in trouble.
            return;
        }

        auto context = state->getContext();
        auto newProgramLayout = state->newProgramLayout;

        auto entryPointLayout = findEntryPointLayout(newProgramLayout, entryPointRequest);


        // Next, we make sure to clone the global value for
        // the entry point function itself, and rely on
        // this step to recursively copy over anything else
        // it might reference.
        auto irEntryPoint = specializeIRForEntryPoint(context, entryPointRequest, entryPointLayout);

        // HACK: right now the bindings for global generic parameters are coming in
        // as part of the original IR module, and we need to make sure these get
        // copied over, even if they aren't referenced.
        //
        for(auto inst : originalIRModule->getGlobalInsts())
        {
            auto bindInst = as<IRBindGlobalGenericParam>(inst);
            if(!bindInst)
                continue;

            cloneValue(context, bindInst);
        }


        // TODO: *technically* we should consider the case where
        // we have global variables with initializers, since
        // these should get run whether or not the entry point
        // references them.

        // For GLSL only, we will need to perform "legalization" of
        // the entry point and any entry-point parameters.
        switch (target)
        {
        case CodeGenTarget::GLSL:
            {
                legalizeEntryPointForGLSL(
                    session,
                    context->getModule(),
                    irEntryPoint,
                    entryPointLayout,
                    &compileRequest->mSink,
                    extensionUsageTracker);
            }
            break;

        default:
            break;
        }
    }

    struct IRGenericSpecContext : IRSpecContextBase
    {
        IRSpecContextBase* parent = nullptr;

        IRSharedSpecContext* getShared() { return shared; }

        // Override the "maybe clone" logic so that we always clone
        virtual IRInst* maybeCloneValue(IRInst* originalVal) override;
    };

    IRInst* IRGenericSpecContext::maybeCloneValue(IRInst* originalVal)
    {
        if (parent)
        {
            return parent->maybeCloneValue(originalVal);
        }
        else
        {
            return originalVal;
        }
    }

    // See the work list for the generic spec context with
    // every relevant instruction from `inst` through its
    // descendents.
    void addToSpecializationWorkListRec(
        IRSharedGenericSpecContext* sharedContext,
        IRInst*                     inst)
    {
        if(auto genericInst = as<IRGeneric>(inst))
        {
            // We do *not* consider generics, or instructions nested under them.
            return;
        }
        else if(auto parentInst = as<IRParentInst>(inst))
        {
            // For a parent instruction, we will scan through its contents,
            // since that will be where the `specialize` instructions are

            for(auto child : parentInst->children)
            {
                addToSpecializationWorkListRec(sharedContext, child);
            }
        }
        else
        {
            // Default case: consider this instruction for specialization.
            sharedContext->addToWorkList(inst);
        }
    }

    IRInst* specializeGeneric(
        IRSharedGenericSpecContext* sharedContext,
        IRSpecContextBase*          parentContext,
        IRGeneric*                  genericVal,
        IRSpecialize*               specializeInst)
    {
        // First, we want to see if an existing specialization
        // has already been made. To do that we will need to
        // compute the mangled name of the specialized value,
        // so that we can look for existing declarations.
        String specMangledName = mangleSpecializedFuncName(getText(genericVal->mangledName), specializeInst);
        auto specMangledNameObj = sharedContext->module->session->getNameObj(specMangledName);

        // Now look up an existing symbol with a matching name
        RefPtr<IRSpecSymbol> symb;
        if (sharedContext->symbols.TryGetValue(specMangledNameObj, symb))
        {
            return symb->irGlobalValue;
        }

        // TODO: This is a terrible linear search, and we should
        // avoid it by building a dictionary ahead of time,
        // as is being done for the `IRSpecContext` used above.
        // We can probalby use the same basic context, actually.
        for(auto ii : sharedContext->module->getGlobalInsts())
        {
            auto gv = as<IRGlobalValue>(ii);
            if (!gv)
                continue;

            if (gv->mangledName == specMangledNameObj)
                return gv;
        }

        // If we get to this point, then we need to construct a
        // new IR value to represent the result of specialization.

        // We need to establish a new mapping from inst->inst to
        // handle the specialization, because we don't want the
        // clones we register in this pass to cause confusion
        // in later steps that might clone the same code.

        IRSpecEnv env;
        env.parent = &sharedContext->globalEnv;
        if (parentContext)
        {
            env.parent = parentContext->getEnv();
        }

        // The result of specialization should be inserted
        // into the global scope, at the same location as
        // the original generic.
        IRBuilder builderStorage;
        IRBuilder* builder = &builderStorage;
        builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
        builder->setInsertBefore(genericVal);

        IRGenericSpecContext context;
        context.shared = sharedContext;
        context.parent = parentContext;
        context.builder = builder;
        context.env = &env;

        // Register the arguments of the `specialize` instruction to be used
        // as the "cloned" value for each of the parameters of the generic.
        //
        UInt argCounter = 0;
        for (auto param = genericVal->getFirstParam(); param; param = param->getNextParam())
        {
            UInt argIndex = argCounter++;
            SLANG_ASSERT(argIndex < specializeInst->getArgCount());

            IRInst* arg = specializeInst->getArg(argIndex);

            registerClonedValue(&context, arg, param);
        }

        // Okay, now we want to run through the body of the generic
        // and clone stuff into the parent scope (which had
        // better be the global scope).
        for (auto bb : genericVal->getBlocks())
        {
            // We expect a generic to only ever contain a single block.
            SLANG_ASSERT(bb == genericVal->getFirstBlock());

            for (auto ii : bb->getChildren())
            {
                // Skip parameters, since they were handled earlier.
                if (auto param = as<IRParam>(ii))
                    continue;

                // The last block of the generic is expected to end with
                // a `return` instruction for the specialized value that
                // comes out of the abstraction.
                //
                // We thus use that cloned value as the result of the
                // specialization step.
                if (auto returnValInst = as<IRReturnVal>(ii))
                {
                    auto clonedResult = cloneValue(&context, returnValInst->getVal());
                    if (auto clonedGlobalValue = as<IRGlobalValue>(clonedResult))
                    {
                        clonedGlobalValue->mangledName = specMangledNameObj;

                        // TODO: create a symbol for it and add it to the map.
                    }

                    return clonedResult;
                }

                // Otherwise, clone the instruction into the global scope
                IRInst* clonedInst = cloneInst(&context, context.builder, ii);

                // Now that we've cloned the instruction to a location outside
                // of a generic, we should consider whether it can now be specialized.
                addToSpecializationWorkListRec(sharedContext, clonedInst);
            }
        }

        // If we reach this point, something went wrong, because we
        // never encountered a `return` inside the body of the generic.
        SLANG_UNEXPECTED("no return from generic");
        UNREACHABLE_RETURN(nullptr);
    }

    // Find the value in the given witness table that
    // satisfies the given requirement (or return
    // null if not found).
    IRInst* findWitnessVal(
        IRWitnessTable* witnessTable,
        IRInst*         requirementKey)
    {
        // For now we will do a dumb linear search
        for( auto entry : witnessTable->getEntries() )
        {
            // If the keys matched, then we use the value from this entry.
            if (requirementKey == entry->requirementKey.get())
            {
                auto satisfyingVal = entry->satisfyingVal.get();
                return satisfyingVal;
            }
        }

        // No matching entry found.
        return nullptr;
    }

    static bool canSpecializeGeneric(
        IRGeneric*  generic)
    {
        IRGeneric* g = generic;
        for(;;)
        {
            auto val = findGenericReturnVal(g);
            if(!val)
                return false;

            if (auto nestedGeneric = as<IRGeneric>(val))
            {
                // The outer generic returns an *inner* generic
                // (so that multiple calls to `specialize` are
                // needed to resolve it). We should look at
                // what the nested generic returns to figure
                // out whether specialization is allowed.
                g = nestedGeneric;
                continue;
            }

            // We've found the leaf value that will be produced after
            // all of the specialization is done. Now we want to know
            // if that is a value suitable for actually specializing

            if (auto globalValue = as<IRGlobalValue>(val))
            {
                if (isDefinition(globalValue))
                    return true;
                return false;
            }
            else
            {
                // There might be other cases with a declaration-vs-definition
                // thing that we need to handle.

                return true;
            }
        }
    }

    // Add any instruction that uses `inst` to the work list,
    // so that it can be evaluated (or re-evaluated) for specialization.
    void addUsesToWorkList(
        IRSharedGenericSpecContext* sharedContext,
        IRInst*                     inst)
    {
        for(auto u = inst->firstUse; u; u = u->nextUse)
        {
            sharedContext->addToWorkList(u->getUser());
        }
    }

    void specializeGenericsForInst(
        IRSharedGenericSpecContext* sharedContext,
        IRInst*                     inst)
    {
        switch(inst->op)
        {
        default:
            // The default behavior is to do nothing.
            // An instruction is specialize-able once its operands
            // are specialized, and after that it is also safe
            // to consider the instruction specialized.
            break;

        case kIROp_Specialize:
            {
                // We have a `specialize` instruction, so lets see
                // whether we have an opportunity to perform the
                // specialization here and now.
                IRSpecialize* specInst = cast<IRSpecialize>(inst);

                // Look at the base of the `specialize`, and see if
                // it directly names a generic, so that we can apply
                // specialization here and now.
                auto baseVal = specInst->getBase();
                if(auto genericVal = as<IRGeneric>(baseVal))
                {
                    if (canSpecializeGeneric(genericVal))
                    {
                        // Okay, we have a candidate for specialization here.
                        //
                        // We will apply the specialization logic to the body of the generic,
                        // which will yield, e.g., a specialized `IRFunc`.
                        //
                        auto specializedVal = specializeGeneric(sharedContext, nullptr, genericVal, specInst);
                        //
                        // Then we will replace the use sites for the `specialize`
                        // instruction with uses of the specialized value.
                        //
                        addUsesToWorkList(sharedContext, specInst);
                        specInst->replaceUsesWith(specializedVal);
                        specInst->removeAndDeallocate();
                    }
                }
            }
            break;

        case kIROp_lookup_interface_method:
            {
                // We have a `lookup_interface_method` instruction,
                // so let's see whether it is a lookup in a known
                // witness table.
                IRLookupWitnessMethod* lookupInst = cast<IRLookupWitnessMethod>(inst);

                // We only want to deal with the case where the witness-table
                // argument points to a concrete global table (and not, e.g., a
                // `specialize` instruction that will yield a table)
                auto witnessTable = as<IRWitnessTable>(lookupInst->witnessTable.get());
                if(!witnessTable)
                    break;

                // Use the witness table to look up the value that
                // satisfies the requirement.
                auto requirementKey = lookupInst->getRequirementKey();
                auto satisfyingVal = findWitnessVal(witnessTable, requirementKey);
                // We expect to always find something, but lets just
                // be careful here.
                if(!satisfyingVal)
                    break;

                // If we get through all of the above checks, then we
                // have a (more) concrete method that implements the interface,
                // and so we should dispatch to that directly, rather than
                // use the `lookup_interface_method` instruction.
                addUsesToWorkList(sharedContext, lookupInst);
                lookupInst->replaceUsesWith(satisfyingVal);
                lookupInst->removeAndDeallocate();
            }
            break;
        }
    }

    static bool isInstSpecialized(
        IRSharedGenericSpecContext* sharedContext,
        IRInst*                     inst)
    {
        // If an instruction is still on our work list, then
        // it isn't specialized, and conversely we say that
        // if it *isn't* on the work list, it must be specialized.
        //
        // Note: if we end up with bugs in this logic, we could
        // maintain an explicit set of specialized insts instead.
        //
        return !sharedContext->workListSet.Contains(inst);
    }

    static bool canSpecializeInst(
        IRSharedGenericSpecContext* sharedContext,
        IRInst*                     inst)
    {
        // We can specialize an instruction once all its
        // operands are specialized.

        UInt operandCount = inst->getOperandCount();
        for(UInt ii = 0; ii < operandCount; ++ii)
        {
            IRInst* operand = inst->getOperand(ii);
            if(!isInstSpecialized(sharedContext, operand))
                return false;
        }
        return true;
    }

    // Go through the code in the module and try to identify
    // calls to generic functions where the generic arguments
    // are known, and specialize the callee based on those
    // known values.
    void specializeGenerics(
        IRModule*   module,
        CodeGenTarget target)
    {
        IRSharedGenericSpecContext sharedContextStorage;
        auto sharedContext = &sharedContextStorage;

        initializeSharedSpecContext(
            sharedContext,
            module->session,
            module,
            module,
            target);

        auto moduleInst = module->getModuleInst();

        // First things first, let's deal with any bindings for global generic parameters.
        for(auto inst : moduleInst->getChildren())
        {
            auto bindInst = as<IRBindGlobalGenericParam>(inst);
            if(!bindInst)
                continue;

            // HACK: Our current front-end emit logic can end up emitting multiple
            // `bindGlobalGeneric` instructions for the same parameter. This is
            // a buggy behavior, but a real fix would require refactoring the way
            // global generic arguments are specified today.
            //
            // For now we will do a sanity check to detect parameters that
            // have already been specialized.
            if( !as<IRGlobalGenericParam>(bindInst->getOperand(0)) )
            {
                // parameter operand is no longer a parameter, so it
                // seems things must have been specialized already.
                continue;
            }

            auto param = bindInst->getParam();
            auto val = bindInst->getVal();

            param->replaceUsesWith(val);
        }
        {
            // Now we will do a second pass to clean up the
            // generic parameters and their bindings.
            IRInst* next = nullptr;
            for(auto inst = moduleInst->getFirstChild(); inst; inst = next)
            {
                next = inst->getNextInst();

                switch(inst->op)
                {
                default:
                    break;

                case kIROp_GlobalGenericParam:
                case kIROp_BindGlobalGenericParam:
                    // A "bind" instruction should have no uses in the
                    // first place, and all the global generic parameters
                    // should have had their uses replaced.
                    SLANG_ASSERT(!inst->firstUse);
                    inst->removeAndDeallocate();
                    break;
                }
            }
        }

        // Our goal here is to find `specialize` instructions that
        // can be replaced with references to, e.g., a suitably
        // specialized function, and to resolve any `lookup_interface_method`
        // instructions to the concrete value fetched from a witness
        // table.
        //
        // We need to be careful of a few things:
        //
        // * It would not in general make sense to consider specialize-able
        //   instructions under an `IRGeneric`, since that could mean "specialziing"
        //   code to parameter values that are still unknown.
        //
        // * We *also* need to be careful not to specialize something when one
        //   or more of its inputs is also a `specialize` or `lookup_interface_method`
        //   instruction, because then we'd be propagating through non-concrete
        //   values.
        //
        // The approach we use here is to build a work list of instructions
        // that *can* become fully specialized, but aren't yet. Any
        // instruction on the work list will be considered to be "unspecialized"
        // and any instruction not on the work list is considered specialized.
        //
        // We will start by recursively walking all the instructions to add
        // the appropriate ones to  our work list:
        //
        addToSpecializationWorkListRec(sharedContext, moduleInst);

        // Now we are going to repeatedly walk our work list, and filter
        // it to create a new work list.
        List<IRInst*> workListCopy;
        for(;;)
        {
            // Swap out the work list on the context so we can
            // process it here without worrying about concurrent
            // modifications.
            workListCopy.Clear();
            workListCopy.SwapWith(sharedContext->workList);

            if(workListCopy.Count() == 0)
                break;

            for(auto inst : workListCopy)
            {
                // We need to check whether it is possible to specialize
                // the instruction yet (it might not be because its
                // operands haven't been specialized)
                if(!canSpecializeInst(sharedContext, inst))
                {
                    // Put it back on the fresh work list, so that
                    // we can re-consider it in another iteration.
                    sharedContext->workList.Add(inst);
                }
                else
                {
                    // Okay, perform any specialization step on this
                    // instruction that makes sense (which might be
                    // doing nothing).
                    specializeGenericsForInst(sharedContext, inst);

                    // Remove the instruction from consideration.
                    sharedContext->workListSet.Remove(inst);
                }
            }
        }

        // Once the work list has gone dry, we should have the invariant
        // that there are no `specialize` instructions inside of non-generic
        // functions that in turn reference a generic function, *except*
        // in the case where that generic is for a builtin function, in
        // which case we wouldn't want to specialize it anyway.
    }

    void applyGlobalGenericParamSubstitution(
        IRSpecContext*  /*context*/)
    {
        // TODO: we need to figure out how to apply this
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
