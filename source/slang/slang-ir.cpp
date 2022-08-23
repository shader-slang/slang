// slang-ir.cpp
#include "slang-ir.h"
#include "slang-ir-insts.h"

#include "../core/slang-basic.h"

#include "slang-mangle.h"

namespace Slang
{
    struct IRSpecContext;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!! DiagnosticSink Impls !!!!!!!!!!!!!!!!!!!!!

    SourceLoc const& getDiagnosticPos(IRInst* inst)
    {
        return inst->sourceLoc;
    }

    void printDiagnosticArg(StringBuilder& sb, IRInst* irObject)
    {
        if (auto nameHint = irObject->findDecoration<IRNameHintDecoration>())
            sb << nameHint->getName();
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


    bool isSimpleDecoration(IROp op)
    {
        switch (op)
        {
            case kIROp_EarlyDepthStencilDecoration: 
            case kIROp_GloballyCoherentDecoration: 
            case kIROp_KeepAliveDecoration: 
            case kIROp_LineAdjInputPrimitiveTypeDecoration: 
            case kIROp_LineInputPrimitiveTypeDecoration: 
            case kIROp_NoInlineDecoration: 
            case kIROp_PointInputPrimitiveTypeDecoration: 
            case kIROp_PreciseDecoration: 
            case kIROp_PublicDecoration: 
            case kIROp_HLSLExportDecoration: 
            case kIROp_ReadNoneDecoration: 
            case kIROp_RequiresNVAPIDecoration: 
            case kIROp_TriangleAdjInputPrimitiveTypeDecoration:
            case kIROp_TriangleInputPrimitiveTypeDecoration:
            case kIROp_UnsafeForceInlineEarlyDecoration:
            case kIROp_VulkanCallablePayloadDecoration:
            case kIROp_VulkanHitAttributesDecoration:
            case kIROp_VulkanRayPayloadDecoration:
            {
                return true;
            }
            default: break;
        }
        return false;
    }

    
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
#include "slang-ir-inst-defs.h"

    // Invalid op sentinel value comes after all the valid ones
    { kIROp_Invalid,{ "invalid", 0, 0 } },
    };

    IROpInfo getIROpInfo(IROp opIn)
    {
        const int op = opIn & kIROpMeta_OpMask;
        if (op < kIROpCount)
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
            if(dd->getOp() == decorationOp)
                return dd;
        }
        return nullptr;
    }

    IROperandList<IRAttr> IRInst::getAllAttrs()
    {
        // We assume as an invariant that all attributes appear at the end of the operand
        // list, after all the non-attribute operands.
        //
        // We will therefore define a range that ends at the end of the operand list ...
        //
        IRUse* end = getOperands() + getOperandCount();
        //
        // ... and begins after the last non-attribute operand.
        //
        IRUse* cursor = getOperands();
        while(cursor != end && !as<IRAttr>(cursor->get()))
            cursor++;

        return IROperandList<IRAttr>(cursor, end);
    }

    // IRConstant

    IRIntegerValue getIntVal(IRInst* inst)
    {
        switch (inst->getOp())
        {
        default:
            SLANG_UNEXPECTED("needed a known integer value");
            UNREACHABLE_RETURN(0);

        case kIROp_IntLit:
            return static_cast<IRConstant*>(inst)->value.intVal;
            break;
        }
    }

    // IRCapabilitySet

    CapabilitySet IRCapabilitySet::getCaps()
    {
        List<CapabilityAtom> atoms;

        Index count = (Index) getOperandCount();
        for(Index i = 0; i < count; ++i)
        {
            auto operand = cast<IRIntLit>(getOperand(i));
            atoms.add(CapabilityAtom(operand->getValue()));
        }

        return CapabilitySet(atoms.getCount(), atoms.getBuffer());
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

    // IRPtrTypeBase

    IRType* tryGetPointedToType(
        IRBuilder*  builder,
        IRType*     type)
    {
        if( auto rateQualType = as<IRRateQualifiedType>(type) )
        {
            type = rateQualType->getValueType();
        }

        // The "true" pointers and the pointer-like stdlib types are the easy cases.
        if( auto ptrType = as<IRPtrTypeBase>(type) )
        {
            return ptrType->getValueType();
        }
        else if( auto ptrLikeType = as<IRPointerLikeType>(type) )
        {
            return ptrLikeType->getElementType();
        }
        //
        // A more interesting case arises when we have a `BindExistentials<P<T>, ...>`
        // where `P<T>` is a pointer(-like) type.
        //
        else if( auto bindExistentials = as<IRBindExistentialsType>(type) )
        {
            // We know that `BindExistentials` won't introduce its own
            // existential type parameters, nor will any of the pointer(-like)
            // type constructors `P`.
            //
            // Thus we know that the type that is pointed to should be
            // the same as `BindExistentials<T, ...>`.
            //
            auto baseType = bindExistentials->getBaseType();
            if( auto baseElementType = tryGetPointedToType(builder, baseType) )
            {
                UInt existentialArgCount = bindExistentials->getExistentialArgCount();
                List<IRInst*> existentialArgs;
                for( UInt ii = 0; ii < existentialArgCount; ++ii )
                {
                    existentialArgs.add(bindExistentials->getExistentialArg(ii));
                }
                return builder->getBindExistentialsType(
                    baseElementType,
                    existentialArgCount,
                    existentialArgs.getBuffer());
            }
        }

        // TODO: We may need to handle other cases here.

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

    // Similar to addParam, but instead of appending `param` to the end
    // of the parameter list, this function inserts `param` before the
    // head of the list.
    void IRBlock::insertParamAtHead(IRParam* param)
    {
        if (auto firstParam = getFirstParam())
        {
            param->insertBefore(firstParam);
        }
        else if (auto firstOrdinary = getFirstOrdinaryInst())
        {
            param->insertBefore(firstOrdinary);
        }
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
        switch (terminator->getOp())
        {
        case kIROp_Return:
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
        switch(getOp())
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
        switch(getOp())
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

    IRInst* IRGlobalValueWithParams::getFirstOrdinaryInst()
    {
        auto firstBlock = getFirstBlock();
        if (!firstBlock)
            return nullptr;
        return firstBlock->getFirstOrdinaryInst();
    }

    // IRFunc

    IRType* IRFunc::getResultType() { return getDataType()->getResultType(); }
    UInt IRFunc::getParamCount() { return getDataType()->getParamCount(); }
    IRType* IRFunc::getParamType(UInt index) { return getDataType()->getParamType(index); }

    void IRGlobalValueWithCode::addBlock(IRBlock* block)
    {
        block->insertAtEnd(this);
    }

    void fixUpFuncType(IRFunc* func, IRType* resultType)
    {
        SLANG_ASSERT(func);

        auto irModule = func->getModule();
        SLANG_ASSERT(irModule);

        SharedIRBuilder sharedBuilder(irModule);
        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(func);

        List<IRType*> paramTypes;
        for(auto param : func->getParams())
        {
            paramTypes.add(param->getFullType());
        }

        auto funcType = builder.getFuncType(paramTypes, resultType);
        builder.setDataType(func, funcType);
    }

    void fixUpFuncType(IRFunc* func)
    {
        fixUpFuncType(func, func->getResultType());
    }

    //

    bool isTerminatorInst(IROp op)
    {
        switch (op)
        {
        default:
            return false;

        case kIROp_Return:
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
        return isTerminatorInst(inst->getOp());
    }

    //
    // IRTypeLayout
    //

    IRTypeSizeAttr* IRTypeLayout::findSizeAttr(LayoutResourceKind kind)
    {
        // TODO: If we could assume the attributes were sorted
        // by `kind`, then we could use a binary search here
        // instead of linear.
        //
        // In practice, the number of entries will be very small,
        // so the cost of the linear search should not be too bad.

        for( auto sizeAttr : getSizeAttrs() )
        {
            if(sizeAttr->getResourceKind() == kind)
                return sizeAttr;
        }
        return nullptr;
    }

    IRTypeLayout* IRTypeLayout::unwrapArray()
    {
        auto typeLayout = this;
        while(auto arrayTypeLayout = as<IRArrayTypeLayout>(typeLayout))
            typeLayout = arrayTypeLayout->getElementTypeLayout();
        return typeLayout;
    }

    IRTypeLayout* IRTypeLayout::getPendingDataTypeLayout()
    {
        if(auto attr = findAttr<IRPendingLayoutAttr>())
            return cast<IRTypeLayout>(attr->getLayout());
        return nullptr;
    }

    IROperandList<IRTypeSizeAttr> IRTypeLayout::getSizeAttrs()
    {
        return findAttrs<IRTypeSizeAttr>();
    }

    IRTypeLayout::Builder::Builder(IRBuilder* irBuilder)
        : m_irBuilder(irBuilder)
    {}

    void IRTypeLayout::Builder::addResourceUsage(
        LayoutResourceKind  kind,
        LayoutSize          size)
    {
        auto& resInfo = m_resInfos[Int(kind)];
        resInfo.kind = kind;
        resInfo.size += size;
    }

    void IRTypeLayout::Builder::addResourceUsage(IRTypeSizeAttr* sizeAttr)
    {
        addResourceUsage(
            sizeAttr->getResourceKind(),
            sizeAttr->getSize());
    }

    void IRTypeLayout::Builder::addResourceUsageFrom(IRTypeLayout* typeLayout)
    {
        for( auto sizeAttr : typeLayout->getSizeAttrs() )
        {
            addResourceUsage(sizeAttr);
        }
    }

    IRTypeLayout* IRTypeLayout::Builder::build()
    {
        IRBuilder* irBuilder = getIRBuilder();

        List<IRInst*> operands;

        addOperands(operands);
        addAttrs(operands);

        return irBuilder->getTypeLayout(
            getOp(),
            operands);
    }

    void IRTypeLayout::Builder::addOperands(List<IRInst*>& operands)
    {
        addOperandsImpl(operands);
    }

    void IRTypeLayout::Builder::addAttrs(List<IRInst*>& operands)
    {
        auto irBuilder = getIRBuilder();

        for(auto resInfo : m_resInfos)
        {
            if(resInfo.kind == LayoutResourceKind::None)
                continue;

            IRInst* sizeAttr = irBuilder->getTypeSizeAttr(
                resInfo.kind,
                resInfo.size);
            operands.add(sizeAttr);
        }

        if( auto pendingTypeLayout = m_pendingTypeLayout )
        {
            operands.add(irBuilder->getPendingLayoutAttr(
                pendingTypeLayout));
        }

        addAttrsImpl(operands);
    }

    //
    // IRParameterGroupTypeLayout
    //

    void IRParameterGroupTypeLayout::Builder::addOperandsImpl(List<IRInst*>& ioOperands)
    {
        ioOperands.add(m_containerVarLayout);
        ioOperands.add(m_elementVarLayout);
        ioOperands.add(m_offsetElementTypeLayout);
    }

    IRParameterGroupTypeLayout* IRParameterGroupTypeLayout::Builder::build()
    {
        return cast<IRParameterGroupTypeLayout>(Super::Builder::build());
    }

    //
    // IRStructTypeLayout
    //

    void IRStructTypeLayout::Builder::addAttrsImpl(List<IRInst*>& ioOperands)
    {
        auto irBuilder = getIRBuilder();
        for(auto field : m_fields)
        {
            ioOperands.add(
                irBuilder->getFieldLayoutAttr(field.key, field.layout));
        }
    }

    //
    // IRArrayTypeLayout
    //

    void IRArrayTypeLayout::Builder::addOperandsImpl(List<IRInst*>& ioOperands)
    {
        ioOperands.add(m_elementTypeLayout);
    }

    //
    // IRStreamOutputTypeLayout
    //

    void IRStreamOutputTypeLayout::Builder::addOperandsImpl(List<IRInst*>& ioOperands)
    {
        ioOperands.add(m_elementTypeLayout);
    }

    //
    // IRMatrixTypeLayout
    //

    IRMatrixTypeLayout::Builder::Builder(IRBuilder* irBuilder, MatrixLayoutMode mode)
        : Super::Builder(irBuilder)
    {
        m_modeInst = irBuilder->getIntValue(irBuilder->getIntType(), IRIntegerValue(mode));
    }

    void IRMatrixTypeLayout::Builder::addOperandsImpl(List<IRInst*>& ioOperands)
    {
        ioOperands.add(m_modeInst);
    }

    //
    // IRTaggedUnionTypeLayout
    //

    IRTaggedUnionTypeLayout::Builder::Builder(IRBuilder* irBuilder, LayoutSize tagOffset)
        : Super::Builder(irBuilder)
    {
        m_tagOffset = irBuilder->getIntValue(irBuilder->getIntType(), tagOffset.raw);
    }

    void IRTaggedUnionTypeLayout::Builder::addCaseTypeLayout(IRTypeLayout* typeLayout)
    {
        m_caseTypeLayoutAttrs.add(getIRBuilder()->getCaseTypeLayoutAttr(typeLayout));
    }

    void IRTaggedUnionTypeLayout::Builder::addOperandsImpl(List<IRInst*>& ioOperands)
    {
        ioOperands.add(m_tagOffset);
    }

    void IRTaggedUnionTypeLayout::Builder::addAttrsImpl(List<IRInst*>& ioOperands)
    {
        for(auto attr : m_caseTypeLayoutAttrs)
            ioOperands.add(attr);
    }

    //
    // IRVarLayout
    //

    bool IRVarLayout::usesResourceKind(LayoutResourceKind kind)
    {
        // TODO: basing this check on whether or not the
        // var layout has an entry for `kind` means that
        // we can't just optimize away any entry where
        // the offset is zero (which might be a small
        // but nice optimization). We could consider shifting
        // this test to use the entries on the type layout
        // instead (since non-zero resource consumption
        // should be an equivalent test).

        return findOffsetAttr(kind) != nullptr;
    }

    IRSystemValueSemanticAttr* IRVarLayout::findSystemValueSemanticAttr()
    {
        return findAttr<IRSystemValueSemanticAttr>();
    }

    IRVarOffsetAttr* IRVarLayout::findOffsetAttr(LayoutResourceKind kind)
    {
        for( auto offsetAttr : getOffsetAttrs() )
        {
            if(offsetAttr->getResourceKind() == kind)
                return offsetAttr;
        }
        return nullptr;
    }

    IROperandList<IRVarOffsetAttr> IRVarLayout::getOffsetAttrs()
    {
        return findAttrs<IRVarOffsetAttr>();
    }

    Stage IRVarLayout::getStage()
    {
        if(auto stageAttr = findAttr<IRStageAttr>())
            return stageAttr->getStage();
        return Stage::Unknown;
    }

    IRVarLayout* IRVarLayout::getPendingVarLayout()
    {
        if( auto pendingLayoutAttr = findAttr<IRPendingLayoutAttr>() )
        {
            return cast<IRVarLayout>(pendingLayoutAttr->getLayout());
        }
        return nullptr;
    }

    IRVarLayout::Builder::Builder(
        IRBuilder*      irBuilder,
        IRTypeLayout*   typeLayout)
        : m_irBuilder(irBuilder)
        , m_typeLayout(typeLayout)
    {}

    bool IRVarLayout::Builder::usesResourceKind(LayoutResourceKind kind)
    {
        return m_resInfos[Int(kind)].kind != LayoutResourceKind::None;
    }

    IRVarLayout::Builder::ResInfo* IRVarLayout::Builder::findOrAddResourceInfo(LayoutResourceKind kind)
    {
        auto& resInfo = m_resInfos[Int(kind)];
        resInfo.kind = kind;
        return &resInfo;
    }

    void IRVarLayout::Builder::setSystemValueSemantic(String const& name, UInt index)
    {
        m_systemValueSemantic = getIRBuilder()->getSystemValueSemanticAttr(name, index);
    }

    void IRVarLayout::Builder::setUserSemantic(String const& name, UInt index)
    {
        m_userSemantic = getIRBuilder()->getUserSemanticAttr(name, index);
    }

    void IRVarLayout::Builder::setStage(Stage stage)
    {
        m_stageAttr = getIRBuilder()->getStageAttr(stage);
    }

    void IRVarLayout::Builder::cloneEverythingButOffsetsFrom(
        IRVarLayout* that)
    {
        if(auto systemValueSemantic = that->findAttr<IRSystemValueSemanticAttr>())
            m_systemValueSemantic = systemValueSemantic;

        if(auto userSemantic = that->findAttr<IRUserSemanticAttr>())
            m_userSemantic = userSemantic;

        if(auto stageAttr = that->findAttr<IRStageAttr>())
            m_stageAttr = stageAttr;
    }

    IRVarLayout* IRVarLayout::Builder::build()
    {
        SLANG_ASSERT(m_typeLayout);

        IRBuilder* irBuilder = getIRBuilder();

        List<IRInst*> operands;

        operands.add(m_typeLayout);

        for(auto resInfo : m_resInfos)
        {
            if(resInfo.kind == LayoutResourceKind::None)
                continue;

            IRInst* varOffsetAttr = irBuilder->getVarOffsetAttr(
                resInfo.kind,
                resInfo.offset,
                resInfo.space);
            operands.add(varOffsetAttr);
        }

        if(auto semanticAttr = m_userSemantic)
            operands.add(semanticAttr);

        if(auto semanticAttr = m_systemValueSemantic)
            operands.add(semanticAttr);

        if(auto stageAttr = m_stageAttr)
            operands.add(stageAttr);

        if(auto pendingVarLayout = m_pendingVarLayout)
        {
            IRInst* pendingLayoutAttr = irBuilder->getPendingLayoutAttr(
                pendingVarLayout);
            operands.add(pendingLayoutAttr);
        }

        return irBuilder->getVarLayout(operands);
    }

    //
    // IREntryPointLayout
    //

    IRStructTypeLayout* getScopeStructLayout(IREntryPointLayout* scopeLayout)
    {
        auto scopeTypeLayout = scopeLayout->getParamsLayout()->getTypeLayout();

        if( auto constantBufferTypeLayout = as<IRParameterGroupTypeLayout>(scopeTypeLayout) )
        {
            scopeTypeLayout = constantBufferTypeLayout->getOffsetElementTypeLayout();
        }

        if( auto structTypeLayout = as<IRStructTypeLayout>(scopeTypeLayout) )
        {
            return structTypeLayout;
        }

        SLANG_UNEXPECTED("uhandled global-scope binding layout");
        UNREACHABLE_RETURN(nullptr);
    }

    //

    IRInst* IRInsertLoc::getParent() const
    {
        auto inst = getInst();
        switch(getMode())
        {
        default:
        case Mode::None:
            return nullptr;
        case Mode::Before:
        case Mode::After:
            return inst->getParent();
        case Mode::AtStart:
        case Mode::AtEnd:
            return inst;
        }
    }

    IRBlock* IRInsertLoc::getBlock() const
    {
        return as<IRBlock>(getParent());
    }

    // Get the current function (or other value with code)
    // that we are inserting into (if any).
    IRGlobalValueWithCode* IRInsertLoc::getFunc() const
    {
        auto pp = getParent();
        if (auto block = as<IRBlock>(pp))
        {
            pp = pp->getParent();
        }
        return as<IRGlobalValueWithCode>(pp);
    }

    // Add an instruction into the current scope
    void IRBuilder::addInst(
        IRInst*     inst)
    {
        inst->insertAt(m_insertLoc);
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

    IRInst* IRModule::_allocateInst(
        IROp    op,
        Int     operandCount,
        size_t  minSizeInBytes)
    {
        // There are two basic cases for instructions that affect how we compute size:
        //
        // * The default case is that an instruction's state is fully defined by the fields
        //   in the `IRInst` base type, along with the trailing operand list (a tail-allocated
        //   array of `IRUse`s. Almost all instructions need space allocated this way.
        //
        // * A small number of cases (currently `IRConstant`s and the `IRModule` type) have
        //   *zero* operands but include additional state beyond the fields in `IRInst`.
        //   For these cases we want to ensure that at least `sizeof(T)` bytes are allocated,
        //   based on the specific leaf type `T`.
        //
        // We handle the combination of the two cases by just taking the maximum of the two
        // different sizes.
        //
        size_t defaultSize = sizeof(IRInst) + (operandCount) * sizeof(IRUse);
        size_t totalSize = minSizeInBytes > defaultSize ? minSizeInBytes : defaultSize;

        IRInst* inst = (IRInst*) m_memoryArena.allocateAndZero(totalSize);

        // TODO: Is it actually important to run a constructor here?
        new(inst) IRInst();

        inst->operandCount = uint32_t(operandCount);
        inst->m_op = op;

        return inst;
    }

        /// Return whichever of `left` or `right` represents the later point in a common parent
    static IRInst* pickLaterInstInSameParent(
        IRInst* left,
        IRInst* right)
    {
        // When using instructions to represent insertion locations,
        // a null instruction represents the end of the parent block,
        // so if either of the two instructions is null, it indicates
        // the end of the parent, and thus comes later.
        //
        if(!left) return nullptr;
        if(!right) return nullptr;

        // In the non-null case, we must have the precondition that
        // the two candidates have the same parent.
        //
        SLANG_ASSERT(left->getParent() == right->getParent());

        // No matter what, figuring out which instruction comes first
        // is a linear-time operation in the number of instructions
        // in the same parent, but we can optimize based on the
        // assumption that in common cases one of the following will
        // hold:
        //
        // * `left` and `right` are close to one another in the IR
        // * `left` and/or `right` is close to the start of its parent
        //
        // To optimize for those conditions, we create two cursors that
        // start at `left` and `right` respectively, and scan backward.
        //
        auto ll = left;
        auto rr = right;
        for(;;)
        {
            // If one of the cursors runs into the other while scanning
            // backwards, then it implies it must have been the later
            // of the two.
            //
            // This is our early-exit condition for `left` and `right`
            // being close together.
            //
            // Note: this condition will trigger on the first iteration
            // in the case where `left == right`.
            //
            if(ll == right) return left;
            if(rr == left)  return right;

            // If one of the cursors reaches the start of the block,
            // then that implies it started at the earlier position.
            // In that case, the other candidate must be the later
            // one.
            //
            // This is the early-exit condition for `left` and/or `right`
            // being close to the start of the parent.
            //
            if(!ll) return right;
            if(!rr) return left;

            // Otherwise, we move both cursors backward and continue
            // the search.
            //
            ll = ll->getPrevInst();
            rr = rr->getPrevInst();

            // Note: in the worst case, one of the cursors is
            // at the end of the parent, and the other is halfway
            // through, so that each cursor needs to visit half
            // of the instructions in the parent before we reach
            // one of our termination conditions.
            //
            // As a result the worst-case running time is still O(N),
            // and there is nothing we can do to improve that
            // with our linked-list representation.
            //
            // If the assumptions given turn out to be wrong, and
            // we find that a common case is instructions close
            // to the *end* of a block, we can either flip the
            // direction that the cursors traverse, or even add
            // two more cursors that scan forward instead of
            // backward.
        }
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

        // We better have ended up with a parent to insert into,
        // or else the invariants of our IR have been violated.
        //
        SLANG_ASSERT(parent);

        // Once we determine the parent instruction that the
        // new instruction should be inserted into, we need
        // to find an appropriate place to insert it.
        //
        // There are two concerns at play here, both of which
        // stem from the property that within a block we
        // require definitions to precede their uses.
        //
        // The first concern is that we want to emit a
        // "hoistable" instruction like a type as early as possible,
        // so that if a subsequent optimization pass requests
        // the same type/value again, it doesn't get a cached/deduplicated
        // pointer to an instruction that comes after the code being
        // processed.
        //
        // The second concern is that we must emit any hoistable
        // instruction after any of its operands (or its type)
        // if they come from the same block/parent.
        //
        // These two conditions together indicate that we want
        // to insert the instruction right after whichever of
        // its operands come last in the parent block and if
        // none of the operands come from the same block, we
        // should try to insert it as early as possible in
        // that block.
        //
        // We want to insert a hoistable instruction at the
        // earliest possible point in its parent, which
        // should be right after whichever of its operands
        // is defined in that same block (if any)
        //
        // We will solve this problem by computing the
        // earliest instruction that it would be valid for
        // us to insert before.
        //
        // We start by considering insertion before the
        // first instruction in the parent (if any) and
        // then move the insertion point later as needed.
        //
        // Note: a null `insertBeforeInst` is used
        // here to mean to insert at the end of the parent.
        //
        IRInst* insertBeforeInst = parent->getFirstChild();

        // Hoistable instructions are always "ordinary"
        // instructions, so they need to come after
        // any parameters of the parent.
        //
        while(auto param = as<IRParam>(insertBeforeInst))
            insertBeforeInst = param->getNextInst();

        // For instructions that will be placed at module scope,
        // we don't care about relative ordering, but for everything
        // else, we want to ensure that an instruction comes after
        // its type and operands.
        //
        if( !as<IRModuleInst>(parent) )
        {
            // We need to make sure that if any of
            // the operands of `inst` come from the same
            // block that we insert after them.
            //
            for (UInt ii = 0; ii < operandCount; ++ii)
            {
                auto operand = inst->getOperand(ii);
                if (!operand)
                    continue;

                if(operand->getParent() != parent)
                    continue;

                insertBeforeInst = pickLaterInstInSameParent(insertBeforeInst, operand->getNextInst());
            }
            //
            // Similarly, if the type of `inst` comes from
            // the same parent, then we need to make sure
            // we insert after the type.
            //
            if(auto type = inst->getFullType())
            {
                if(type->getParent() == parent)
                {
                    insertBeforeInst = pickLaterInstInSameParent(insertBeforeInst, type->getNextInst());
                }
            }
        }

        if( insertBeforeInst )
        {
            inst->insertBefore(insertBeforeInst);
        }
        else
        {
            inst->insertAtEnd(parent);
        }
    }

    void IRBuilder::_maybeSetSourceLoc(
        IRInst*     inst)
    {
        auto sourceLocInfo = getSourceLocInfo();
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

        inst->sourceLoc = sourceLocInfo->sourceLoc;
    }

#if SLANG_ENABLE_IR_BREAK_ALLOC
    SLANG_API uint32_t _slangIRAllocBreak = 0xFFFFFFFF;
    uint32_t& _debugGetIRAllocCounter()
    {
        static uint32_t counter = 0;
        return counter;
    }
    uint32_t _debugGetAndIncreaseInstCounter()
    {
        if (_slangIRAllocBreak != 0xFFFFFFFF && _debugGetIRAllocCounter() == _slangIRAllocBreak)
        {
#if _WIN32 && defined(_MSC_VER)
            __debugbreak();
#endif
        }
        return _debugGetIRAllocCounter()++;
    }
#endif

    IRInst* IRBuilder::_createInst(
        size_t                  minSizeInBytes,
        IRType*                 type,
        IROp                    op,
        Int                     fixedArgCount,
        IRInst* const*          fixedArgs,
        Int                     varArgListCount,
        Int const*              listArgCounts,
        IRInst* const* const*   listArgs)
    {
        Int varArgCount = 0;
        for (Int ii = 0; ii < varArgListCount; ++ii)
        {
            varArgCount += listArgCounts[ii];
        }

        Int totalOperandCount = fixedArgCount + varArgCount;

        auto module = getModule();
        SLANG_ASSERT(module);
        IRInst* inst = module->_allocateInst(op, totalOperandCount, minSizeInBytes);

#if SLANG_ENABLE_IR_BREAK_ALLOC
        inst->_debugUID = _debugGetAndIncreaseInstCounter();
#endif

        inst->typeUse.init(inst, type);

        _maybeSetSourceLoc(inst);

        auto operand = inst->getOperands();

        for( Int aa = 0; aa < fixedArgCount; ++aa )
        {
            if (fixedArgs)
            {
                operand->init(inst, fixedArgs[aa]);
            }
            else
            {
                operand->init(inst, nullptr);
            }
            operand++;
        }

        for (Int ii = 0; ii < varArgListCount; ++ii)
        {
            Int listArgCount = listArgCounts[ii];
            for (Int jj = 0; jj < listArgCount; ++jj)
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

    // Create an IR instruction/value and initialize it.
    //
    // In this case `argCount` and `args` represent the
    // arguments *after* the type (which is a mandatory
    // argument for all instructions).
    template<typename T>
    static T* createInstImpl(
        IRBuilder*              builder,
        IROp                    op,
        IRType*                 type,
        Int                     fixedArgCount,
        IRInst* const*          fixedArgs,
        Int                     varArgListCount,
        Int const*              listArgCounts,
        IRInst* const* const*   listArgs)
    {
        return (T*) builder->_createInst(
            sizeof(T),
            type,
            op,
            fixedArgCount,
            fixedArgs,
            varArgListCount,
            listArgCounts,
            listArgs);
    }

    template<typename T>
    static T* createInstImpl(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        Int             fixedArgCount,
        IRInst* const*  fixedArgs,
        Int             varArgCount = 0,
        IRInst* const*  varArgs = nullptr)
    {
        return createInstImpl<T>(
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
    static T* createInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        Int             argCount,
        IRInst* const*  args)
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
        Int             argCount,
        IRInst* const*  args)
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
        Int             fixedArgCount,
        IRInst* const*  fixedArgs,
        Int             varArgCount,
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
        Int             varArgCount,
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
        if(left.inst->getOp() != right.inst->getOp()) return false;
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

    HashCode IRInstKey::getHashCode()
    {
        auto code = Slang::getHashCode(inst->getOp());
        code = combineHash(code, Slang::getHashCode(inst->getFullType()));
        code = combineHash(code, Slang::getHashCode(inst->getOperandCount()));

        auto argCount = inst->getOperandCount();
        auto args = inst->getOperands();
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            code = combineHash(code, Slang::getHashCode(args[aa].get()));
        }
        return code;
    }

    UnownedStringSlice IRConstant::getStringSlice()
    {
        assert(getOp() == kIROp_StringLit);
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

    bool IRConstant::isFinite() const
    {
        SLANG_ASSERT(getOp() == kIROp_FloatLit);
        
        // Lets check we can analyze as double, at least in principal
        SLANG_COMPILE_TIME_ASSERT(sizeof(IRFloatingPointValue) == sizeof(double));
        // We are in effect going to type pun (yay!), lets make sure they are the same size
        SLANG_COMPILE_TIME_ASSERT(sizeof(IRIntegerValue) == sizeof(IRFloatingPointValue));

        const uint64_t i = uint64_t(value.intVal);
        int e = int(i >> 52) & 0x7ff;
        return (e != 0x7ff);
    }

    IRConstant::FloatKind IRConstant::getFloatKind() const
    {
        SLANG_ASSERT(getOp() == kIROp_FloatLit);

        const uint64_t i = uint64_t(value.intVal);
        int e = int(i >> 52) & 0x7ff;
        if ( e == 0x7ff)
        {
            if (i << 12)
            {
                return FloatKind::Nan;
            }
            // Sign bit (top bit) will indicate positive or negative nan
            return value.intVal < 0 ? FloatKind::NegativeInfinity : FloatKind::PositiveInfinity;
        }
        return FloatKind::Finite;
    }

    bool IRConstant::isValueEqual(IRConstant* rhs)
    {
        // If they are literally the same thing.. 
        if (this == rhs)
        {
            return true;
        }
        // Check the type and they are the same op & same type
        if (getOp() != rhs->getOp())
        {
            return false;
        }

        switch (getOp())
        {
            case kIROp_BoolLit:
            case kIROp_FloatLit:
            case kIROp_IntLit:
            {
                SLANG_COMPILE_TIME_ASSERT(sizeof(IRFloatingPointValue) == sizeof(IRIntegerValue));
                // ... we can just compare as bits
                return value.intVal == rhs->value.intVal;
            }
            case kIROp_PtrLit:
            {
                return value.ptrVal == rhs->value.ptrVal;
            }
            case kIROp_StringLit:
            {
                return getStringSlice() == rhs->getStringSlice();
            }
            case kIROp_VoidLit:
            {
                return true;
            }
            default: break;
        }

        SLANG_ASSERT(!"Unhandled type");
        return false;
    }

    /// True if constants are equal
    bool IRConstant::equal(IRConstant* rhs)
    {
        // TODO(JS): Only equal if pointer types are identical (to match how getHashCode works below)
        return isValueEqual(rhs) && getFullType() == rhs->getFullType();
    }

    HashCode IRConstant::getHashCode()
    {
        auto code = Slang::getHashCode(getOp());
        code = combineHash(code, Slang::getHashCode(getFullType()));

        switch (getOp())
        {
            case kIROp_BoolLit:
            case kIROp_FloatLit:
            case kIROp_IntLit:
            {
                SLANG_COMPILE_TIME_ASSERT(sizeof(IRFloatingPointValue) == sizeof(IRIntegerValue));
                // ... we can just compare as bits
                return combineHash(code, Slang::getHashCode(value.intVal));
            }
            case kIROp_PtrLit:
            {
                return combineHash(code, Slang::getHashCode(value.ptrVal));
            }
            case kIROp_StringLit:
            {
                const UnownedStringSlice slice = getStringSlice();
                return combineHash(code, Slang::getHashCode(slice.begin(), slice.getLength()));
            }
            case kIROp_VoidLit:
            {
                return code;
            }
            default:
            {
                SLANG_ASSERT(!"Invalid type");
                return 0;
            }
        }
    }

    IRConstant* IRBuilder::_findOrEmitConstant(
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
        if( getSharedBuilder()->getConstantMap().TryGetValue(key, irValue) )
        {
            // We found a match, so just use that.
            return irValue;
        }
    
        // Calculate the minimum object size (ie not including the payload of value)    
        const size_t prefixSize = SLANG_OFFSET_OF(IRConstant, value);

        switch (keyInst.getOp())
        {
        default:
            SLANG_UNEXPECTED("missing case for IR constant");
            break;

            case kIROp_BoolLit:
            case kIROp_IntLit:
            {
                const size_t instSize = prefixSize + sizeof(IRIntegerValue);
                irValue = static_cast<IRConstant*>(_createInst(instSize, keyInst.getFullType(), keyInst.getOp()));
                irValue->value.intVal = keyInst.value.intVal;
                break; 
            }
            case kIROp_FloatLit:
            {
                const size_t instSize = prefixSize + sizeof(IRFloatingPointValue);
                irValue = static_cast<IRConstant*>(_createInst(instSize, keyInst.getFullType(), keyInst.getOp()));
                irValue->value.floatVal = keyInst.value.floatVal;
                break;
            }
            case kIROp_PtrLit:
            {
                const size_t instSize = prefixSize + sizeof(void*);
                irValue = static_cast<IRConstant*>(_createInst(instSize, keyInst.getFullType(), keyInst.getOp()));
                irValue->value.ptrVal = keyInst.value.ptrVal;
                break;
            }
            case kIROp_VoidLit:
            {
                const size_t instSize = prefixSize;
                irValue = static_cast<IRConstant*>(
                    _createInst(instSize, keyInst.getFullType(), keyInst.getOp()));
                irValue->value.ptrVal = keyInst.value.ptrVal;
                break;
            }
            case kIROp_StringLit:
            {
                const UnownedStringSlice slice = keyInst.getStringSlice();

                const size_t sliceSize = slice.getLength();
                const size_t instSize = prefixSize + offsetof(IRConstant::StringValue, chars) + sliceSize; 

                irValue = static_cast<IRConstant*>(_createInst(instSize, keyInst.getFullType(), keyInst.getOp()));

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
        getSharedBuilder()->getConstantMap().Add(key, irValue);

        addHoistableInst(this, irValue);

        return irValue;
    }

    //

    IRInst* IRBuilder::getBoolValue(bool inValue)
    {
        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.m_op = kIROp_BoolLit;
        keyInst.typeUse.usedValue = getBoolType();
        keyInst.value.intVal = IRIntegerValue(inValue);
        return _findOrEmitConstant(keyInst);
    }

    IRInst* IRBuilder::getIntValue(IRType* type, IRIntegerValue inValue)
    {
        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.m_op = kIROp_IntLit;
        keyInst.typeUse.usedValue = type;
        // Truncate the input value based on `type`.
        switch (type->getOp())
        {
        case kIROp_Int8Type:
            keyInst.value.intVal = static_cast<int8_t>(inValue);
            break;
        case kIROp_Int16Type:
            keyInst.value.intVal = static_cast<int16_t>(inValue);
            break;
        case kIROp_IntType:
            keyInst.value.intVal = static_cast<int32_t>(inValue);
            break;
        case kIROp_UInt8Type:
            keyInst.value.intVal = static_cast<uint8_t>(inValue);
            break;
        case kIROp_UInt16Type:
            keyInst.value.intVal = static_cast<uint16_t>(inValue);
            break;
        case kIROp_BoolType:
        case kIROp_UIntType:
            keyInst.value.intVal = static_cast<uint32_t>(inValue);
            break;
        default:
            keyInst.value.intVal = inValue;
            break;
        }
        return _findOrEmitConstant(keyInst);
    }

    IRInst* IRBuilder::getFloatValue(IRType* type, IRFloatingPointValue inValue)
    {
        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.m_op = kIROp_FloatLit;
        keyInst.typeUse.usedValue = type;
        // Truncate the input value based on `type`.
        switch (type->getOp())
        {
        case kIROp_FloatType:
            keyInst.value.floatVal = static_cast<float>(inValue);
            break;
        case kIROp_HalfType:
            keyInst.value.floatVal = HalfToFloat(FloatToHalf((float)inValue));
            break;
        default:
            keyInst.value.floatVal = inValue;
            break;
        }
        
        return _findOrEmitConstant(keyInst);
    }

    IRStringLit* IRBuilder::getStringValue(const UnownedStringSlice& inSlice)
    {
        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        
        // Mark that this is on the stack...
        IRDecoration stackDecoration;
        memset(&stackDecoration, 0, sizeof(stackDecoration));
        stackDecoration.m_op = kIROp_TransitoryDecoration;
        stackDecoration.insertAtEnd(&keyInst);
            
        keyInst.m_op = kIROp_StringLit;
        keyInst.typeUse.usedValue = getStringType();
        
        IRConstant::StringSliceValue& dstSlice = keyInst.value.transitoryStringVal;
        dstSlice.chars = const_cast<char*>(inSlice.begin());
        dstSlice.numChars = uint32_t(inSlice.getLength());

        return static_cast<IRStringLit*>(_findOrEmitConstant(keyInst));
    }

    IRPtrLit* IRBuilder::getPtrValue(void* value)
    {
        IRType* type = getPtrType(getVoidType());

        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.m_op = kIROp_PtrLit;
        keyInst.typeUse.usedValue = type;
        keyInst.value.ptrVal = value;
        return (IRPtrLit*) _findOrEmitConstant(keyInst);
    }

    IRVoidLit* IRBuilder::getVoidValue()
    {
        IRType* type = getVoidType();

        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.m_op = kIROp_VoidLit;
        keyInst.typeUse.usedValue = type;
        keyInst.value.intVal= 0;
        return (IRVoidLit*)_findOrEmitConstant(keyInst);
    }

    IRInst* IRBuilder::getCapabilityValue(CapabilitySet const& caps)
    {
        IRType* capabilityAtomType = getIntType();
        IRType* capabilitySetType = getCapabilitySetType();

        // Not: Our `CapabilitySet` representation consists of a list
        // of `CapabilityAtom`s, and by default the list is stored
        // "expanded" so that it includes atoms that are transitively
        // implied by one another.
        //
        // For representation in the IR, it is preferable to include
        // as few atoms as possible, so that we don't store anything
        // redundant in, e.g., serialized modules.
        //
        // We thus requqest a list of "compacted" atoms which should
        // be a minimal list of atoms such that they will produce
        // the same `CapabilitySet` when expanded.

        List<CapabilityAtom> compactedAtoms;
        caps.calcCompactedAtoms(compactedAtoms);

        List<IRInst*> args;
        for( auto atom : compactedAtoms )
        {
            args.add(getIntValue(capabilityAtomType, Int(atom)));
        }

        return findOrEmitHoistableInst(
            capabilitySetType, kIROp_CapabilitySet, args.getCount(), args.getBuffer());
    }

    IRInst* IRBuilder::findOrEmitHoistableInst(
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

        auto& memoryArena = getModule()->getMemoryArena();
        void* cursor = memoryArena.getCursor();

        // We are going to create a 'dummy' instruction on the memoryArena
        // which can be used as a key for lookup, so see if we
        // already have an equivalent instruction available to use.
        size_t keySize = sizeof(IRInst) + operandCount * sizeof(IRUse);
        IRInst* inst = (IRInst*) memoryArena.allocateAndZero(keySize);
        
        void* endCursor = memoryArena.getCursor();
        // Mark as 'unused' cos it is unused on release builds. 
        SLANG_UNUSED(endCursor);

        new(inst) IRInst();
#if SLANG_ENABLE_IR_BREAK_ALLOC
        inst->_debugUID = _debugGetAndIncreaseInstCounter();
#endif
        inst->m_op = op;
        inst->typeUse.usedValue = type;
        inst->operandCount = (uint32_t) operandCount;

        // Don't link up as we may free (if we already have this key)
        {
            IRUse* operand = inst->getOperands();
            for (UInt ii = 0; ii < operandListCount; ++ii)
            {
                UInt listOperandCount = listOperandCounts[ii];
                for (UInt jj = 0; jj < listOperandCount; ++jj)
                {
                    operand->usedValue = listOperands[ii][jj];
                    operand++;
                }
            }
        }

        // Find or add the key/inst
        {
            IRInstKey key = { inst };

            // Ideally we would add if not found, else return if was found instead of testing & then adding.
            IRInst** found = getSharedBuilder()->getGlobalValueNumberingMap().TryGetValueOrAdd(key, inst);
            SLANG_ASSERT(endCursor == memoryArena.getCursor());
            // If it's found, just return, and throw away the instruction
            if (found)
            {
                memoryArena.rewindToCursor(cursor);
                return *found;
            }
        }

        // Make the lookup 'inst' instruction into 'proper' instruction. Equivalent to
        // IRInst* inst = createInstImpl<IRInst>(builder, op, type, 0, nullptr, operandListCount, listOperandCounts, listOperands);
        {
            if (type)
            {
                inst->typeUse.usedValue = nullptr;
                inst->typeUse.init(inst, type);
            }

            _maybeSetSourceLoc(inst);

            IRUse*const operands = inst->getOperands();
            for (UInt i = 0; i < operandCount; ++i)
            {
                IRUse& operand = operands[i];
                auto value = operand.usedValue;

                operand.usedValue = nullptr;
                operand.init(inst, value);
            }
        }

        addHoistableInst(this, inst);

        return inst;
    }

    IRInst* IRBuilder::findOrAddInst(
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

        auto& memoryArena = getModule()->getMemoryArena();
        void* cursor = memoryArena.getCursor();

        // We are going to create a 'dummy' instruction on the memoryArena
        // which can be used as a key for lookup, so see if we
        // already have an equivalent instruction available to use.
        size_t keySize = sizeof(IRInst) + operandCount * sizeof(IRUse);
        IRInst* inst = (IRInst*)memoryArena.allocateAndZero(keySize);

        void* endCursor = memoryArena.getCursor();
        // Mark as 'unused' cos it is unused on release builds. 
        SLANG_UNUSED(endCursor);

        new(inst) IRInst();
#if SLANG_ENABLE_IR_BREAK_ALLOC
        inst->_debugUID = _debugGetAndIncreaseInstCounter();
#endif
        inst->m_op = op;
        inst->typeUse.usedValue = type;
        inst->operandCount = (uint32_t)operandCount;

        // Don't link up as we may free (if we already have this key)
        {
            IRUse* operand = inst->getOperands();
            for (UInt ii = 0; ii < operandListCount; ++ii)
            {
                UInt listOperandCount = listOperandCounts[ii];
                for (UInt jj = 0; jj < listOperandCount; ++jj)
                {
                    operand->usedValue = listOperands[ii][jj];
                    operand++;
                }
            }
        }

        // Find or add the key/inst
        {
            IRInstKey key = { inst };

            // Ideally we would add if not found, else return if was found instead of testing & then adding.
            IRInst** found = getSharedBuilder()->getGlobalValueNumberingMap().TryGetValueOrAdd(key, inst);
            SLANG_ASSERT(endCursor == memoryArena.getCursor());
            // If it's found, just return, and throw away the instruction
            if (found)
            {
                memoryArena.rewindToCursor(cursor);
                return *found;
            }
        }

        // Make the lookup 'inst' instruction into 'proper' instruction. Equivalent to
        // IRInst* inst = createInstImpl<IRInst>(builder, op, type, 0, nullptr, operandListCount, listOperandCounts, listOperands);
        {
            if (type)
            {
                inst->typeUse.usedValue = nullptr;
                inst->typeUse.init(inst, type);
            }

            _maybeSetSourceLoc(inst);

            IRUse*const operands = inst->getOperands();
            for (UInt i = 0; i < operandCount; ++i)
            {
                IRUse& operand = operands[i];
                auto value = operand.usedValue;

                operand.usedValue = nullptr;
                operand.init(inst, value);
            }
        }

        addInst(inst);
        return inst;
    }


    IRInst* IRBuilder::findOrEmitHoistableInst(
        IRType*         type,
        IROp            op,
        UInt            operandCount,
        IRInst* const*  operands)
    {
        return findOrEmitHoistableInst(
            type,
            op,
            1,
            &operandCount,
            &operands);
    }

    IRInst* IRBuilder::findOrEmitHoistableInst(
        IRType*         type,
        IROp            op,
        IRInst*         operand,
        UInt            operandCount,
        IRInst* const*  operands)
    {
        UInt counts[] = { 1, operandCount };
        IRInst* const* lists[] = { &operand, operands };

        return findOrEmitHoistableInst(
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

    IRType* IRBuilder::getType(
        IROp            op,
        IRInst*         operand0)
    {
        return getType(op, 1, &operand0);
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

    IRBasicType* IRBuilder::getUIntType()
    {
        return (IRBasicType*)getType(kIROp_UIntType);
    }

    IRBasicType* IRBuilder::getUInt64Type()
    {
        return (IRBasicType*)getType(kIROp_UInt64Type);
    }

    IRBasicType* IRBuilder::getCharType()
    {
        return (IRBasicType*)getType(kIROp_CharType);
    }

    IRStringType* IRBuilder::getStringType()
    {
        return (IRStringType*)getType(kIROp_StringType);
    }

    IRNativeStringType* IRBuilder::getNativeStringType()
    {
        return (IRNativeStringType*)getType(kIROp_NativeStringType);
    }

    IRNativePtrType* IRBuilder::getNativePtrType(IRType* valueType)
    {
        return (IRNativePtrType*)getType(kIROp_NativePtrType, (IRInst*)valueType);
    }


    IRType* IRBuilder::getCapabilitySetType()
    {
        return getType(kIROp_CapabilitySetType);
    }

    IRDynamicType* IRBuilder::getDynamicType() { return (IRDynamicType*)getType(kIROp_DynamicType); }

    IRAssociatedType* IRBuilder::getAssociatedType(ArrayView<IRInterfaceType*> constraintTypes)
    {
        return (IRAssociatedType*)getType(kIROp_AssociatedType,
            constraintTypes.getCount(),
            (IRInst**)constraintTypes.getBuffer());
    }

    IRThisType* IRBuilder::getThisType(IRInterfaceType* interfaceType)
    {
        return (IRThisType*)getType(kIROp_ThisType, interfaceType);
    }

    IRRawPointerType* IRBuilder::getRawPointerType()
    {
        return (IRRawPointerType*)getType(kIROp_RawPointerType);
    }

    IRRTTIPointerType* IRBuilder::getRTTIPointerType(IRInst* rttiPtr)
    {
        return (IRRTTIPointerType*)getType(kIROp_RTTIPointerType, rttiPtr);
    }

    IRRTTIType* IRBuilder::getRTTIType()
    {
        return (IRRTTIType*)getType(kIROp_RTTIType);
    }

    IRRTTIHandleType* IRBuilder::getRTTIHandleType()
    {
        return (IRRTTIHandleType*)getType(kIROp_RTTIHandleType);
    }

    IRAnyValueType* IRBuilder::getAnyValueType(IRIntegerValue size)
    {
        return (IRAnyValueType*)getType(kIROp_AnyValueType,
            getIntValue(getUIntType(), size));
    }

    IRAnyValueType* IRBuilder::getAnyValueType(IRInst* size)
    {
        return (IRAnyValueType*)getType(kIROp_AnyValueType, size);
    }

    IRTupleType* IRBuilder::getTupleType(UInt count, IRType* const* types)
    {
        return (IRTupleType*)getType(kIROp_TupleType, count, (IRInst*const*)types);
    }

    IRTupleType* IRBuilder::getTupleType(IRType* type0, IRType* type1)
    {
        IRType* operands[] = { type0, type1 };
        return getTupleType(2, operands);
    }

    IRTupleType* IRBuilder::getTupleType(IRType* type0, IRType* type1, IRType* type2)
    {
        IRType* operands[] = { type0, type1, type2 };
        return getTupleType(3, operands);
    }

    IRTupleType* IRBuilder::getTupleType(IRType* type0, IRType* type1, IRType* type2, IRType* type3)
    {
        IRType* operands[] = { type0, type1, type2, type3 };
        return getTupleType(SLANG_COUNT_OF(operands), operands);
    }

    IRResultType* IRBuilder::getResultType(IRType* valueType, IRType* errorType)
    {
        IRInst* operands[] = {valueType, errorType};
        return (IRResultType*)getType(kIROp_ResultType, 2, operands);
    }

    IROptionalType* IRBuilder::getOptionalType(IRType* valueType)
    {
        return (IROptionalType*)getType(kIROp_OptionalType, valueType);
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

    IRSPIRVLiteralType* IRBuilder::getSPIRVLiteralType(IRType* type)
    {
        IRInst* operands[] = { type };
        return (IRSPIRVLiteralType*)getType(kIROp_SPIRVLiteralType, 1, operands);
    }

    IRPtrTypeBase* IRBuilder::getPtrType(IROp op, IRType* valueType)
    {
        IRInst* operands[] = { valueType };
        return (IRPtrTypeBase*) getType(
            op,
            1,
            operands);
    }

    IRPtrType* IRBuilder::getPtrType(IROp op, IRType* valueType, IRIntegerValue addressSpace)
    {
        IRInst* operands[] = {valueType, getIntValue(getIntType(), addressSpace)};
        return (IRPtrType*)getType(op, 2, operands);
    }

    IRComPtrType* IRBuilder::getComPtrType(IRType* valueType)
    {
        return (IRComPtrType*)getType(kIROp_ComPtrType, valueType);
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

    IRDifferentialPairType* IRBuilder::getDifferentialPairType(
        IRType* valueType,
        IRWitnessTable* witnessTable)
    {
        IRInst* operands[] = { valueType, witnessTable };
        return (IRDifferentialPairType*)getType(
            kIROp_DifferentialPairType,
            sizeof(operands) / sizeof(operands[0]),
            operands);
    }

    IRFuncType* IRBuilder::getFuncType(
        UInt            paramCount,
        IRType* const*  paramTypes,
        IRType*         resultType)
    {
        return (IRFuncType*) findOrEmitHoistableInst(
            nullptr,
            kIROp_FuncType,
            resultType,
            paramCount,
            (IRInst* const*) paramTypes);
    }

    IRFuncType* IRBuilder::getFuncType(
        UInt paramCount, IRType* const* paramTypes, IRType* resultType, IRAttr* attribute)
    {
        UInt counts[3] = {1, paramCount, 1};
        IRInst** lists[3] = {(IRInst**)&resultType, (IRInst**)paramTypes, (IRInst**)&attribute};
        return (IRFuncType*)findOrEmitHoistableInst(nullptr, kIROp_FuncType, 3, counts, lists);
    }

    IRWitnessTableType* IRBuilder::getWitnessTableType(
        IRType* baseType)
    {
        return (IRWitnessTableType*)findOrEmitHoistableInst(
            nullptr,
            kIROp_WitnessTableType,
            1,
            (IRInst* const*)&baseType);
    }

    IRWitnessTableIDType* IRBuilder::getWitnessTableIDType(
        IRType* baseType)
    {
        return (IRWitnessTableIDType*)findOrEmitHoistableInst(
            nullptr,
            kIROp_WitnessTableIDType,
            1,
            (IRInst* const*)&baseType);
    }

    IRConstantBufferType* IRBuilder::getConstantBufferType(IRType* elementType)
    {
        IRInst* operands[] = { elementType };
        return (IRConstantBufferType*) getType(
            kIROp_ConstantBufferType,
            1,
            operands);
    }

    IRConstExprRate* IRBuilder::getConstExprRate()
    {
        return (IRConstExprRate*)getType(kIROp_ConstExprRate);
    }

    IRGroupSharedRate* IRBuilder::getGroupSharedRate()
    {
        return (IRGroupSharedRate*)getType(kIROp_GroupSharedRate);
    }
    IRActualGlobalRate* IRBuilder::getActualGlobalRate()
    {
        return (IRActualGlobalRate*)getType(kIROp_ActualGlobalRate);
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

    IRType* IRBuilder::getTaggedUnionType(
        UInt            caseCount,
        IRType* const*  caseTypes)
    {
        return (IRType*) findOrEmitHoistableInst(
            getTypeKind(),
            kIROp_TaggedUnionType,
            caseCount,
            (IRInst* const*) caseTypes);
    }

    IRType* IRBuilder::getBindExistentialsType(
        IRInst*         baseType,
        UInt            slotArgCount,
        IRInst* const*  slotArgs)
    {
        if(slotArgCount == 0)
            return (IRType*) baseType;

        // If we are trying to bind an interface type, then
        // we will go ahead and simplify the instruction
        // away impmediately.
        // 
        if(as<IRInterfaceType>(baseType))
        {
            if(slotArgCount >= 2)
            {
                // We are being asked to emit `BindExistentials(someInterface, someConcreteType, ...)`
                // so we just want to return `ExistentialBox<someConcreteType>`.
                //
                auto concreteType = (IRType*) slotArgs[0];
                auto witnessTable = slotArgs[1];
                auto ptrType = getBoundInterfaceType((IRType*) baseType, concreteType, witnessTable);
                return ptrType;
            }
        }

        return (IRType*) findOrEmitHoistableInst(
            getTypeKind(),
            kIROp_BindExistentialsType,
            baseType,
            slotArgCount,
            (IRInst* const*) slotArgs);
    }

    IRType* IRBuilder::getBindExistentialsType(
        IRInst*         baseType,
        UInt            slotArgCount,
        IRUse const*    slotArgUses)
    {
        if(slotArgCount == 0)
            return (IRType*) baseType;

        List<IRInst*> slotArgs;
        for( UInt ii = 0; ii < slotArgCount; ++ii )
        {
            slotArgs.add(slotArgUses[ii].get());
        }
        return getBindExistentialsType(
            baseType,
            slotArgCount,
            slotArgs.getBuffer());
    }

    IRType* IRBuilder::getBoundInterfaceType(
        IRType* interfaceType,
        IRType* concreteType,
        IRInst* witnessTable)
    {
        // Don't wrap an existential box if concreteType is __Dynamic.
        if (as<IRDynamicType>(concreteType))
            return interfaceType;

        IRInst* operands[] = {interfaceType, concreteType, witnessTable};
        return getType(kIROp_BoundInterfaceType, SLANG_COUNT_OF(operands), operands);
    }

    IRType* IRBuilder::getPseudoPtrType(
        IRType* concreteType)
    {
        IRInst* operands[] = {concreteType};
        return getType(kIROp_PseudoPtrType, SLANG_COUNT_OF(operands), operands);
    }

    IRType* IRBuilder::getConjunctionType(
        UInt            typeCount,
        IRType* const*  types)
    {
        return getType(kIROp_ConjunctionType, typeCount, (IRInst* const*)types);
    }

    IRType* IRBuilder::getAttributedType(
        IRType*         baseType,
        UInt            attributeCount,
        IRAttr* const*  attributes)
    {
        List<IRInst*> operands;
        operands.add(baseType);
        for(UInt i = 0; i < attributeCount; ++i)
            operands.add(attributes[i]);
        return getType(kIROp_AttributedType, operands.getCount(), operands.getBuffer());
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

    IRInst* IRBuilder::emitGetValueFromBoundInterface(IRType* type, IRInst* boundInterfaceValue)
    {
        auto inst =
            createInst<IRInst>(this, kIROp_GetValueFromBoundInterface, type, 1, &boundInterfaceValue);
        addInst(inst);
        return inst;
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

    IRInst* IRBuilder::emitReinterpret(IRInst* type, IRInst* value)
    {
        return emitIntrinsicInst((IRType*)type, kIROp_Reinterpret, 1, &value);
    }

    IRLiveRangeStart* IRBuilder::emitLiveRangeStart(IRInst* referenced)
    {
        // This instruction doesn't produce any result, 
        // so we make it's type void.
        auto inst = createInst<IRLiveRangeStart>(
            this,
            kIROp_LiveRangeStart,
            getVoidType(),
            referenced);
        
        addInst(inst);

        return inst;
    }

    IRLiveRangeEnd* IRBuilder::emitLiveRangeEnd(IRInst* referenced)
    {
        // This instruction doesn't produce any result, 
        // so we make it's type void.
        auto inst = createInst<IRLiveRangeEnd>(
            this,
            kIROp_LiveRangeEnd,
            getVoidType(),
            referenced);

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
        auto type = getWitnessTableType(existentialValue->getDataType());
        auto inst = createInst<IRInst>(
            this,
            kIROp_ExtractExistentialWitnessTable,
            type,
            1,
            &existentialValue);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitJVPDifferentiateInst(IRType* type, IRInst* baseFn)
    {
        auto inst = createInst<IRJVPDifferentiate>(
            this,
            kIROp_JVPDifferentiate,
            type,
            baseFn);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitMakeDifferentialPair(IRType* type, IRInst* primal, IRInst* differential)
    {
        IRInst* args[] = {primal, differential};
        auto inst = createInstWithTrailingArgs<IRMakeDifferentialPair>(
            this, kIROp_MakeDifferentialPair, type, 2, args);
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
        // TODO: if somebody tries to declare a struct that inherits
        // an interface conformance from a base type, then we hit
        // this assert. The problem should be fixed higher up in
        // the emit logic, but this is a reasonably early place
        // to catch it.
        //
        SLANG_ASSERT(witnessTableVal->getOp() != kIROp_StructKey);

        auto inst = createInst<IRLookupWitnessMethod>(
            this,
            kIROp_lookup_interface_method,
            type,
            witnessTableVal,
            interfaceMethodVal);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitGetSequentialIDInst(IRInst* rttiObj)
    {
        auto inst = createInst<IRAlloca>(this, kIROp_GetSequentialID, getUIntType(), rttiObj);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitAlloca(IRInst* type, IRInst* rttiObjPtr)
    {
        auto inst = createInst<IRAlloca>(
            this,
            kIROp_Alloca,
            (IRType*)type,
            rttiObjPtr);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitPackAnyValue(IRType* type, IRInst* value)
    {
        auto inst = createInst<IRPackAnyValue>(
            this,
            kIROp_PackAnyValue,
            type,
            value);

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitUnpackAnyValue(IRType* type, IRInst* value)
    {
        auto inst = createInst<IRPackAnyValue>(
            this,
            kIROp_UnpackAnyValue,
            type,
            value);

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

    IRInst* IRBuilder::emitTryCallInst(
        IRType* type,
        IRBlock* successBlock,
        IRBlock* failureBlock,
        IRInst* func,
        UInt argCount,
        IRInst* const* args)
    {
        IRInst* fixedArgs[] = {successBlock, failureBlock, func};
        auto inst = createInstWithTrailingArgs<IRTryCall>(
            this, kIROp_TryCall, type, 3, fixedArgs, argCount, args);
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

    IRInst* IRBuilder::emitMakeUInt64(IRInst* low, IRInst* high)
    {
        IRInst* args[2] = {low, high};
        return emitIntrinsicInst(getUInt64Type(), kIROp_makeUInt64, 2, args);
    }

    IRInst* IRBuilder::emitMakeRTTIObject(IRInst* typeInst)
    {
        auto inst = createInst<IRRTTIObject>(
            this,
            kIROp_RTTIObject,
            getRTTIType(),
            typeInst);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitMakeTuple(IRType* type, UInt count, IRInst* const* args)
    {
        return emitIntrinsicInst(type, kIROp_MakeTuple, count, args);
    }

    IRInst* IRBuilder::emitMakeTuple(UInt count, IRInst* const* args)
    {
        List<IRType*> types;
        for(UInt i = 0; i < count; ++i)
            types.add(args[i]->getFullType());

        auto type = getTupleType(types);
        return emitMakeTuple(type, count, args);
    }

    IRInst* IRBuilder::emitMakeString(IRInst* nativeStr)
    {
        return emitIntrinsicInst(getStringType(), kIROp_makeString, 1, &nativeStr);
    }

    IRInst* IRBuilder::emitGetNativeString(IRInst* str)
    {
        return emitIntrinsicInst(getNativeStringType(), kIROp_getNativeStr, 1, &str);
    }

    IRInst* IRBuilder::emitGetTupleElement(IRType* type, IRInst* tuple, UInt element)
    {
        // As a quick simplification/optimization, if the user requests
        // `getTupleElement(makeTuple(a_0, a_1, ... a_N), i)` then we should
        // just return `a_i`, provided that the index is properly in range.
        //
        if( auto makeTuple = as<IRMakeTuple>(tuple) )
        {
            if( element < makeTuple->getOperandCount() )
            {
                return makeTuple->getOperand(element);
            }
        }

        IRInst* args[] = { tuple, getIntValue(getIntType(), element) };
        return emitIntrinsicInst(type, kIROp_GetTupleElement, 2, args);
    }

    IRInst* IRBuilder::emitMakeResultError(IRType* resultType, IRInst* errorVal)
    {
        return emitIntrinsicInst(resultType, kIROp_MakeResultError, 1, &errorVal);
    }

    IRInst* IRBuilder::emitMakeResultValue(IRType* resultType, IRInst* value)
    {
        return emitIntrinsicInst(resultType, kIROp_MakeResultValue, 1, &value);
    }

    IRInst* IRBuilder::emitIsResultError(IRInst* result)
    {
        return emitIntrinsicInst(getBoolType(), kIROp_IsResultError, 1, &result);
    }

    IRInst* IRBuilder::emitGetResultError(IRInst* result)
    {
        SLANG_ASSERT(result->getDataType());
        return emitIntrinsicInst(
            cast<IRResultType>(result->getDataType())->getErrorType(),
            kIROp_GetResultError,
            1,
            &result);
    }

    IRInst* IRBuilder::emitGetResultValue(IRInst* result)
    {
        SLANG_ASSERT(result->getDataType());
        return emitIntrinsicInst(
            cast<IRResultType>(result->getDataType())->getValueType(),
            kIROp_GetResultValue,
            1,
            &result);
    }

    IRInst* IRBuilder::emitOptionalHasValue(IRInst* optValue)
    {
        return emitIntrinsicInst(
            getBoolType(),
            kIROp_OptionalHasValue,
            1,
            &optValue);
    }

    IRInst* IRBuilder::emitGetOptionalValue(IRInst* optValue)
    {
        return emitIntrinsicInst(
            cast<IROptionalType>(optValue->getDataType())->getValueType(),
            kIROp_GetOptionalValue,
            1,
            &optValue);
    }

    IRInst* IRBuilder::emitMakeOptionalValue(IRInst* optType, IRInst* value)
    {
        return emitIntrinsicInst(
            (IRType*)optType,
            kIROp_MakeOptionalValue,
            1,
            &value);
    }

    IRInst* IRBuilder::emitMakeOptionalNone(IRInst* optType, IRInst* defaultValue)
    {
        return emitIntrinsicInst(
            (IRType*)optType,
            kIROp_MakeOptionalNone,
            1,
            &defaultValue);
    }

    IRInst* IRBuilder::emitMakeVector(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args)
    {
        return emitIntrinsicInst(type, kIROp_makeVector, argCount, args);
    }

    IRInst* IRBuilder::emitMakeMatrix(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args)
    {
        return emitIntrinsicInst(type, kIROp_MakeMatrix, argCount, args);
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

    IRInst* IRBuilder::emitMakeExistentialWithRTTI(
        IRType* type,
        IRInst* value,
        IRInst* witnessTable,
        IRInst* rtti)
    {
        IRInst* args[] = { value, witnessTable, rtti };
        return emitIntrinsicInst(type, kIROp_MakeExistentialWithRTTI, SLANG_COUNT_OF(args), args);
    }

    IRInst* IRBuilder::emitWrapExistential(
        IRType*         type,
        IRInst*         value,
        UInt            slotArgCount,
        IRInst* const*  slotArgs)
    {
        if(slotArgCount == 0)
            return value;

        // If we are wrapping a single concrete value into
        // an interface type, then this is really a `makeExistential`
        //
        // TODO: We may want to check for a `specialize` of a generic interface as well.
        //
        if(as<IRInterfaceType>(type))
        {
            if(slotArgCount >= 2)
            {
                // We are being asked to emit `wrapExistential(value, concreteType, witnessTable, ...) : someInterface`
                //
                // We also know that a concrete value being wrapped will always be an existential box,
                // so we expect that `value : BindInterface<I, C>` for some concrete `C`.
                //
                // We want to emit `makeExistential(getValueFromBoundInterface(value) : C, witnessTable)`.
                //
                auto concreteType = (IRType*)(slotArgs[0]);
                auto witnessTable = slotArgs[1];
                if (slotArgs[0]->getOp() == kIROp_DynamicType)
                    return value;
                auto deref = emitGetValueFromBoundInterface(concreteType, value);
                return emitMakeExistential(type, deref, witnessTable);
            }
        }

        IRInst* fixedArgs[] = {value};
        auto inst = createInstImpl<IRInst>(
            this,
            kIROp_WrapExistential,
            type,
            SLANG_COUNT_OF(fixedArgs),
            fixedArgs,
            slotArgCount,
            slotArgs);
        addInst(inst);
        return inst;
    }

    RefPtr<IRModule> IRModule::create(Session* session)
    {
        RefPtr<IRModule> module = new IRModule(session);

        auto moduleInst = module->_allocateInst<IRModuleInst>(kIROp_Module, 0);

        module->m_moduleInst = moduleInst;
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
        auto defaultInsertLoc = builder->getInsertLoc();
        auto defaultParent = defaultInsertLoc.getParent();
        auto parent = defaultParent;
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
        if (parent == defaultParent)
        {
            value->insertAt(defaultInsertLoc);
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
        _maybeSetSourceLoc(rsFunc);
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
        _maybeSetSourceLoc(globalVar);
        addGlobalValue(this, globalVar);
        return globalVar;
    }

    IRGlobalParam* IRBuilder::createGlobalParam(
        IRType* valueType)
    {
        IRGlobalParam* inst = createInst<IRGlobalParam>(
            this,
            kIROp_GlobalParam,
            valueType);
        _maybeSetSourceLoc(inst);
        addGlobalValue(this, inst);
        return inst;
    }

    IRWitnessTable* IRBuilder::createWitnessTable(IRType* baseType, IRType* subType)
    {
        IRWitnessTable* witnessTable = createInst<IRWitnessTable>(
            this,
            kIROp_WitnessTable,
            getWitnessTableType(baseType),
            subType);
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

    IRInterfaceRequirementEntry* IRBuilder::createInterfaceRequirementEntry(
        IRInst* requirementKey,
        IRInst* requirementVal)
    {
        IRInterfaceRequirementEntry* entry = createInst<IRInterfaceRequirementEntry>(
            this,
            kIROp_InterfaceRequirementEntry,
            nullptr,
            requirementKey,
            requirementVal);
        addGlobalValue(this, entry);
        return entry;
    }

    IRStructType* IRBuilder::createStructType()
    {
        IRStructType* structType = createInst<IRStructType>(
            this,
            kIROp_StructType,
            getTypeKind());
        addGlobalValue(this, structType);
        return structType;
    }

    IRClassType* IRBuilder::createClassType()
    {
        IRClassType* classType = createInst<IRClassType>(
            this,
            kIROp_ClassType,
            getTypeKind());
        addGlobalValue(this, classType);
        return classType;
    }

    IRInterfaceType* IRBuilder::createInterfaceType(UInt operandCount, IRInst* const* operands)
    {
        IRInterfaceType* interfaceType = createInst<IRInterfaceType>(
            this,
            kIROp_InterfaceType,
            getTypeKind(),
            operandCount,
            operands);
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
        IRType*         aggType,
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

        if (aggType)
        {
            field->insertAtEnd(aggType);
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

    IRParam* IRBuilder::emitParamAtHead(
        IRType* type)
    {
        auto param = createParam(type);
        if (auto bb = getBlock())
        {
            bb->insertParamAtHead(param);
        }
        return param;
    }

    IRInst* IRBuilder::emitAllocObj(IRType* type)
    {
        return emitIntrinsicInst(type, kIROp_AllocObj, 0, nullptr);
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
        IRType* type,
        IRInst* ptr)
    {
        auto inst = createInst<IRLoad>(
            this,
            kIROp_Load,
            type,
            ptr);

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

        IRType* valueType = tryGetPointedToType(this, ptr->getFullType());
        SLANG_ASSERT(valueType);

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

        return emitLoad(valueType, ptr);
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

    IRInst* IRBuilder::emitImageLoad(IRType* type, IRInst* image, IRInst* coord)
    {
        auto inst = createInst<IRImageLoad>(this, kIROp_ImageLoad, type, image, coord);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitImageStore(IRType* type, IRInst* image, IRInst* coord, IRInst* value)
    {
        IRInst* args[] = {image, coord, value};
        auto inst = createInst<IRImageStore>(this, kIROp_ImageStore, type, 3, args);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitIsType(IRInst* value, IRInst* witness, IRInst* typeOperand, IRInst* targetWitness)
    {
        IRInst* args[] = { value, witness, typeOperand, targetWitness };
        auto inst = createInst<IRIsType>(this, kIROp_IsType, getBoolType(), SLANG_COUNT_OF(args), args);
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

    IRInst* IRBuilder::emitGetAddress(
        IRType* type,
        IRInst* value)
    {
        auto inst = createInst<IRGetAddress>(
            this,
            kIROp_getAddr,
            type,
            value);

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
        auto inst = createInst<IRReturn>(
            this,
            kIROp_Return,
            nullptr,
            val);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitReturn()
    {
        auto voidVal = getVoidValue();
        auto inst = createInst<IRReturn>(this, kIROp_Return, nullptr, voidVal);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitThrow(IRInst* val)
    {
        auto inst = createInst<IRThrow>(this, kIROp_Throw, nullptr, val);
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

    IRInst* IRBuilder::emitBranch(IRBlock* block, Int argCount, IRInst* const* args)
    {
        List<IRInst*> argList;
        argList.add(block);
        for (Int i = 0; i < argCount; ++i)
            argList.add(args[i]);
        auto inst =
            createInst<IRUnconditionalBranch>(this, kIROp_unconditionalBranch, nullptr, argList.getCount(), argList.getBuffer());
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

    IRInst* IRBuilder::emitIfElseWithBlocks(
        IRInst* val, IRBlock*& outTrueBlock, IRBlock*& outFalseBlock, IRBlock*& outAfterBlock)
    {
        outTrueBlock = createBlock();
        outAfterBlock = createBlock();
        outFalseBlock = createBlock();
        auto f = getFunc();
        SLANG_ASSERT(f);
        if (f)
        {
            f->addBlock(outTrueBlock);
            f->addBlock(outAfterBlock);
            f->addBlock(outFalseBlock);
        }
        auto result = emitIfElse(val, outTrueBlock, outFalseBlock, outAfterBlock);
        setInsertInto(outTrueBlock);
        return result;
    }

    IRInst* IRBuilder::emitIf(
        IRInst*    val,
        IRBlock*    trueBlock,
        IRBlock*    afterBlock)
    {
        return emitIfElse(val, trueBlock, afterBlock, afterBlock);
    }

    IRInst* IRBuilder::emitIfWithBlocks(
        IRInst* val, IRBlock*& outTrueBlock, IRBlock*& outAfterBlock)
    {
        outTrueBlock = createBlock();
        outAfterBlock = createBlock();
        auto result = emitIf(val, outTrueBlock, outAfterBlock);
        insertBlock(outTrueBlock);
        insertBlock(outAfterBlock);
        setInsertInto(outTrueBlock);
        return result;
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

    IRGlobalGenericParam* IRBuilder::emitGlobalGenericParam(
        IRType* type)
    {
        IRGlobalGenericParam* irGenericParam = createInst<IRGlobalGenericParam>(
            this,
            kIROp_GlobalGenericParam,
            type);
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

    IRDecoration* IRBuilder::addBindExistentialSlotsDecoration(
        IRInst*         value,
        UInt            argCount,
        IRInst* const*  args)
    {
        auto decoration = createInstWithTrailingArgs<IRDecoration>(
            this,
            kIROp_BindExistentialSlotsDecoration,
            getVoidType(),
            0,
            nullptr,
            argCount,
            args);

        decoration->insertAtStart(value);

        return decoration;
    }

    IRInst* IRBuilder::emitExtractTaggedUnionTag(
        IRInst* val)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_ExtractTaggedUnionTag,
            getBasicType(BaseType::UInt),
            val);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitExtractTaggedUnionPayload(
        IRType* type,
        IRInst* val,
        IRInst* tag)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_ExtractTaggedUnionPayload,
            type,
            val,
            tag);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitBitCast(
        IRType* type,
        IRInst* val)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_BitCast,
            type,
            val);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitCastPtrToBool(IRInst* val)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_CastPtrToBool,
            getBoolType(),
            val);
        addInst(inst);
        return inst;
    }

    IRGlobalConstant* IRBuilder::emitGlobalConstant(
        IRType* type)
    {
        auto inst = createInst<IRGlobalConstant>(
            this,
            kIROp_GlobalConstant,
            type);
        addInst(inst);
        return inst;
    }

    IRGlobalConstant* IRBuilder::emitGlobalConstant(
        IRType* type,
        IRInst* val)
    {
        auto inst = createInst<IRGlobalConstant>(
            this,
            kIROp_GlobalConstant,
            type,
            val);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitWaveMaskBallot(IRType* type, IRInst* mask, IRInst* condition)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_WaveMaskBallot,
            type,
            mask,
            condition);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitWaveMaskMatch(IRType* type, IRInst* mask, IRInst* value)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_WaveMaskMatch,
            type,
            mask,
            value);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitBitAnd(IRType* type, IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_BitAnd,
            type,
            left,
            right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitBitOr(IRType* type, IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_BitOr,
            type,
            left,
            right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitBitNot(IRType* type, IRInst* value)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_BitNot,
            type,
            value);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitAdd(IRType* type, IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_Add,
            type,
            left,
            right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitSub(IRType* type, IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_Sub,
            type,
            left,
            right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitEql(IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(this, kIROp_Eql, getBoolType(), left, right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitNeq(IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(this, kIROp_Neq, getBoolType(), left, right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitLess(IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(this, kIROp_Less, getBoolType(), left, right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitMul(IRType* type, IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_Mul,
            type,
            left,
            right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitDiv(IRType* type, IRInst* numerator, IRInst* denominator)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_Div,
            type,
            numerator,
            denominator);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitShr(IRType* type, IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(this, kIROp_Rsh, type, left, right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitShl(IRType* type, IRInst* left, IRInst* right)
    {
        auto inst = createInst<IRInst>(this, kIROp_Lsh, type, left, right);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitGetNativePtr(IRInst* value)
    {
        auto valueType = value->getDataType();
        SLANG_RELEASE_ASSERT(valueType);
        switch (valueType->getOp())
        {
        case kIROp_InterfaceType:
            return emitIntrinsicInst(
                getNativePtrType((IRType*)valueType), kIROp_GetNativePtr, 1, &value);
            break;
        case kIROp_ComPtrType:
            return emitIntrinsicInst(
                getNativePtrType((IRType*)valueType->getOperand(0)), kIROp_GetNativePtr, 1, &value);
            break;
        default:
            SLANG_UNEXPECTED("invalid operand type for `getNativePtr`.");
            UNREACHABLE_RETURN(nullptr);
        }
    }

    IRInst* IRBuilder::emitManagedPtrAttach(IRInst* managedPtrVar, IRInst* value)
    {
        IRInst* args[] = { managedPtrVar, value };
        return emitIntrinsicInst(getVoidType(), kIROp_ManagedPtrAttach, 2, args);
    }

    IRInst* IRBuilder::emitManagedPtrDetach(IRType* type, IRInst* managedPtrVal)
    {
        return emitIntrinsicInst(type, kIROp_ManagedPtrDetach, 1, &managedPtrVal);
    }

    IRInst* IRBuilder::emitGetManagedPtrWriteRef(IRInst* ptrToManagedPtr)
    {
        auto type = ptrToManagedPtr->getDataType();
        auto ptrType = as<IRPtrTypeBase>(type);
        SLANG_RELEASE_ASSERT(ptrType);
        auto managedPtrType = ptrType->getValueType();
        switch (managedPtrType->getOp())
        {
        case kIROp_InterfaceType:
        case kIROp_ComPtrType:
            return emitIntrinsicInst(
                getPtrType(getNativePtrType((IRType*)managedPtrType->getOperand(0))), kIROp_GetManagedPtrWriteRef, 1, &ptrToManagedPtr);
            break;
        default:
            SLANG_UNEXPECTED("invalid operand type for `getNativePtr`.");
            UNREACHABLE_RETURN(nullptr);
        }
    }

    IRInst* IRBuilder::emitGpuForeach(List<IRInst*> args)
    {
        auto inst = createInst<IRInst>(
            this,
            kIROp_GpuForeach,
            getVoidType(),
            args.getCount(),
            args.getBuffer());
        addInst(inst);
        return inst;
    }

    //
    // Decorations
    //

    IRDecoration* IRBuilder::addDecoration(IRInst* value, IROp op, IRInst* const* operands, Int operandCount)
    {
        // If it's a simple (ie stateless) decoration, don't add it again.
        if (operandCount == 0 && isSimpleDecoration(op))
        {
            auto decoration = value->findDecorationImpl(op);
            if (decoration)
            {
                return decoration;
            }
        }

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
        auto ptrConst = getPtrValue(decl);
        addDecoration(inst, kIROp_HighLevelDeclDecoration, ptrConst);
    }

    void IRBuilder::addLayoutDecoration(IRInst* value, IRLayout* layout)
    {
        addDecoration(value, kIROp_LayoutDecoration, layout);
    }

    IRTypeSizeAttr* IRBuilder::getTypeSizeAttr(
        LayoutResourceKind kind,
        LayoutSize size)
    {
        auto kindInst = getIntValue(getIntType(), IRIntegerValue(kind));
        auto sizeInst = getIntValue(getIntType(), IRIntegerValue(size.raw));

        IRInst* operands[] = { kindInst, sizeInst };

        return cast<IRTypeSizeAttr>(findOrEmitHoistableInst(
            getVoidType(),
            kIROp_TypeSizeAttr,
            SLANG_COUNT_OF(operands),
            operands));
    }

    IRVarOffsetAttr* IRBuilder::getVarOffsetAttr(
        LayoutResourceKind  kind,
        UInt                offset,
        UInt                space)
    {
        IRInst* operands[3];
        UInt operandCount = 0;

        auto kindInst = getIntValue(getIntType(), IRIntegerValue(kind));
        operands[operandCount++] = kindInst;

        auto offsetInst = getIntValue(getIntType(), IRIntegerValue(offset));
        operands[operandCount++] = offsetInst;

        if(space)
        {
            auto spaceInst = getIntValue(getIntType(), IRIntegerValue(space));
            operands[operandCount++] = spaceInst;
        }

        return cast<IRVarOffsetAttr>(findOrEmitHoistableInst(
            getVoidType(),
            kIROp_VarOffsetAttr,
            operandCount,
            operands));
    }

    IRPendingLayoutAttr* IRBuilder::getPendingLayoutAttr(
        IRLayout* pendingLayout)
    {
        IRInst* operands[] = { pendingLayout };

        return cast<IRPendingLayoutAttr>(findOrEmitHoistableInst(
            getVoidType(),
            kIROp_PendingLayoutAttr,
            SLANG_COUNT_OF(operands),
            operands));
    }

    IRStructFieldLayoutAttr* IRBuilder::getFieldLayoutAttr(
        IRInst*         key,
        IRVarLayout*    layout)
    {
        IRInst* operands[] = { key, layout };

        return cast<IRStructFieldLayoutAttr>(findOrEmitHoistableInst(
            getVoidType(),
            kIROp_StructFieldLayoutAttr,
            SLANG_COUNT_OF(operands),
            operands));
    }

    IRCaseTypeLayoutAttr* IRBuilder::getCaseTypeLayoutAttr(
        IRTypeLayout*   layout)
    {
        IRInst* operands[] = { layout };

        return cast<IRCaseTypeLayoutAttr>(findOrEmitHoistableInst(
            getVoidType(),
            kIROp_CaseTypeLayoutAttr,
            SLANG_COUNT_OF(operands),
            operands));
    }

    IRSemanticAttr* IRBuilder::getSemanticAttr(
        IROp            op,
        String const&   name,
        UInt            index)
    {
        auto nameInst = getStringValue(name.getUnownedSlice());
        auto indexInst = getIntValue(getIntType(), index);

        IRInst* operands[] = { nameInst, indexInst };

        return cast<IRSemanticAttr>(findOrEmitHoistableInst(
            getVoidType(),
            op,
            SLANG_COUNT_OF(operands),
            operands));
    }

    IRStageAttr* IRBuilder::getStageAttr(Stage stage)
    {
        auto stageInst = getIntValue(getIntType(), IRIntegerValue(stage));
        IRInst* operands[] = { stageInst };
        return cast<IRStageAttr>(findOrEmitHoistableInst(
            getVoidType(),
            kIROp_StageAttr,
            SLANG_COUNT_OF(operands),
            operands));
    }

    IRAttr* IRBuilder::getAttr(IROp op, UInt operandCount, IRInst* const* operands)
    {
        return cast<IRAttr>(findOrEmitHoistableInst(
            getVoidType(),
            op,
            operandCount,
            operands));
    }



    IRTypeLayout* IRBuilder::getTypeLayout(IROp op, List<IRInst*> const& operands)
    {
        return cast<IRTypeLayout>(findOrEmitHoistableInst(
            getVoidType(),
            op,
            operands.getCount(),
            operands.getBuffer()));
    }

    IRVarLayout* IRBuilder::getVarLayout(List<IRInst*> const& operands)
    {
        return cast<IRVarLayout>(findOrEmitHoistableInst(
            getVoidType(),
            kIROp_VarLayout,
            operands.getCount(),
            operands.getBuffer()));
    }

    IREntryPointLayout* IRBuilder::getEntryPointLayout(
        IRVarLayout* paramsLayout,
        IRVarLayout* resultLayout)
    {
        IRInst* operands[] = { paramsLayout, resultLayout };

        return cast<IREntryPointLayout>(findOrEmitHoistableInst(
            getVoidType(),
            kIROp_EntryPointLayout,
            SLANG_COUNT_OF(operands),
            operands));
    }

    //

    struct IRDumpContext
    {
        StringBuilder*  builder = nullptr;
        int             indent  = 0;

        IRDumpOptions   options;
        SourceManager*  sourceManager;
        PathInfo        lastPathInfo = PathInfo::makeUnknown();
        
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
        context->builder->append(val);
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
        if(type->getOp() == kIROp_VoidType)
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

        // Allow an empty name
        // Special case a name that is the empty string, just in case.
        if(name.getLength() == 0)
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

    static void dumpDebugID(IRDumpContext* context, IRInst* inst)
    {
#if SLANG_ENABLE_IR_BREAK_ALLOC
        if (context->options.flags & IRDumpOptions::Flag::DumpDebugIds)
        {
            dump(context, "[#");
            dump(context, String(inst->_debugUID));
            dump(context, "]");
        }
#else
        SLANG_UNUSED(context);
        SLANG_UNUSED(inst);
#endif
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
        dumpDebugID(context, inst);
    }
    
    static void dumpEncodeString(
        IRDumpContext*  context, 
        const UnownedStringSlice& slice)
    {
        auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Slang);
        StringBuilder& builder = *context->builder;
        StringEscapeUtil::appendQuoted(handler, slice, builder);
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
        if(context->options.mode == IRDumpOptions::Mode::Detailed)
            return false;

        if(as<IRConstant>(inst))
            return true;

        // We are going to have a general rule that
        // a type should be folded into its use site,
        // which improves output in most cases, but
        // we would like to not apply that rule to
        // "nominal" types like `struct`s.
        //
        switch( inst->getOp() )
        {
        case kIROp_StructType:
        case kIROp_InterfaceType:
            return false;

        default:
            break;
        }

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
        auto opInfo = getIROpInfo(code->getOp());

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

    static void dumpInstOperandList(
        IRDumpContext* context,
        IRInst* inst)
    {
        UInt argCount = inst->getOperandCount();

        if (argCount == 0)
            return;

        UInt ii = 0;

        // Special case: make printing of `call` a bit
        // nicer to look at
        if (inst->getOp() == kIROp_Call && argCount > 0)
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
        auto opInfo = getIROpInfo(inst->getOp());

        dumpIndent(context);
        dump(context, opInfo.name);
        dump(context, " ");
        dumpID(context, inst);

        dumpInstTypeClause(context, inst->getFullType());

        dumpInstOperandList(context, inst);

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

        auto op = inst->getOp();
        auto opInfo = getIROpInfo(op);

        // Special-case the literal instructions.
        if(auto irConst = as<IRConstant>(inst))
        {
            switch (op)
            {
            case kIROp_IntLit:
                dump(context, irConst->value.intVal);
                dump(context, " : ");
                dumpType(context, irConst->getFullType());
                return;

            case kIROp_FloatLit:
                dump(context, irConst->value.floatVal);
                dump(context, " : ");
                dumpType(context, irConst->getFullType());
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
        dumpDebugID(context, inst);
        dumpInstOperandList(context, inst);
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

        auto op = inst->getOp();

        dumpIRDecorations(context, inst);

        // There are several ops we want to special-case here,
        // so that they will be more pleasant to look at.
        //
        switch (op)
        {
        case kIROp_Func:
        case kIROp_GlobalVar:
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

        // Output the originating source location
        {
            SourceManager* sourceManager = context->sourceManager;
            if (sourceManager && context->options.flags & IRDumpOptions::Flag::SourceLocations)
            {
                StringBuilder buf;
                buf << " loc: ";

                // Output the line number information
                if (inst->sourceLoc.isValid())
                {
                    // Might want to output actual, but nominal is okay for default
                    const SourceLocType sourceLocType = SourceLocType::Nominal;

                    // Get the source location
                    const HumaneSourceLoc humaneLoc = sourceManager->getHumaneLoc(inst->sourceLoc, sourceLocType);
                    if (humaneLoc.line >= 0)
                    {
                        buf << humaneLoc.line << "," << humaneLoc.column;

                        if (humaneLoc.pathInfo != context->lastPathInfo)
                        {
                            buf << " ";
                            // Output the the location
                            humaneLoc.pathInfo.appendDisplayName(buf);
                            context->lastPathInfo = humaneLoc.pathInfo;
                        }
                    }
                    else
                    {
                        buf << "not found";
                    }
                }
                else
                {
                    buf << "na";
                }

                dump(context, buf.getUnownedSlice());
            }
        }

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

    void printSlangIRAssembly(StringBuilder& builder, IRModule* module, const IRDumpOptions& options, SourceManager* sourceManager)
    {
        IRDumpContext context;
        context.builder = &builder;
        context.indent = 0;
        context.options = options;
        context.sourceManager = sourceManager;
        
        dumpIRModule(&context, module);
    }

    void dumpIR(IRInst* globalVal, const IRDumpOptions& options, SourceManager* sourceManager, ISlangWriter* writer)
    {
        StringBuilder sb;

        IRDumpContext context;
        context.builder = &sb;
        context.indent = 0;
        context.options = options;
        context.sourceManager = sourceManager;

        dumpInst(&context, globalVal);

        writer->write(sb.getBuffer(), sb.getLength());
        writer->flush();
    }

    void dumpIR(IRModule* module, const IRDumpOptions& options, char const* label, SourceManager* sourceManager, ISlangWriter* inWriter)
    {
        WriterHelper writer(inWriter);

        if (label)
        {
            writer.put("### ");
            writer.put(label);
            writer.put(":\n");
        }

        dumpIR(module, options, sourceManager, inWriter);

        if (label)
        {
            writer.put("###\n");
        }
    }

    String getSlangIRAssembly(IRModule* module, const IRDumpOptions& options, SourceManager* sourceManager)
    {
        StringBuilder sb;
        printSlangIRAssembly(sb, module, options, sourceManager);
        return sb;
    }

    void dumpIR(IRModule* module, const IRDumpOptions& options, SourceManager* sourceManager, ISlangWriter* writer)
    {
        String ir = getSlangIRAssembly(module, options, sourceManager);
        writer->write(ir.getBuffer(), ir.getLength());
        writer->flush();
    }

    // Pre-declare
    static bool _isTypeOperandEqual(IRInst* a, IRInst* b);

    static bool _areTypeOperandsEqual(IRInst* a, IRInst* b)
    {
        // Must have same number of operands
        const auto operandCountA = Index(a->getOperandCount());
        if (operandCountA != Index(b->getOperandCount()))
        {
            return false;
        }

        // All the operands must be equal
        for (Index i = 0; i < operandCountA; ++i)
        {
            IRInst* operandA = a->getOperand(i);
            IRInst* operandB = b->getOperand(i);

            if (!_isTypeOperandEqual(operandA, operandB))
            {
                return false;
            }
        }

        return true;
    }

    bool isNominalOp(IROp op)
    {
        // True if the op identity is 'nominal'
        switch (op)
        {
            case kIROp_StructType:
            case kIROp_ClassType:
            case kIROp_InterfaceType:
            case kIROp_Generic:
            case kIROp_Param:
            {
                return true;
            }
        }
        return false;
    }

    // True if a type operand is equal. Operands are 'IRInst' - but it's only a restricted set that
    // can be operands of IRType instructions
    static bool _isTypeOperandEqual(IRInst* a, IRInst* b)
    {
        if (a == b)
        {
            return true;
        }

        if (a == nullptr || b == nullptr)
        {
            return false;
        }

        const IROp opA = IROp(a->getOp() & kIROpMeta_OpMask);
        const IROp opB = IROp(b->getOp() & kIROpMeta_OpMask);

        if (opA != opB)
        {
            return false;
        }

        // If the type is nominal - it can only be the same if the pointer is the same.
        if (isNominalOp(opA))
        {
            // The pointer isn't the same (as that was already tested), so cannot be equal
            return false;
        }

        // Both are types
        if (IRType::isaImpl(opA))
        {
            if (IRBasicType::isaImpl(opA))
            {
                // If it's a basic type, then their op being the same means we are done
                return true;
            }

            // We don't care about the parent or positioning
            // We also don't care about 'type' - because these instructions are defining the type.
            // 
            // We may want to care about decorations.

            // If it's a resource type - special case the handling of the resource flavor 
            if (IRResourceTypeBase::isaImpl(opA) &&
                static_cast<const IRResourceTypeBase*>(a)->getFlavor() != static_cast<const IRResourceTypeBase*>(b)->getFlavor())
            {
                return false;
            }

            // TODO(JS): There is a question here about what to do about decorations.
            // For now we ignore decorations. Are two types potentially different if there decorations different?
            // If decorations play a part in difference in types - the order of decorations presumably is not important.

            // All the operands of the types must be equal
            return _areTypeOperandsEqual(a, b);
        }
       
        // If it's a constant...
        if (IRConstant::isaImpl(opA))
        {
            // TODO: This is contrived in that we want two types that are the same, but are different
            // pointers to match here.
            // If we make getHashCode for IRType* compatible with isTypeEqual, then we should probably use that.
            return static_cast<IRConstant*>(a)->isValueEqual(static_cast<IRConstant*>(b)) &&
                isTypeEqual(a->getFullType(), b->getFullType());
        }

        SLANG_ASSERT(!"Unhandled comparison");

        // We can't equate any other type..
        return false;
    }

    bool isTypeEqual(IRType* a, IRType* b)
    {
        // _isTypeOperandEqual handles comparison of types so can defer to it
        return _isTypeOperandEqual(a, b);
    }
     
    void findAllInstsBreadthFirst(IRInst* inst, List<IRInst*>& outInsts)
    {
        Index index = outInsts.getCount();

        outInsts.add(inst);

        while (index < outInsts.getCount())
        {
            IRInst* cur = outInsts[index++];

            IRInstListBase childrenList = cur->getDecorationsAndChildren();
            for (IRInst* child : childrenList)
            {
                outInsts.add(child);
            }
        }
    }

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
        removeFromParent();
        _insertAt(other, other->getNextInst(), other->getParent());
    }

    void IRInst::insertAtEnd(IRInst* newParent)
    {
        SLANG_ASSERT(newParent);
        removeFromParent();
        _insertAt(newParent->getLastDecorationOrChild(), nullptr, newParent);
    }

    void IRInst::moveToEnd()
    {
        auto p = parent;
        removeFromParent();
        insertAtEnd(p);
    }

    void IRInst::insertAt(IRInsertLoc const& loc)
    {
        removeFromParent();
        IRInst* other = loc.getInst();
        switch(loc.getMode())
        {
        case IRInsertLoc::Mode::None:
            break;
        case IRInsertLoc::Mode::Before:
            insertBefore(other);
            break;
        case IRInsertLoc::Mode::After:
            insertAfter(other);
            break;
        case IRInsertLoc::Mode::AtStart:
            insertAtStart(other);
            break;
        case IRInsertLoc::Mode::AtEnd:
            insertAtEnd(other);
            break;
        }
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

    void IRInst::transferDecorationsTo(IRInst* target)
    {
        while( auto decoration = getFirstDecoration() )
        {
            decoration->removeFromParent();
            decoration->insertAtStart(target);
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

        if(as<IRLayout>(this))
            return false;

        if(as<IRAttr>(this))
            return false;

        switch(getOp())
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
        case kIROp_RTTIPointerType:
        case kIROp_RTTIObject:
        case kIROp_RTTIType:
        case kIROp_Func:
        case kIROp_Generic:
        case kIROp_Var:
        case kIROp_GlobalVar: // Note: the IRGlobalVar represents the *address*, so only a load/store would have side effects
        case kIROp_GlobalConstant:
        case kIROp_GlobalParam:
        case kIROp_StructKey:
        case kIROp_GlobalGenericParam:
        case kIROp_WitnessTable:
        case kIROp_WitnessTableEntry:
        case kIROp_InterfaceRequirementEntry:
        case kIROp_Block:
            return false;

            /// Liveness markers have no side effects
        case kIROp_LiveRangeStart:
        case kIROp_LiveRangeEnd:

        case kIROp_Nop:
        case kIROp_undefined:
        case kIROp_DefaultConstruct:
        case kIROp_Specialize:
        case kIROp_lookup_interface_method:
        case kIROp_GetSequentialID:
        case kIROp_getAddr:
        case kIROp_GetValueFromBoundInterface:
        case kIROp_Construct:
        case kIROp_makeUInt64:
        case kIROp_makeVector:
        case kIROp_MakeMatrix:
        case kIROp_makeArray:
        case kIROp_makeStruct:
        case kIROp_makeString:
        case kIROp_getNativeStr:
        case kIROp_MakeResultError:
        case kIROp_MakeResultValue:
        case kIROp_GetResultError:
        case kIROp_GetResultValue:
        case kIROp_IsResultError:
        case kIROp_MakeOptionalValue:
        case kIROp_MakeOptionalNone:
        case kIROp_OptionalHasValue:
        case kIROp_GetOptionalValue:
        case kIROp_Load:    // We are ignoring the possibility of loads from bad addresses, or `volatile` loads
        case kIROp_ImageSubscript:
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
        //case kIROp_Rem:
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
        case kIROp_MakeExistential:
        case kIROp_ExtractExistentialType:
        case kIROp_ExtractExistentialValue:
        case kIROp_ExtractExistentialWitnessTable:
        case kIROp_WrapExistential:
        case kIROp_BitCast:
        case kIROp_AllocObj:
        case kIROp_PackAnyValue:
        case kIROp_UnpackAnyValue:
        case kIROp_Reinterpret:
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

    //
    // IRTargetIntrinsicDecoration
    //

    IRTargetIntrinsicDecoration* findAnyTargetIntrinsicDecoration(
            IRInst*                 val)
    {
        IRInst* inst = getResolvedInstForDecorations(val);
        return inst->findDecoration<IRTargetIntrinsicDecoration>();
    }

    IRTargetSpecificDecoration* findBestTargetDecoration(
        IRInst*                 inInst,
        CapabilitySet const&    targetCaps)
    {
        IRInst* inst = getResolvedInstForDecorations(inInst);

        // We will search through all the `IRTargetIntrinsicDecoration`s on
        // the instruction, looking for those that are applicable to the
        // current code generation target. Among the application decorations
        // we will try to find one that is "best" in the sense that it is
        // more (or at least as) specialized for the target than the
        // others.
        //
        IRTargetSpecificDecoration* bestDecoration = nullptr;
        CapabilitySet bestCaps;
        for(auto dd : inst->getDecorations())
        {
            auto decoration = as<IRTargetSpecificDecoration>(dd);
            if(!decoration)
                continue;

            auto decorationCaps = decoration->getTargetCaps();
            if (decorationCaps.isIncompatibleWith(targetCaps))
                continue;

            if(!bestDecoration || decorationCaps.isBetterForTarget(bestCaps, targetCaps))
            {
                bestDecoration = decoration;
                bestCaps = decorationCaps;
            }
        }

        return bestDecoration;
    }

    IRTargetSpecificDecoration* findBestTargetDecoration(
            IRInst*         val,
            CapabilityAtom  targetCapabilityAtom)
    {
        return findBestTargetDecoration(val, CapabilitySet(targetCapabilityAtom));
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

        auto returnInst = as<IRReturn>(lastBlock->getTerminator());
        if (!returnInst)
            return nullptr;

        auto val = returnInst->getVal();
        return val;
    }
    
    IRInst* findInnerMostGenericReturnVal(IRGeneric* generic)
    {
        IRInst* inst = generic;
        while (auto genericInst = as<IRGeneric>(inst))
            inst = findGenericReturnVal(genericInst);
        return inst;
    }

    IRGeneric* findSpecializedGeneric(IRSpecialize* specialize)
    {
        return as<IRGeneric>(specialize->getBase());
    }


    IRInst* findSpecializeReturnVal(IRSpecialize* specialize)
    {
        auto base = specialize->getBase();

        while( auto baseSpec = as<IRSpecialize>(base) )
        {
            auto returnVal = findSpecializeReturnVal(baseSpec);
            if(!returnVal)
                break;

            base = returnVal;
        }

        if( auto generic = as<IRGeneric>(base) )
        {
            return findGenericReturnVal(generic);
        }

        return nullptr;
    }

    IRInst* getResolvedInstForDecorations(IRInst* inst)
    {
        IRInst* candidate = inst;
        for(;;)
        {
            if(auto specInst = as<IRSpecialize>(candidate))
            {
                candidate = specInst->getBase();
                continue;
            }
            if( auto genericInst = as<IRGeneric>(candidate) )
            {
                if( auto returnVal = findGenericReturnVal(genericInst) )
                {
                    candidate = returnVal;
                    continue;
                }
            }

            return candidate;
        }
    }

    bool isDefinition(
        IRInst* inVal)
    {
        IRInst* val = inVal;
        // unwrap any generic declarations to see
        // the value they return.
        for(;;)
        {
            // An instruciton marked `[import(...)]` cannot
            // be a definition, since it is claiming that
            // the actual body comes from another module.
            //
            if(val->findDecoration<IRImportDecoration>())
                return false;

            auto genericInst = as<IRGeneric>(val);
            if(!genericInst)
                break;

            auto returnVal = findGenericReturnVal(genericInst);
            if(!returnVal)
                break;

            val = returnVal;
        }

        // Some cases of instructions have structural
        // rules about when they are considered to have
        // a definition (e.g., a function must have a body).
        //
        switch (val->getOp())
        {
        case kIROp_Func:
        case kIROp_Generic:
            return val->getFirstChild() != nullptr;

        case kIROp_GlobalConstant:
            return cast<IRGlobalConstant>(val)->getValue() != nullptr;

        default:
            break;
        }

        // In all other cases, if we have an instruciton
        // that has *not* been marked for import, then
        // we consider it to be a definition.

        return true;
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

    bool isBuiltin(IRInst* inst)
    {
        return inst->findDecoration<IRBuiltinDecoration>() != nullptr;
    }
    IRFunc* getParentFunc(IRInst* inst)
    {
        auto parent = inst->getParent();
        while (parent)
        {
            if (auto func = as<IRFunc>(parent))
                return func;
            parent = parent->getParent();
        }
        return nullptr;
    }
} // namespace Slang

#if SLANG_VC
#ifdef _DEBUG
// Natvis sometimes cannot find enum values.
// Export symbols for them to make sure natvis works correctly when debugging external projects.
SLANG_API const int SlangDebug__IROpNameHint = Slang::kIROp_NameHintDecoration;
SLANG_API const int SlangDebug__IROpExport = Slang::kIROp_ExportDecoration;
SLANG_API const int SlangDebug__IROpImport = Slang::kIROp_ImportDecoration;
SLANG_API const int SlangDebug__IROpStringLit = Slang::kIROp_StringLit;
SLANG_API const int SlangDebug__IROpIntLit = Slang::kIROp_IntLit;
#endif
#endif

