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


    static const IROpInfo kIROpInfos[] =
    {
#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    { #MNEMONIC, ARG_COUNT, FLAGS, },
#include "ir-inst-defs.h"
    };

    //

    IROp findIROp(char const* name)
    {
        // TODO: need to make this faster by using a dictionary...

        static const struct {
            char const* mnemonic;
            IROp op;
        } kOps[] = {

#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
        { #MNEMONIC, kIROp_##ID },

#define PSEUDO_INST(ID)  \
        { #ID, kIRPseudoOp_##ID },

#include "ir-inst-defs.h"

        };

        for (auto ee : kOps)
        {
            if (strcmp(name, ee.mnemonic) == 0)
                return ee.op;
        }

        return IROp(kIROp_Invalid);
    }

    IROpInfo getIROpInfo(IROp op)
    {
        return kIROpInfos[op];
    }

    //

    void IRUse::debugValidate()
    {
#ifdef _DEBUG
        auto uv = this->usedValue;
        if(!uv)
        {
            assert(!user);
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

    void IRUse::init(IRUser* u, IRValue* v)
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

    void IRUse::set(IRValue* uv)
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

    IRUse* IRUser::getArgs()
    {
        // We assume that *all* instructions are laid out
        // in memory such that their arguments come right
        // after the first `sizeof(IRInst)` bytes.
        //
        // TODO: we probably need to be careful and make
        // this more robust.

        return (IRUse*)(this + 1);
    }

    IRDecoration* IRValue::findDecorationImpl(IRDecorationOp decorationOp)
    {
        for( auto dd = firstDecoration; dd; dd = dd->next )
        {
            if(dd->op == decorationOp)
                return dd;
        }
        return nullptr;
    }

    // IRBlock

    void IRBlock::addParam(IRParam* param)
    {
        if (auto lp = lastParam)
        {
            lp->nextParam = param;
            param->prevParam = lp;
        }
        else
        {
            firstParam = param;
        }
        lastParam = param;
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
        if (!terminator || !isTerminatorInst(terminator))
            return IRBlock::SuccessorList(nullptr, nullptr);

        // Otherwise, based on the opcode of the terminator
        // instruction, we will build up our list of uses.
        IRUse* begin = nullptr;
        IRUse* end = nullptr;
        UInt stride = 1;

        auto args = terminator->getArgs();
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
            begin = args + 0;
            end = begin + 1;
            break;

        case kIROp_conditionalBranch:
        case kIROp_ifElse:
            // conditionalBranch <condition> <trueBlock> <falseBlock>
            begin = args + 1;
            end = begin + 2;
            break;

        case kIROp_switch:
            // switch <val> <break> <default> <caseVal1> <caseBlock1> ...
            begin = args + 4;
            end = args + terminator->getArgCount() + 1;
            stride = 2;
            break;

        default:
            assert(!"unepxected");
            return IRBlock::SuccessorList(nullptr, nullptr);
        }

        return IRBlock::SuccessorList(begin, end, stride);
    }

    void IRBlock::insertAfter(IRBlock* other)
    {
        assert(other);
        insertAfter(other, other->parentFunc);
    }

    void IRBlock::insertAfter(IRBlock* other, IRGlobalValueWithCode* func)
    {
        assert(other || func);

        if (!other) other = func->lastBlock;
        if (!func) func = other->parentFunc;

        assert(func);

        auto pp = other;
        auto nn = other ? other->nextBlock : nullptr;

        if (pp)
        {
            pp->nextBlock = this;
        }
        else
        {
            func->firstBlock = this;
        }

        if (nn)
        {
            nn->prevBlock = this;
        }
        else
        {
            func->lastBlock = this;
        }

        this->prevBlock = pp;
        this->nextBlock = nn;
        this->parentFunc = func;
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

    // IRFunc

    IRType* IRFunc::getResultType() { return getType()->getResultType(); }
    UInt IRFunc::getParamCount() { return getType()->getParamCount(); }
    IRType* IRFunc::getParamType(UInt index) { return getType()->getParamType(index); }

    IRParam* IRFunc::getFirstParam()
    {
        auto entryBlock = getFirstBlock();
        if(!entryBlock) return nullptr;

        return entryBlock->getFirstParam();
    }

    void IRGlobalValueWithCode::addBlock(IRBlock* block)
    {
        block->parentFunc = this;

        if (auto lb = lastBlock)
        {
            lb->nextBlock = block;
            block->prevBlock = lb;
        }
        else
        {
            firstBlock = block;
        }
        lastBlock = block;
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

    void IRValueListBase::addImpl(IRValue* parent, IRChildValue* val)
    {
        val->parent = parent;
        val->prev = last;
        val->next = nullptr;

        if (last)
        {
            last->next = val;
        }
        else
        {
            first = val;
        }

        last = val;
    }


    //

    // Add an instruction to a specific parent
    void IRBuilder::addInst(IRBlock* pblock, IRInst* inst)
    {
        inst->parent = pblock;

        if (!pblock->firstInst)
        {
            inst->prev = nullptr;
            inst->next = nullptr;

            pblock->firstInst = inst;
            pblock->lastInst = inst;
        }
        else
        {
            auto prev = pblock->lastInst;

            inst->prev = prev;
            inst->next = nullptr;

            prev->next = inst;
            pblock->lastInst = inst;
        }
    }

    // Add an instruction into the current scope
    void IRBuilder::addInst(
        IRInst*     inst)
    {
        if(insertBeforeInst)
        {
            inst->insertBefore(insertBeforeInst);
            return;
        }

        auto parent = curBlock;
        if (!parent)
            return;

        addInst(parent, inst);
    }

    static void maybeSetSourceLoc(
        IRBuilder*  builder,
        IRValue*    value)
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

    template<typename T>
    static T* createValue(
        IRBuilder*  builder,
        IROp        op,
        IRType*     type)
    {
        assert(builder->getModule());
        T* value = (T*)builder->getModule()->memoryPool.allocZero(sizeof(T));
        new(value)T();
        value->op = op;
        value->type = type;
        builder->getModule()->irObjectsToFree.Add(value);
        return value;
    }


    // Create an IR instruction/value and initialize it.
    //
    // In this case `argCount` and `args` represnt the
    // arguments *after* the type (which is a mandatory
    // argument for all instructions).
    template<typename T>
    static T* createInstImpl(
        IRBuilder*      builder,
        UInt            size,
        IROp            op,
        IRType*         type,
        UInt            fixedArgCount,
        IRValue* const* fixedArgs,
        UInt            varArgCount = 0,
        IRValue* const* varArgs = nullptr)
    {
        assert(builder->getModule());
        T* inst = (T*)builder->getModule()->memoryPool.allocZero(size);
        new(inst)T();

        inst->argCount = (uint32_t)(fixedArgCount + varArgCount);

        inst->op = op;

        inst->type = type;

        maybeSetSourceLoc(builder, inst);

        auto operand = inst->getArgs();

        for( UInt aa = 0; aa < fixedArgCount; ++aa )
        {
            if (fixedArgs)
            {
                operand->init(inst, fixedArgs[aa]);
            }
            operand++;
        }

        for( UInt aa = 0; aa < varArgCount; ++aa )
        {
            if (varArgs)
            {
                operand->init(inst, varArgs[aa]);
            }
            operand++;
        }
        builder->getModule()->irObjectsToFree.Add(inst);
        return inst;
    }

    template<typename T>
    static T* createInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        UInt            argCount,
        IRValue* const* args)
    {
        return createInstImpl<T>(
            builder,
            sizeof(T),
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
            sizeof(T),
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
        IRValue*        arg)
    {
        return createInstImpl<T>(
            builder,
            sizeof(T),
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
        IRValue*        arg1,
        IRValue*        arg2)
    {
        IRValue* args[] = { arg1, arg2 };
        return createInstImpl<T>(
            builder,
            sizeof(T),
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
        IRValue* const* args)
    {
        return createInstImpl<T>(
            builder,
            sizeof(T) + argCount * sizeof(IRUse),
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
        IRValue* const* fixedArgs,
        UInt            varArgCount,
        IRValue* const* varArgs)
    {
        return createInstImpl<T>(
            builder,
            sizeof(T) + varArgCount * sizeof(IRUse),
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
        IRValue*        arg1,
        UInt            varArgCount,
        IRValue* const* varArgs)
    {
        IRValue* fixedArgs[] = { arg1 };
        UInt fixedArgCount = sizeof(fixedArgs) / sizeof(fixedArgs[0]);

        return createInstImpl<T>(
            builder,
            sizeof(T) + varArgCount * sizeof(IRUse),
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
        if(left.inst->parent != right.inst->parent) return false;
        if(left.inst->argCount != right.inst->argCount) return false;

        auto argCount = left.inst->argCount;
        auto leftArgs = left.inst->getArgs();
        auto rightArgs = right.inst->getArgs();
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
        code = combineHash(code, Slang::GetHashCode(inst->parent));
        code = combineHash(code, Slang::GetHashCode(inst->argCount));

        auto argCount = inst->argCount;
        auto args = inst->getArgs();
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            code = combineHash(code, Slang::GetHashCode(args[aa].get()));
        }
        return code;
    }

    //

    bool operator==(IRConstantKey const& left, IRConstantKey const& right)
    {
        if(left.inst->op            != right.inst->op)              return false;
        if(left.inst->type          != right.inst->type)            return false;
        if(left.inst->u.ptrData[0]  != right.inst->u.ptrData[0])    return false;
        if(left.inst->u.ptrData[1]  != right.inst->u.ptrData[1])    return false;
        return true;
    }

    int IRConstantKey::GetHashCode()
    {
        auto code = Slang::GetHashCode(inst->op);
        code = combineHash(code, Slang::GetHashCode(inst->type));
        code = combineHash(code, Slang::GetHashCode(inst->u.ptrData[0]));
        code = combineHash(code, Slang::GetHashCode(inst->u.ptrData[1]));
        return code;
    }

    static IRConstant* findOrEmitConstant(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        UInt            valueSize,
        void const*     value)
    {
        // First, we need to pick a good insertion point
        // for the instruction, which we do by looking
        // at its operands.
        //

        IRConstant keyInst;
        memset(&keyInst, 0, sizeof(keyInst));
        keyInst.op = op;
        keyInst.type = type;
        memcpy(&keyInst.u, value, valueSize);

        IRConstantKey key;
        key.inst = &keyInst;

        IRConstant* irValue = nullptr;
        if( builder->sharedBuilder->constantMap.TryGetValue(key, irValue) )
        {
            // We found a match, so just use that.
            return irValue;
        }

        // We now know where we want to insert, but there might
        // already be an equivalent instruction in that block.
        //
        // We will check for such an instruction in a slightly hacky
        // way: we will construct a temporary instruction and
        // then use it to look up in a cache of instructions.

        irValue = createValue<IRConstant>(builder, op, type);
        memcpy(&irValue->u, value, valueSize);

        key.inst = irValue;
        builder->sharedBuilder->constantMap.Add(key, irValue);

        return irValue;
    }


    //

    IRValue* IRBuilder::getBoolValue(bool inValue)
    {
        IRIntegerValue value = inValue;
        return findOrEmitConstant(
            this,
            kIROp_boolConst,
            getSession()->getBoolType(),
            sizeof(value),
            &value);
    }

    IRValue* IRBuilder::getIntValue(IRType* type, IRIntegerValue value)
    {
        return findOrEmitConstant(
            this,
            kIROp_IntLit,
            type,
            sizeof(value),
            &value);
    }

    IRValue* IRBuilder::getFloatValue(IRType* type, IRFloatingPointValue value)
    {
        return findOrEmitConstant(
            this,
            kIROp_FloatLit,
            type,
            sizeof(value),
            &value);
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

    IRValue* IRBuilder::getDeclRefVal(
        DeclRefBase const&  declRef)
    {
        // TODO: we should cache these...
        auto irValue = createValue<IRDeclRef>(
            this,
            kIROp_decl_ref,
            nullptr);
        irValue->declRef = DeclRef<Decl>(declRef.decl, declRef.substitutions);
        return irValue;
    }

    IRValue * IRBuilder::getTypeVal(IRType * type)
    {
        auto irValue = createValue<IRValue>(
            this,
            kIROp_TypeType,
            nullptr);
        irValue->type = type;
        if (auto typetype = dynamic_cast<TypeType*>(type))
            irValue->type = typetype->type;
        return irValue;
    }

    IRValue* IRBuilder::emitSpecializeInst(
        Type*       type,
        IRValue*    genericVal,
        IRValue*    specDeclRef)
    {
        auto inst = createInst<IRSpecialize>(
            this,
            kIROp_specialize,
            type,
            genericVal,
            specDeclRef);
        addInst(inst);
        return inst;
    }

    IRValue* IRBuilder::emitSpecializeInst(
        Type*           type,
        IRValue*        genericVal,
        DeclRef<Decl>   specDeclRef)
    {
        auto specDeclRefVal = getDeclRefVal(specDeclRef);
        auto inst = createInst<IRSpecialize>(
            this,
            kIROp_specialize,
            type,
            genericVal,
            specDeclRefVal);
        addInst(inst);
        return inst;
    }

    IRValue* IRBuilder::emitLookupInterfaceMethodInst(
        IRType*     type,
        IRValue*    witnessTableVal,
        IRValue*    interfaceMethodVal)
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

    IRValue* IRBuilder::emitLookupInterfaceMethodInst(
        IRType*         type,
        DeclRef<Decl>   witnessTableDeclRef,
        DeclRef<Decl>   interfaceMethodDeclRef)
    {
        auto witnessTableVal = getDeclRefVal(witnessTableDeclRef);
        DeclRef<Decl> removeSubstDeclRef = interfaceMethodDeclRef;
        removeSubstDeclRef.substitutions = SubstitutionSet();
        auto interfaceMethodVal = getDeclRefVal(removeSubstDeclRef);
        return emitLookupInterfaceMethodInst(type, witnessTableVal, interfaceMethodVal);
    }

    IRValue* IRBuilder::emitLookupInterfaceMethodInst(
        IRType*         type,
        IRValue*   witnessTableVal,
        DeclRef<Decl>   interfaceMethodDeclRef)
    {
        DeclRef<Decl> removeSubstDeclRef = interfaceMethodDeclRef;
        removeSubstDeclRef.substitutions = SubstitutionSet();
        auto interfaceMethodVal = getDeclRefVal(removeSubstDeclRef);
        return emitLookupInterfaceMethodInst(type, witnessTableVal, interfaceMethodVal);
    }

    IRValue* IRBuilder::emitFindWitnessTable(
        DeclRef<Decl> baseTypeDeclRef,
        IRType* interfaceType)
    {
        auto interfaceTypeDeclRef = interfaceType->AsDeclRefType();
        SLANG_ASSERT(interfaceTypeDeclRef);
        auto inst = createInst<IRLookupWitnessTable>(
            this,
            kIROp_lookup_witness_table,
            interfaceType,
            getDeclRefVal(baseTypeDeclRef),
            getDeclRefVal(interfaceTypeDeclRef->declRef));
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitCallInst(
        IRType*         type,
        IRValue*        pFunc,
        UInt            argCount,
        IRValue* const* args)
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
        IRValue* const* args)
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
        IRValue* const* args)
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
        IRValue* const* args)
    {
        return emitIntrinsicInst(type, kIROp_makeVector, argCount, args);
    }

    IRInst* IRBuilder::emitMakeArray(
        IRType*         type,
        UInt            argCount,
        IRValue* const* args)
    {
        return emitIntrinsicInst(type, kIROp_makeArray, argCount, args);
    }

    IRInst* IRBuilder::emitMakeStruct(
        IRType*         type,
        UInt            argCount,
        IRValue* const* args)
    {
        return emitIntrinsicInst(type, kIROp_makeStruct, argCount, args);
    }

    IRModule* IRBuilder::createModule()
    {
        auto module = new IRModule();
        module->session = getSession();
        return module;
    }

    void IRGlobalValue::insertBefore(IRGlobalValue* other)
    {
        assert(other);
        insertBefore(other, other->parentModule);
    }

    void IRGlobalValue::insertBefore(IRGlobalValue* other, IRModule* module)
    {
        assert(other || module);

        if(!other) other = module->firstGlobalValue;
        if(!module) module = other->parentModule;

        assert(module);

        auto nn = other;
        auto pp = other ? other->prevGlobalValue : nullptr;

        if(pp)
        {
            pp->nextGlobalValue = this;
        }
        else
        {
            module->firstGlobalValue = this;
        }

        if(nn)
        {
            nn->prevGlobalValue = this;
        }
        else
        {
            module->lastGlobalValue = this;
        }

        this->prevGlobalValue = pp;
        this->nextGlobalValue = nn;
        this->parentModule = module;
    }

    void IRGlobalValue::insertAtStart(IRModule* module)
    {
        insertBefore(module->firstGlobalValue, module);
    }

    void IRGlobalValue::insertAfter(IRGlobalValue* other)
    {
        assert(other);
        insertAfter(other, other->parentModule);
    }

    void IRGlobalValue::insertAfter(IRGlobalValue* other, IRModule* module)
    {
        assert(other || module);

        if(!other) other = module->lastGlobalValue;
        if(!module) module = other->parentModule;

        assert(module);

        auto pp = other;
        auto nn = other ? other->nextGlobalValue : nullptr;

        if(pp)
        {
            pp->nextGlobalValue = this;
        }
        else
        {
            module->firstGlobalValue = this;
        }

        if(nn)
        {
            nn->prevGlobalValue = this;
        }
        else
        {
            module->lastGlobalValue = this;
        }

        this->prevGlobalValue = pp;
        this->nextGlobalValue = nn;
        this->parentModule = module;
    }

    void IRGlobalValue::insertAtEnd(IRModule* module)
    {
        assert(module);
        insertAfter(module->lastGlobalValue, module);
    }

    void IRGlobalValue::removeFromParent()
    {
        auto module = parentModule;
        if(!module)
            return;

        auto pp = this->prevGlobalValue;
        auto nn = this->nextGlobalValue;

        if(pp)
        {
            pp->nextGlobalValue = nn;
        }
        else
        {
            module->firstGlobalValue = nn;
        }

        if( nn )
        {
            nn->prevGlobalValue = pp;
        }
        else
        {
            module->lastGlobalValue = pp;
        }
    }

    void IRGlobalValue::moveToEnd()
    {
        auto module = parentModule;
        removeFromParent();
        insertAtEnd(module);
    }



    void addGlobalValue(
        IRModule*   module,
        IRGlobalValue*  value)
    {
        if(!module)
            return;

        value->parentModule = module;
        value->insertAfter(module->lastGlobalValue, module);
    }

    IRFunc* IRBuilder::createFunc()
    {
        IRFunc* rsFunc = createValue<IRFunc>(
            this,
            kIROp_Func,
            nullptr);
        maybeSetSourceLoc(this, rsFunc);
        addGlobalValue(getModule(), rsFunc);
        return rsFunc;
    }

    IRGlobalVar* IRBuilder::createGlobalVar(
        IRType* valueType)
    {
        auto ptrType = getSession()->getPtrType(valueType);
        IRGlobalVar* globalVar = createValue<IRGlobalVar>(
            this,
            kIROp_global_var,
            ptrType);
        maybeSetSourceLoc(this, globalVar);
        addGlobalValue(getModule(), globalVar);
        return globalVar;
    }

    IRGlobalConstant* IRBuilder::createGlobalConstant(
        IRType* valueType)
    {
        IRGlobalConstant* globalConstant = createValue<IRGlobalConstant>(
            this,
            kIROp_global_constant,
            valueType);
        maybeSetSourceLoc(this, globalConstant);
        addGlobalValue(getModule(), globalConstant);
        return globalConstant;
    }

    IRWitnessTable* IRBuilder::createWitnessTable()
    {
        IRWitnessTable* witnessTable = createValue<IRWitnessTable>(
            this,
            kIROp_witness_table,
            nullptr);
        addGlobalValue(getModule(), witnessTable);
        return witnessTable;
    }

    IRWitnessTableEntry* IRBuilder::createWitnessTableEntry(
        IRWitnessTable* witnessTable,
        IRValue*        requirementKey,
        IRValue*        satisfyingVal)
    {
        IRWitnessTableEntry* entry = createInst<IRWitnessTableEntry>(
            this,
            kIROp_witness_table_entry,
            nullptr,
            requirementKey,
            satisfyingVal);

        if (witnessTable)
        {
            witnessTable->entries.add(witnessTable, entry);
        }

        return entry;
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
        return createValue<IRBlock>(
            this,
            kIROp_Block,
            getSession()->getIRBasicBlockType());
    }

    IRBlock* IRBuilder::emitBlock()
    {
        auto bb = createBlock();

        auto f = this->curFunc;
        if (f)
        {
            f->addBlock(bb);
            this->curBlock = bb;
        }
        return bb;
    }

    IRParam* IRBuilder::createParam(
        IRType* type)
    {
        auto param = createValue<IRParam>(
            this,
            kIROp_Param,
            type);
        return param;
    }

    IRParam* IRBuilder::emitParam(
        IRType* type)
    {
        auto param = createParam(type);

        if (auto bb = curBlock)
        {
            bb->addParam(param);
        }
        return param;
    }

    IRVar* IRBuilder::emitVar(
        IRType*         type)
    {
        auto allocatedType = getSession()->getPtrType(type);
        auto inst = createInst<IRVar>(
            this,
            kIROp_Var,
            allocatedType);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitLoad(
        IRValue*    ptr)
    {
        // Note: a `load` operation does not consider the rate
        // (if any) attached to its operand (see the use of `getDataType`
        // below). This means that a load from a rate-qualified
        // variable will still conceptually execute (and return
        // results) at the "default" rate of the parent function,
        // unless a subsequent analysis pass constraints it.

        RefPtr<Type> valueType;
        if(auto ptrType = ptr->getDataType()->As<PtrTypeBase>())
        {
            valueType = ptrType->getValueType();
        }
        else if(auto ptrLikeType = ptr->getDataType()->As<PointerLikeType>())
        {
            valueType = ptrLikeType->getElementType();
        }
        else
        {
            // Bad!
            SLANG_ASSERT(ptrType);
            return nullptr;
        }

        // Ugly special case: the result of loading from `groupshared`
        // memory should not itself be `groupshared`.
        //
        // TODO: This special case will go away once `GroupSharedType`
        // is replaced by a `GroupSharedRate` that gets used together
        // with `RateQualifiedType`.
        if(auto rateType = valueType->As<GroupSharedType>())
        {
            valueType = rateType->valueType;
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
        IRValue*    dstPtr,
        IRValue*    srcVal)
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
        IRType*         type,
        IRValue*        base,
        IRValue*        field)
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
        IRType*         type,
        IRValue*        base,
        IRValue*        field)
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
        IRType*     type,
        IRValue*    base,
        IRValue*    index)
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
        IRValue*    basePtr,
        IRValue*    index)
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
        IRValue*        base,
        UInt            elementCount,
        IRValue* const* elementIndices)
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
        IRValue*        base,
        UInt            elementCount,
        UInt const*     elementIndices)
    {
        auto intType = getSession()->getBuiltinType(BaseType::Int);

        IRValue* irElementIndices[4];
        for (UInt ii = 0; ii < elementCount; ++ii)
        {
            irElementIndices[ii] = getIntValue(intType, elementIndices[ii]);
        }

        return emitSwizzle(type, base, elementCount, irElementIndices);
    }


    IRInst* IRBuilder::emitSwizzleSet(
        IRType*         type,
        IRValue*        base,
        IRValue*        source,
        UInt            elementCount,
        IRValue* const* elementIndices)
    {
        IRValue* fixedArgs[] = { base, source };
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
        IRValue*        base,
        IRValue*        source,
        UInt            elementCount,
        UInt const*     elementIndices)
    {
        auto intType = getSession()->getBuiltinType(BaseType::Int);

        IRValue* irElementIndices[4];
        for (UInt ii = 0; ii < elementCount; ++ii)
        {
            irElementIndices[ii] = getIntValue(intType, elementIndices[ii]);
        }

        return emitSwizzleSet(type, base, source, elementCount, irElementIndices);
    }

    IRInst* IRBuilder::emitReturn(
        IRValue*    val)
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
        IRValue* args[] = { target, breakBlock, continueBlock };
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
        IRValue*    val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock)
    {
        IRValue* args[] = { val, trueBlock, falseBlock };
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
        IRValue*    val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock,
        IRBlock*    afterBlock)
    {
        IRValue* args[] = { val, trueBlock, falseBlock, afterBlock };
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
        IRValue*    val,
        IRBlock*    trueBlock,
        IRBlock*    afterBlock)
    {
        return emitIfElse(val, trueBlock, afterBlock, afterBlock);
    }

    IRInst* IRBuilder::emitLoopTest(
        IRValue*    val,
        IRBlock*    bodyBlock,
        IRBlock*    breakBlock)
    {
        return emitIfElse(val, bodyBlock, breakBlock, bodyBlock);
    }

    IRInst* IRBuilder::emitSwitch(
        IRValue*        val,
        IRBlock*        breakLabel,
        IRBlock*        defaultLabel,
        UInt            caseArgCount,
        IRValue* const* caseArgs)
    {
        IRValue* fixedArgs[] = { val, breakLabel, defaultLabel };
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

    IRHighLevelDeclDecoration* IRBuilder::addHighLevelDeclDecoration(IRValue* inst, Decl* decl)
    {
        auto decoration = addDecoration<IRHighLevelDeclDecoration>(inst, kIRDecorationOp_HighLevelDecl);
        decoration->decl = decl;
        return decoration;
    }

    IRLayoutDecoration* IRBuilder::addLayoutDecoration(IRValue* inst, Layout* layout)
    {
        auto decoration = addDecoration<IRLayoutDecoration>(inst);
        decoration->layout = layout;
        return decoration;
    }

    //


    struct IRDumpContext
    {
        StringBuilder* builder;
        int     indent = 0;

        UInt                        idCounter = 1;
        Dictionary<IRValue*, UInt>  mapValueToID;
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

    bool opHasResult(IRValue* inst);

    static UInt getID(
        IRDumpContext*  context,
        IRValue*        value)
    {
        UInt id = 0;
        if (context->mapValueToID.TryGetValue(value, id))
            return id;

        if (opHasResult(value))
        {
            id = context->idCounter++;
        }

        context->mapValueToID.Add(value, id);
        return id;
    }

    static void dumpID(
        IRDumpContext* context,
        IRValue*        inst)
    {
        if (!inst)
        {
            dump(context, "<null>");
            return;
        }

        switch(inst->op)
        {
        case kIROp_Func:
        case kIROp_global_var:
        case kIROp_global_constant:
        case kIROp_witness_table:
            {
                auto irFunc = (IRFunc*) inst;
                dump(context, "@");
                dump(context, getText(irFunc->mangledName).Buffer());
            }
            break;

        default:
            {
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
            break;
        }
    }

    static void dumpType(
        IRDumpContext*  context,
        IRType*         type);

    static void dumpDeclRef(
        IRDumpContext*          context,
        DeclRef<Decl> const&    declRef);

    static void dumpOperand(
        IRDumpContext*  context,
        IRValue*        inst)
    {
        // TODO: we should have a dedicated value for the `undef` case
        if (!inst)
        {
            dump(context, "undef");
            return;
        }

        switch (inst->op)
        {
        case kIROp_IntLit:
            dump(context, ((IRConstant*)inst)->u.intVal);
            return;

        case kIROp_FloatLit:
            dump(context, ((IRConstant*)inst)->u.floatVal);
            return;

        case kIROp_boolConst:
            dump(context, ((IRConstant*)inst)->u.intVal ? "true" : "false");
            return;

        case kIROp_TypeType:
            dumpType(context, (IRType*)inst);
            return;

        case kIROp_decl_ref:
            dump(context, "$\"");
            dumpDeclRef(context, ((IRDeclRef*)inst)->declRef);
            dump(context, "\"");
            return;

        default:
            break;
        }

        dumpID(context, inst);
    }

    static void dump(
        IRDumpContext*  context,
        Name*           name)
    {
        dump(context, getText(name).Buffer());
    }

    static void dumpVal(
        IRDumpContext*  context,
        Val*            val)
    {
        if(auto type = dynamic_cast<Type*>(val))
        {
            dumpType(context, type);
        }
        else if(auto constIntVal = dynamic_cast<ConstantIntVal*>(val))
        {
            dump(context, constIntVal->value);
        }
        else if(auto genericParamVal = dynamic_cast<GenericParamIntVal*>(val))
        {
            dumpDeclRef(context, genericParamVal->declRef);
        }
        else if(auto declaredSubtypeWitness = dynamic_cast<DeclaredSubtypeWitness*>(val))
        {
            dump(context, "DeclaredSubtypeWitness(");
            dumpType(context, declaredSubtypeWitness->sub);
            dump(context, ", ");
            dumpType(context, declaredSubtypeWitness->sup);
            dump(context, ", ");
            dumpDeclRef(context, declaredSubtypeWitness->declRef);
            dump(context, ")");
        }
        else if (auto proxyVal = dynamic_cast<IRProxyVal*>(val))
        {
            dumpOperand(context, proxyVal->inst.get());
        }
        else
        {
            dump(context, "???");
        }
    }

    static void dumpDeclRef(
        IRDumpContext*          context,
        DeclRef<Decl> const&    declRef)
    {
        auto decl = declRef.getDecl();

        auto parentDeclRef = declRef.GetParent();
        auto genericParentDeclRef = parentDeclRef.As<GenericDecl>();
        if (genericParentDeclRef)
        {
            if (genericParentDeclRef.getDecl()->inner.Ptr() == decl)
            {
                parentDeclRef = genericParentDeclRef.GetParent();
            }
            else
            {
                genericParentDeclRef = DeclRef<GenericDecl>();
            }
        }

        if(parentDeclRef.As<ModuleDecl>())
        {
            parentDeclRef = DeclRef<ContainerDecl>();
        }
        else if(parentDeclRef.As<GenericDecl>())
        {
            parentDeclRef = DeclRef<ContainerDecl>();
        }

        if(parentDeclRef)
        {
            dumpDeclRef(context, parentDeclRef);
            dump(context, ".");
        }
        dump(context, decl->getName());
        if (auto genericTypeConstraintDecl = dynamic_cast<GenericTypeConstraintDecl*>(decl))
        {
            dump(context, "{");
            dumpType(context, genericTypeConstraintDecl->sub);
            dump(context, " : ");
            dumpType(context, genericTypeConstraintDecl->sup);
            dump(context, "}");
        }
        else if (auto inheritanceDecl = dynamic_cast<InheritanceDecl*>(decl))
        {
            dump(context, "{ _ : ");
            dumpType(context, inheritanceDecl->base);
            dump(context, "}");
        }

        if(genericParentDeclRef)
        {
            auto subst = declRef.substitutions.genericSubstitutions;
            if( !subst || subst->genericDecl != genericParentDeclRef.getDecl() )
            {
                // No actual substitutions in place here
                dump(context, "<>");
            }
            else
            {
                auto args = subst->args;
                bool first = true;
                dump(context, "<");
                for(auto aa : args)
                {
                    if(!first) dump(context, ",");
                    dumpVal(context, aa);
                    first = false;
                }
                dump(context, ">");
            }
        }
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

        if(auto funcType = type->As<FuncType>())
        {
            UInt paramCount = funcType->getParamCount();
            dump(context, "(");
            for( UInt pp = 0; pp < paramCount; ++pp )
            {
                if(pp != 0) dump(context, ", ");
                dumpType(context, funcType->getParamType(pp));
            }
            dump(context, ") -> ");
            dumpType(context, funcType->getResultType());
        }
        else if(auto arrayType = type->As<ArrayExpressionType>())
        {
            dumpType(context, arrayType->baseType);
            dump(context, "[");
            if(auto elementCount = arrayType->ArrayLength)
            {
                dumpVal(context, elementCount);
            }
            dump(context, "]");
        }
        else if(auto declRefType = type->As<DeclRefType>())
        {
            dumpDeclRef(context, declRefType->declRef);
        }
        else if(auto groupSharedType = type->As<GroupSharedType>())
        {
            dump(context, "@ThreadGroup ");
            dumpType(context, groupSharedType->valueType);
        }
        else if(auto rateQualifiedType = type->As<RateQualifiedType>())
        {
            dump(context, "@");
            dumpType(context, rateQualifiedType->rate);
            dump(context, " ");
            dumpType(context, rateQualifiedType->valueType);
        }
        else if(auto constExprRate = type->As<ConstExprRate>())
        {
            dump(context, "ConstExpr");
        }
        else
        {
            // Need a default case here
            dump(context, "???");
        }

#if 0
        auto op = type->op;
        auto opInfo = kIROpInfos[op];

        switch (op)
        {
        case kIROp_StructType:
            dumpID(context, type);
            break;

        default:
            {
                dump(context, opInfo.name);
                UInt argCount = type->getArgCount();

                if (argCount > 1)
                {
                    dump(context, "<");
                    for (UInt aa = 1; aa < argCount; ++aa)
                    {
                        if (aa != 1) dump(context, ",");
                        dumpOperand(context, type->getArg(aa));

                    }
                    dump(context, ">");
                }
            }
            break;
        }
#endif
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

    static void dumpChildrenRaw(
        IRDumpContext*  context,
        IRBlock*        block)
    {
        for (auto ii = block->firstInst; ii; ii = ii->getNextInst())
        {
            dumpInst(context, ii);
        }
    }

    static void dumpBlock(
        IRDumpContext*  context,
        IRBlock*        block)
    {
        context->indent--;
        dump(context, "block ");
        dumpID(context, block);

        if( block->getFirstParam() )
        {
            dump(context, "(\n");
            context->indent += 2;
            for (auto pp = block->getFirstParam(); pp; pp = pp->getNextParam())
            {
                if (pp != block->getFirstParam())
                    dump(context, ",\n");

                dumpIndent(context);
                dump(context, "param ");
                dumpID(context, pp);
                dumpInstTypeClause(context, pp->getFullType());
            }
            context->indent -= 2;
            dump(context, ")");
        }
        dump(context, ":\n");
        context->indent++;

        dumpChildrenRaw(context, block);
    }
#if 0
    static void dumpChildrenRaw(
        IRDumpContext*  context,
        IRFunc*         func)
    {
        for (auto bb = func->getFirstBlock(); bb; bb = bb->getNextBlock())
        {
            dumpBlock(context, bb);
        }
    }

    static void dumpChildren(
        IRDumpContext*  context,
        IRFunc*         func)
    {
        dumpIndent(context);
        dump(context, "{\n");
        context->indent++;
        dumpChildrenRaw(context, func);
        context->indent--;
        dumpIndent(context);
        dump(context, "}\n");
    }
#endif
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
#if 0
        switch (op)
        {
        case kIROp_Module:
            dumpIndent(context);
            dump(context, "module\n");
            dumpChildren(context, inst);
            return;

        case kIROp_Func:
            {
                IRFunc* func = (IRFunc*)inst;
                dump(context, "\n");
                dumpIndent(context);
                dump(context, "func ");
                dumpID(context, func);
                dumpInstTypeClause(context, func->getType());
                dump(context, "\n");

                dumpIndent(context);
                dump(context, "{\n");
                context->indent++;

                for (auto bb = func->getFirstBlock(); bb; bb = bb->getNextBlock())
                {
                    if (bb != func->getFirstBlock())
                        dump(context, "\n");
                    dumpInst(context, bb);
                }

                context->indent--;
                dump(context, "}\n");
            }
            return;

        case kIROp_TypeType:
        case kIROp_Param:
        case kIROp_IntLit:
        case kIROp_FloatLit:
        case kIROp_boolConst:
            // Don't dump here
            return;

        case kIROp_Block:
            {
                IRBlock* block = (IRBlock*)inst;

                context->indent--;
                dump(context, "block ");
                dumpID(context, block);

                if( block->getFirstParam() )
                {
                    dump(context, "(");
                    context->indent++;
                    for (auto pp = block->getFirstParam(); pp; pp = pp->getNextParam())
                    {
                        if (pp != block->getFirstParam())
                            dump(context, ",\n");

                        dumpIndent(context);
                        dump(context, "param ");
                        dumpID(context, pp);
                        dumpInstTypeClause(context, pp->getType());
                    }
                    context->indent--;
                    dump(context, ")\n");
                }
                dump(context, ":\n");
                context->indent++;

                dumpChildrenRaw(context, block);
            }
            return;

        default:
            break;
        }
#endif

#if 0
        // We also want to special-case based on the *type*
        // of the instruction
        auto type = inst->getType();
        if (type && type->op == kIROp_TypeType)
        {
            // We probably don't want to print most types
            // when producing "friendly" output.
            switch (type->op)
            {
            case kIROp_StructType:
                break;

            default:
                return;
            }
        }
#endif


        // Okay, we have a seemingly "ordinary" op now
        dumpIndent(context);

        auto opInfo = &kIROpInfos[op];
        auto type = inst->getFullType();
        auto dataType = inst->getDataType();

        if (!dataType)
        {
            // No result, okay...
        }
        else
        {
            auto basicType = dataType->As<BasicExpressionType>();
            if (basicType && basicType->baseType == BaseType::Void)
            {
                // No result, okay...
            }
            else
            {
                dump(context, "let  ");
                dumpID(context, inst);
                dumpInstTypeClause(context, type);
                dump(context, "\t= ");
            }
        }

        dump(context, opInfo->name);

        uint32_t argCount = inst->argCount;
        dump(context, "(");
        for (uint32_t ii = 0; ii < argCount; ++ii)
        {
            if (ii != 0)
                dump(context, ", ");

            auto argVal = inst->getArgs()[ii].get();

            dumpOperand(context, argVal);
        }
        dump(context, ")");

        dump(context, "\n");
    }

    void dumpGenericSignature(
        IRDumpContext*  context,
        GenericDecl*    genericDecl)
    {
        for( auto pp = genericDecl->ParentDecl; pp; pp = pp->ParentDecl )
        {
            if( auto genericAncestor = dynamic_cast<GenericDecl*>(pp) )
            {
                dumpGenericSignature(context, genericAncestor);
                break;
            }
        }

        dump(context, " <");
        bool first = true;
        for (auto mm : genericDecl->Members)
        {

            if( auto typeParamDecl = mm.As<GenericTypeParamDecl>() )
            {
                if (!first) dump(context, ", ");
                dumpDeclRef(context, makeDeclRef(typeParamDecl.Ptr()));
                first = false;
            }
            else if( auto valueParamDecl = mm.As<GenericTypeParamDecl>() )
            {
                if (!first) dump(context, ", ");
                dumpDeclRef(context, makeDeclRef(valueParamDecl.Ptr()));
                first = false;
            }
        }
        first = true;
        for (auto mm : genericDecl->Members)
        {
            if( auto constraintDecl = mm.As<GenericTypeConstraintDecl>() )
            {
                if (!first) dump(context, ", ");
                else dump(context, " where ");

                dumpType(context, constraintDecl->sub);
                dump(context, " : ");
                dumpType(context, constraintDecl->sup);
                first = false;
            }
        }
        dump(context, ">");
    }

    void dumpIRFunc(
        IRDumpContext*  context,
        IRFunc*         func)
    {

        for( auto dd = func->firstDecoration; dd; dd = dd->next )
        {
            switch( dd->op )
            {
            case kIRDecorationOp_Target:
                {
                    auto decoration = (IRTargetDecoration*) dd;

                    dump(context, "\n");
                    dumpIndent(context);
                    dump(context, "[target(");
                    dump(context, decoration->targetName.Buffer());
                    dump(context, ")]");
                }
                break;

            }
        }

        dump(context, "\n");
        dumpIndent(context);
        dump(context, "ir_func ");
        dumpID(context, func);

        if (func->getGenericDecl())
        {
            dump(context, " ");
            dumpGenericSignature(context, func->getGenericDecl());
        }

        dumpInstTypeClause(context, func->getType());

        if (!func->getFirstBlock())
        {
            // Just a declaration.
            dump(context, ";\n");
            return;
        }

        dump(context, "\n");

        dumpIndent(context);
        dump(context, "{\n");
        context->indent++;

        for (auto bb = func->getFirstBlock(); bb; bb = bb->getNextBlock())
        {
            if (bb != func->getFirstBlock())
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
        dumpIRFunc(&dumpContext, func);
        auto strFunc = sbDump.ToString();
        return strFunc;
    }

    void dumpIRGlobalVar(
        IRDumpContext*  context,
        IRGlobalVar*    var)
    {
        dump(context, "\n");
        dumpIndent(context);
        dump(context, "ir_global_var ");
        dumpID(context, var);
        dumpInstTypeClause(context, var->getFullType());

        // TODO: deal with the case where a global
        // might have embedded initialization logic.

        dump(context, ";\n");
    }

    void dumpIRGlobalConstant(
        IRDumpContext*      context,
        IRGlobalConstant*   val)
    {
        dump(context, "\n");
        dumpIndent(context);
        dump(context, "ir_global_constant ");
        dumpID(context, val);
        dumpInstTypeClause(context, val->getFullType());

        // TODO: deal with the case where a global
        // might have embedded initialization logic.

        dump(context, ";\n");
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

    void dumpIRWitnessTable(
        IRDumpContext*  context,
        IRWitnessTable* witnessTable)
    {
        dump(context, "\n");
        dumpIndent(context);
        dump(context, "ir_witness_table ");
        dumpID(context, witnessTable);
        dump(context, "\n{\n");
        context->indent++;

        for (auto entry : witnessTable->entries)
        {
            dumpIRWitnessTableEntry(context, entry);
        }

        context->indent--;
        dump(context, "}\n");
    }

    void dumpIRGlobalValue(
        IRDumpContext*  context,
        IRGlobalValue*  value)
    {
        switch (value->op)
        {
        case kIROp_Func:
            dumpIRFunc(context, (IRFunc*)value);
            break;

        case kIROp_global_var:
            dumpIRGlobalVar(context, (IRGlobalVar*)value);
            break;

        case kIROp_global_constant:
            dumpIRGlobalConstant(context, (IRGlobalConstant*)value);
            break;

        case kIROp_witness_table:
            dumpIRWitnessTable(context, (IRWitnessTable*)value);
            break;

        default:
            dump(context, "???\n");
            break;
        }
    }

    void dumpIRModule(
        IRDumpContext*  context,
        IRModule*       module)
    {
        for( auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue() )
        {
            dumpIRGlobalValue(context, gv);
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

        dumpIRGlobalValue(&context, globalVal);

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

    Type* IRValue::getRate()
    {
        if(auto rateQualifiedType = type->As<RateQualifiedType>())
            return rateQualifiedType->rate;

        return nullptr;
    }

    Type* IRValue::getDataType()
    {
        if(auto rateQualifiedType = type->As<RateQualifiedType>())
            return rateQualifiedType->valueType;

        return type;
    }

    void IRValue::replaceUsesWith(IRValue* other)
    {
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
            assert(uu->get() == this);

            // Swap this use over to use the other value.
            uu->usedValue = other;

            // Try to move to the next use, but bail
            // out if we are at the last one.
            IRUse* next = uu->nextUse;
            if( !next )
                break;

            uu = next;
        }

        // We are at the last use (and there must
        // be at least one, because we handled
        // the case of an empty list earlier).
        assert(uu);

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

    void IRValue::deallocate()
    {
#if 0
        // I'm going to intentionally leak here,
        // just to test that this is the source
        // of my heap-corruption crashes.
#else
        // Run destructor to be sure...
        this->~IRValue();
#endif
    }

    void IRValue::dispose()
    {
        IRObject::dispose();
        type = decltype(type)();
    }

    // Insert this instruction into the same basic block
    // as `other`, right before it.
    void IRInst::insertBefore(IRInst* other)
    {
        // Make sure this instruction has been removed from any previous parent
        this->removeFromParent();

        auto bb = other->getParentBlock();
        assert(bb);

        auto pp = other->getPrevInst();
        if( pp )
        {
            pp->next = this;
        }
        else
        {
            bb->firstInst = this;
        }

        this->prev = pp;
        this->next = other;
        this->parent = bb;

        other->prev = this;
    }

    // Remove this instruction from its parent block,
    // and then destroy it (it had better have no uses!)
    void IRInst::removeFromParent()
    {
        // If we don't currently have a parent, then
        // we are doing fine.
        if(!getParentBlock())
            return;

        auto bb = getParentBlock();
        auto pp = getPrevInst();
        auto nn = getNextInst();

        if(pp)
        {
            SLANG_ASSERT(pp->getParentBlock() == bb);
            pp->next = nn;
        }
        else
        {
            bb->firstInst = nn;
        }

        if(nn)
        {
            SLANG_ASSERT(nn->getParentBlock() == bb);
            nn->prev = pp;
        }
        else
        {
            bb->lastInst = pp;
        }

        prev = nullptr;
        next = nullptr;
        parent = nullptr;
    }

    void IRInst::removeArguments()
    {
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            IRUse& use = getArgs()[aa];
            use.clear();
        }
    }

    // Remove this instruction from its parent block,
    // and then destroy it (it had better have no uses!)
    void IRInst::removeAndDeallocate()
    {
        removeFromParent();
        removeArguments();
        deallocate();
    }


    //
    // Legalization of entry points for GLSL:
    //

    IRGlobalVar* addGlobalVariable(
        IRModule*   module,
        Type*       valueType)
    {
        auto session = module->session;

        SharedIRBuilder shared;
        shared.module = module;
        shared.session = session;

        IRBuilder builder;
        builder.sharedBuilder = &shared;

        RefPtr<PtrType> ptrType = session->getPtrType(valueType);

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

            // A simple `IRValue*` that represents the actual value
            value,

            // An `IRValue*` that represents the address of the actual value
            address,

            // A `TupleValImpl` that represents zero or more `ScalarizedVal`s
            tuple,

            // A `TypeAdapterValImpl` that wraps a single `ScalarizedVal` and
            // represents an implicit type conversion applied to it on read
            // or write.
            typeAdapter,
        };

        // Create a value representing a simple value
        static ScalarizedVal value(IRValue* irValue)
        {
            ScalarizedVal result;
            result.flavor = Flavor::value;
            result.irValue = irValue;
            return result;
        }


        // Create a value representing an address
        static ScalarizedVal address(IRValue* irValue)
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
        IRValue*                    irValue = nullptr;
        RefPtr<ScalarizedValImpl>   impl;
    };

    // This is the case for a value that is a "tuple" of other values
    struct ScalarizedTupleValImpl : ScalarizedValImpl
    {
        struct Element
        {
            ScalarizedVal   val;
            DeclRef<Decl>   declRef;
        };

        RefPtr<Type>    type;
        List<Element>   elements;
    };

    // This is the case for a value that is stored with one type,
    // but needs to present itself as having a different type
    struct ScalarizedTypeAdapterValImpl : ScalarizedValImpl
    {
        ScalarizedVal   val;
        RefPtr<Type>    actualType;   // the actual type of `val`
        RefPtr<Type>    pretendType;     // the type this value pretends to have
    };

    struct GlobalVaryingDeclarator
    {
        enum class Flavor
        {
            array,
        };

        Flavor                      flavor;
        IntVal*                     elementCount;
        GlobalVaryingDeclarator*    next;
    };

    struct GLSLSystemValueInfo
    {
        // The name of the built-in GLSL variable
        char const*     name;

        // The required type of the built-in variable
        RefPtr<Type>    requiredType;
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
    };

    GLSLSystemValueInfo* getGLSLSystemValueInfo(
        GLSLLegalizationContext*    context,
        VarLayout*                  varLayout,
        LayoutResourceKind          kind,
        GLSLSystemValueInfo*        inStorage)
    {
        char const* name = nullptr;

        auto semanticNameSpelling = varLayout->systemValueSemantic;
        if(semanticNameSpelling.Length() == 0)
            return nullptr;

        auto semanticName = semanticNameSpelling.ToLower();

        RefPtr<Type> requiredType;

        if(semanticName == "sv_position")
        {
            // TODO: need to pick between `gl_Position` and
            // `gl_FragCoord` based on whether this is an input
            // or an output.
            if( kind == LayoutResourceKind::VaryingOutput )
            {
                name = "gl_Position";
            }
            else
            {
                name = "gl_FragCoord";
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
            requiredType = context->session->getBuiltinType(BaseType::Int);
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
            inStorage->requiredType = requiredType;
            return inStorage;
        }

        context->getSink()->diagnose(varLayout->varDecl.getDecl()->loc, Diagnostics::unknownSystemValueSemantic, semanticNameSpelling);
        return nullptr;
    }

    ScalarizedVal createSimpleGLSLGlobalVarying(
        GLSLLegalizationContext*    context,
        IRBuilder*                  builder,
        Type*                       inType,
        VarLayout*                  inVarLayout,
        TypeLayout*                 inTypeLayout,
        LayoutResourceKind          kind,
        UInt                        bindingIndex,
        GlobalVaryingDeclarator*    declarator)
    {
        // Check if we have a system value on our hands.
        GLSLSystemValueInfo systemValueInfoStorage;
        auto systemValueInfo = getGLSLSystemValueInfo(
            context,
            inVarLayout,
            kind,
            &systemValueInfoStorage);

        RefPtr<Type> type = inType;

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
            assert(dd->flavor == GlobalVaryingDeclarator::Flavor::array);

            RefPtr<ArrayExpressionType> arrayType = builder->getSession()->getArrayType(
                type,
                dd->elementCount);

            RefPtr<ArrayTypeLayout> arrayTypeLayout = new ArrayTypeLayout();
            arrayTypeLayout->type = arrayType;
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

                if( !fromType->Equals(toType) )
                {
                    RefPtr<ScalarizedTypeAdapterValImpl> typeAdapter = new ScalarizedTypeAdapterValImpl;
                    typeAdapter->actualType = systemValueInfo->requiredType;
                    typeAdapter->pretendType = inType;
                    typeAdapter->val = val;

                    val = ScalarizedVal::typeAdapter(typeAdapter);
                }
            }
        }

        builder->addLayoutDecoration(globalVariable, varLayout);

        return val;
    }

    ScalarizedVal createGLSLGlobalVaryingsImpl(
        GLSLLegalizationContext*    context,
        IRBuilder*                  builder,
        Type*                       type,
        VarLayout*                  varLayout,
        TypeLayout*                 typeLayout,
        LayoutResourceKind          kind,
        UInt                        bindingIndex,
        GlobalVaryingDeclarator*    declarator)
    {
        if( type->As<BasicExpressionType>() )
        {
            return createSimpleGLSLGlobalVarying(
                context,
                builder, type, varLayout, typeLayout, kind, bindingIndex, declarator);
        }
        else if( type->As<VectorExpressionType>() )
        {
            return createSimpleGLSLGlobalVarying(
                context,
                builder, type, varLayout, typeLayout, kind, bindingIndex, declarator);
        }
        else if( type->As<MatrixExpressionType>() )
        {
            // TODO: a matrix-type varying should probably be handled like an array of rows
            return createSimpleGLSLGlobalVarying(
                context,
                builder, type, varLayout, typeLayout, kind, bindingIndex, declarator);
        }
        else if( auto arrayType = type->As<ArrayExpressionType>() )
        {
            // We will need to SOA-ize any nested types.

            auto elementType = arrayType->baseType;
            auto elementCount = arrayType->ArrayLength;
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
                bindingIndex,
                &arrayDeclarator);
        }
        else if( auto streamType = type->As<HLSLStreamOutputType>() )
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
                bindingIndex,
                declarator);
        }
        else if( auto declRefType = type->As<DeclRefType>() )
        {
            auto declRef = declRefType->declRef;
            if( auto structDeclRef = declRef.As<StructDecl>() )
            {
                // This is either a user-defined struct, or a builtin type.
                // TODO: exclude resource types here.

                // We need to recurse down into the individual fields,
                // and generate a variable for each of them.

                // Note: we can use the presence of a `StructTypeLayout` as
                // a quick way to reject a bunch of types that aren't actually `struct`s
                auto structTypeLayout = dynamic_cast<StructTypeLayout*>(typeLayout);
                if( structTypeLayout )
                {
                    RefPtr<ScalarizedTupleValImpl> tupleValImpl = new ScalarizedTupleValImpl();


                    // Construct the actual type for the tuple (including any outer arrays)
                    RefPtr<Type> fullType = type;
                    for( auto dd = declarator; dd; dd = dd->next )
                    {
                        assert(dd->flavor == GlobalVaryingDeclarator::Flavor::array);
                        fullType = builder->getSession()->getArrayType(
                            fullType,
                            dd->elementCount);
                    }

                    tupleValImpl->type = fullType;

                    // Okay, we want to walk through the fields here, and
                    // generate one variable for each.
                    for( auto ff : structTypeLayout->fields )
                    {
                        UInt fieldBindingIndex = bindingIndex;
                        if(auto fieldResInfo = ff->FindResourceInfo(kind))
                            fieldBindingIndex += fieldResInfo->index;

                        auto fieldVal = createGLSLGlobalVaryingsImpl(
                            context,
                            builder,
                            ff->typeLayout->type,
                            ff,
                            ff->typeLayout,
                            kind,
                            fieldBindingIndex,
                            declarator);

                        ScalarizedTupleValImpl::Element element;
                        element.val = fieldVal;
                        element.declRef = ff->varDecl;

                        tupleValImpl->elements.Add(element);
                    }

                    return ScalarizedVal::tuple(tupleValImpl);
                }
            }
        }

        // Default case is to fall back on the simple behavior
        return createSimpleGLSLGlobalVarying(
            context,
            builder, type, varLayout, typeLayout, kind, bindingIndex, declarator);
    }

    ScalarizedVal createGLSLGlobalVaryings(
        GLSLLegalizationContext*    context,
        IRBuilder*                  builder,
        Type*                       type,
        VarLayout*                  layout,
        LayoutResourceKind          kind)
    {
        UInt bindingIndex = 0;
        if(auto rr = layout->FindResourceInfo(kind))
            bindingIndex = rr->index;
        return createGLSLGlobalVaryingsImpl(
            context,
            builder, type, layout, layout->typeLayout, kind, bindingIndex, nullptr);
    }

    ScalarizedVal extractField(
        IRBuilder*              builder,
        ScalarizedVal const&    val,
        UInt                    fieldIndex,
        DeclRef<Decl>           fieldDeclRef)
    {
        switch( val.flavor )
        {
        case ScalarizedVal::Flavor::value:
            return ScalarizedVal::value(
                builder->emitFieldExtract(
                    GetType(fieldDeclRef.As<VarDeclBase>()),
                    val.irValue,
                    builder->getDeclRefVal(fieldDeclRef)));

        case ScalarizedVal::Flavor::address:
            return ScalarizedVal::address(
                builder->emitFieldAddress(
                    GetType(fieldDeclRef.As<VarDeclBase>()),
                    val.irValue,
                    builder->getDeclRefVal(fieldDeclRef)));

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
        IRValue*                val,
        Type*                   toType,
        Type*                   /*fromType*/)
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
        Type*                   toType,
        Type*                   fromType)
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
                            rightElement.declRef);
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
                        leftTupleVal->elements[ee].declRef);
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
        Type*           elementType,
        ScalarizedVal   val,
        IRValue*        indexVal)
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
                    builder->getSession()->getPtrType(elementType),
                    val.irValue,
                    indexVal));

        case ScalarizedVal::Flavor::tuple:
            {
                auto inputTuple = val.impl.As<ScalarizedTupleValImpl>();

                RefPtr<ScalarizedTupleValImpl> resultTuple = new ScalarizedTupleValImpl();
                resultTuple->type = elementType;

                UInt elementCount = inputTuple->elements.Count();
                UInt elementCounter = 0;

                auto declRefType = dynamic_cast<DeclRefType*>(elementType);
                SLANG_RELEASE_ASSERT(declRefType);

                auto aggTypeDeclRef = declRefType->declRef.As<AggTypeDecl>();
                SLANG_RELEASE_ASSERT(aggTypeDeclRef);

                for(auto fieldDeclRef : getMembersOfType<StructField>(aggTypeDeclRef))
                {
                    if(fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
                        continue;

                    auto tupleElementType = GetType(fieldDeclRef);

                    UInt elementIndex = elementCounter++;

                    SLANG_RELEASE_ASSERT(elementIndex < elementCount);
                    auto inputElement = inputTuple->elements[elementIndex];

                    ScalarizedTupleValImpl::Element resultElement;
                    resultElement.declRef = inputElement.declRef;
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
        Type*           elementType,
        ScalarizedVal   val,
        UInt            index)
    {
        return getSubscriptVal(
            builder,
            elementType,
            val,
            builder->getIntValue(
                builder->getSession()->getIntType(),
                index));
    }

    IRValue* materializeValue(
        IRBuilder*              builder,
        ScalarizedVal const&    val);

    IRValue* materializeTupleValue(
        IRBuilder*      builder,
        ScalarizedVal   val)
    {
        auto tupleVal = val.impl.As<ScalarizedTupleValImpl>();
        assert(tupleVal);

        UInt elementCount = tupleVal->elements.Count();
        auto type = tupleVal->type;

        if( auto arrayType = type.As<ArrayExpressionType>() )
        {
            // The tuple represent an array, which means that the
            // individual elements are expected to yield arrays as well.
            //
            // We will extract a value for each array element, and
            // then use these to construct our result.

            List<IRValue*> arrayElementVals;
            UInt arrayElementCount = (UInt) GetIntVal(arrayType->ArrayLength);

            for( UInt ii = 0; ii < arrayElementCount; ++ii )
            {
                auto arrayElementPseudoVal = getSubscriptVal(
                    builder,
                    arrayType->baseType,
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

            List<IRValue*> elementVals;
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

    IRValue* materializeValue(
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
        IRValue*        val,
        String const&   targetName)
    {
        for( auto dd = val->firstDecoration; dd; dd = dd->next )
        {
            if(dd->op != kIRDecorationOp_TargetIntrinsic)
                continue;

            auto decoration = (IRTargetIntrinsicDecoration*) dd;
            if(decoration->targetName == targetName)
                return decoration;
        }

        return nullptr;
    }

    void legalizeEntryPointForGLSL(
        Session*                session,
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

        auto module = func->parentModule;

        // We require that the entry-point function has no uses,
        // because otherwise we'd invalidate the signature
        // at all existing call sites.
        //
        // TODO: the right thing to do here is to split any
        // function that both gets called as an entry point
        // and as an ordinary function.
        assert(!func->firstUse);

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
        builder.curFunc = func;

        // We will start by looking at the return type of the
        // function, because that will enable us to do an
        // early-out check to avoid more work.
        //
        // Specifically, we need to check if the function has
        // a `void` return type, because there is no work
        // to be done on its return value in that case.
        auto resultType = func->getResultType();
        if( resultType->Equals(session->getVoidType()) )
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
                LayoutResourceKind::FragmentOutput);

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
                    IRValue* returnValue = returnInst->getVal();

                    // Make sure we add these instructions to the right block
                    builder.curBlock = bb;

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
            UInt paramCounter = 0;
            for( auto pp = firstBlock->getFirstParam(); pp; pp = pp->getNextParam() )
            {
                UInt paramIndex = paramCounter++;

                // We assume that the entry-point layout includes information
                // on each parameter, and that these arrays are kept aligned.
                // Note that this means that any transformations that mess
                // with function signatures will need to also update layout info...
                //
                assert(entryPointLayout->fields.Count() > paramIndex);
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

                // Any initialization code we insert nees to be at the start
                // of the block:
                builder.curBlock = firstBlock;
                builder.insertBeforeInst = firstBlock->getFirstInst();

                // First we will special-case stage input/outputs that
                // don't fit into the standard varying model.
                // For right now we are only doing special-case handling
                // of geometry shader output streams.
                if( auto paramPtrType = paramType->As<OutTypeBase>() )
                {
                    auto valueType = paramPtrType->getValueType();
                    if( auto gsStreamType = valueType->As<HLSLStreamOutputType>() )
                    {
                        // An output stream type like `TriangleStream<Foo>` should
                        // more or less translate into `out Foo` (plus scalarization).

                        auto globalOutputVal = createGLSLGlobalVaryings(
                            &context,
                            &builder,
                            valueType,
                            paramLayout,
                            LayoutResourceKind::VaryingOutput);

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
                                auto callee = ii->getArg(0);
                                while( callee->op == kIROp_specialize )
                                {
                                    callee = ((IRSpecialize*) callee)->getArg(0);
                                }
                                if(callee->op != kIROp_Func)
                                    continue;

                                // HACK: we will identify the operation based
                                // on the target-intrinsic definition that was
                                // given to it.
                                auto decoration = findTargetIntrinsicDecoration(callee, "glsl");
                                if(!decoration)
                                    continue;

                                if(decoration->definition != "EmitVertex()")
                                {
                                    continue;
                                }

                                // Okay, we have a declaration, and we want to modify it!

                                builder.curBlock = bb;
                                builder.insertBeforeInst = ii;

                                assign(&builder, globalOutputVal, ScalarizedVal::value(ii->getArg(2)));
                            }
                        }

                        continue;
                    }
                }


                // Is the parameter type a special pointer type
                // that indicates the parameter is used for `out`
                // or `inout` access?
                if(auto paramPtrType = paramType->As<OutTypeBase>() )
                {
                    // Okay, we have the more interesting case here,
                    // where the parameter was being passed by reference.
                    // We are going to create a local variable of the appropriate
                    // type, which will replace the parameter, along with
                    // one or more global variables for the actual input/output.

                    auto valueType = paramPtrType->getValueType();

                    auto localVariable = builder.emitVar(valueType);
                    auto localVal = ScalarizedVal::address(localVariable);

                    if( auto inOutType = paramPtrType->As<InOutType>() )
                    {
                        // In the `in out` case we need to declare two
                        // sets of global variables: one for the `in`
                        // side and one for the `out` side.
                        auto globalInputVal = createGLSLGlobalVaryings(
                            &context,
                            &builder, valueType, paramLayout, LayoutResourceKind::VertexInput);

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
                            &builder, valueType, paramLayout, LayoutResourceKind::FragmentOutput);

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

                        builder.curBlock = bb;
                        builder.insertBeforeInst = terminatorInst;

                        assign(&builder, globalOutputVal, localVal);
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
                        &builder, paramType, paramLayout, LayoutResourceKind::VaryingInput);

                    // Next we need to replace uses of the parameter with
                    // references to the variable(s). We are going to do that
                    // somewhat naively, by simply materializing the
                    // variables at the start.
                    IRValue* materialized = materializeValue(&builder, globalValue);

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
                pp->deallocate();
                pp = next;
            }
            firstBlock->firstParam = nullptr;
            firstBlock->lastParam = nullptr;
        }

        // Finally, we need to patch up the type of the entry point,
        // because it is no longer accurate.

        RefPtr<FuncType> voidFuncType = new FuncType();
        voidFuncType->setSession(session);
        voidFuncType->resultType = session->getVoidType();
        func->type = voidFuncType;

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

        // A map from values in the original IR module
        // to their equivalent in the cloned module.
        typedef Dictionary<IRValue*, IRValue*> ClonedValueDictionary;
        ClonedValueDictionary clonedValues;

        SharedIRBuilder sharedBuilderStorage;
        IRBuilder builderStorage;

        // Non-generic functions to be processed (for generic specialization context)
        List<IRFunc*> workList;
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

        IRSharedSpecContext::ClonedValueDictionary& getClonedValues() { return getShared()->clonedValues; }

        // The IR builder to use for creating nodes
        IRBuilder*  builder;

        SubstitutionSet subst;

        // A callback to be used when a value that is not registerd in `clonedValues`
        // is needed during cloning. This gives the subtype a chance to intercept
        // the operation and clone (or not) as needed.
        virtual IRValue* maybeCloneValue(IRValue* originalVal)
        {
            return originalVal;
        }

        // A callback used to clone (or not) types.
        virtual RefPtr<Type> maybeCloneType(Type* originalType)
        {
            return originalType;
        }

        // A callback used to clone (or not) a declaration reference
        virtual DeclRef<Decl> maybeCloneDeclRef(DeclRef<Decl> const& declRef)
        {
            return declRef;
        }

        // A callback used to clone (or not) a Val
        virtual RefPtr<Val> maybeCloneVal(Val* val)
        {
            return val;
        }
    };

    void registerClonedValue(
        IRSpecContextBase*  context,
        IRValue*    clonedValue,
        IRValue*    originalValue)
    {
        if(!originalValue)
            return;

        // Note: setting the entry direclty here rather than
        // using `Add` or `AddIfNotExists` because we can conceivably
        // clone the same value (e.g., a basic block inside a generic
        // function) multiple times, and that is okay, and we really
        // just need to keep track of the most recent value.

        // TODO: The same thing could potentially be handled more
        // cleanly by having a notion of scoping for these cloned-value
        // mappings, so that we register cloned values for things
        // inside of a function to a temporary mapping that we
        // throw away after the function is done.

        context->getClonedValues()[originalValue] = clonedValue;
    }

    // Information on values to use when registering a cloned value
    struct IROriginalValuesForClone
    {
        IRValue*        originalVal = nullptr;
        IRSpecSymbol*   sym = nullptr;

        IROriginalValuesForClone() {}

        IROriginalValuesForClone(IRValue* originalValue)
            : originalVal(originalValue)
        {}

        IROriginalValuesForClone(IRSpecSymbol* symbol)
            : sym(symbol)
        {}
    };

    void registerClonedValue(
        IRSpecContextBase*              context,
        IRValue*                        clonedValue,
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
        IRValue*        clonedValue,
        IRValue*        originalValue)
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

            default:
                // Don't clone any decorations we don't understand.
                break;
            }
        }

        // We will also clone the location here, just because this is a convenient bottleneck
        clonedValue->sourceLoc = originalValue->sourceLoc;
    }

    struct IRSpecContext : IRSpecContextBase
    {
        // Override the "maybe clone" logic so that we always clone
        virtual IRValue* maybeCloneValue(IRValue* originalVal) override;

        // Override teh "maybe clone" logic so that we carefully
        // clone any IR proxy values inside substitutions
        virtual DeclRef<Decl> maybeCloneDeclRef(DeclRef<Decl> const& declRef) override;

        virtual RefPtr<Type> maybeCloneType(Type* originalType) override;
        virtual RefPtr<Val> maybeCloneVal(Val* val) override;
    };


    IRGlobalValue* cloneGlobalValue(IRSpecContext* context, IRGlobalValue* originalVal);
    RefPtr<Substitutions> cloneSubstitutions(
        IRSpecContext*  context,
        Substitutions*  subst);

    RefPtr<Type> IRSpecContext::maybeCloneType(Type* originalType)
    {
        return originalType->Substitute(subst).As<Type>();
    }

    RefPtr<Val> IRSpecContext::maybeCloneVal(Val * val)
    {
        return val->Substitute(subst);
    }


    IRValue* IRSpecContext::maybeCloneValue(IRValue* originalValue)
    {
        switch (originalValue->op)
        {
        case kIROp_global_var:
        case kIROp_global_constant:
        case kIROp_Func:
        case kIROp_witness_table:
            return cloneGlobalValue(this, (IRGlobalValue*) originalValue);

        case kIROp_boolConst:
            {
                IRConstant* c = (IRConstant*)originalValue;
                return builder->getBoolValue(c->u.intVal != 0);
            }
            break;


        case kIROp_IntLit:
            {
                IRConstant* c = (IRConstant*)originalValue;
                return builder->getIntValue(c->type, c->u.intVal);
            }
            break;

        case kIROp_FloatLit:
            {
                IRConstant* c = (IRConstant*)originalValue;
                return builder->getFloatValue(c->type, c->u.floatVal);
            }
            break;

        case kIROp_decl_ref:
            {
                IRDeclRef* od = (IRDeclRef*)originalValue;
                auto newDeclRef = od->declRef;

                // if the declRef is one of the __generic_param decl being substituted by subst
                // return the substituted decl
                if (subst.globalGenParamSubstitutions)
                {
                    int diff = 0;
                    newDeclRef = od->declRef.SubstituteImpl(subst, &diff);
                    for (auto globalGenSubst = subst.globalGenParamSubstitutions; globalGenSubst; globalGenSubst = globalGenSubst->outer)
                    {
                        if (!globalGenSubst)
                            continue;
                        if (newDeclRef.getDecl() == globalGenSubst->paramDecl)
                            return builder->getTypeVal(globalGenSubst->actualType.As<Type>());
                        else if (auto genConstraint = newDeclRef.As<GenericTypeConstraintDecl>())
                        {
                            // a decl-ref to GenericTypeConstraintDecl as a result of
                            // referencing a generic parameter type should be replaced with
                            // the actual witness table
                            if (genConstraint.getDecl()->ParentDecl == globalGenSubst->paramDecl)
                            {
                                // find the witness table from subst
                                for (auto witness : globalGenSubst->witnessTables)
                                {
                                    if (witness.Key->EqualsVal(GetSup(genConstraint)))
                                    {
                                        auto proxyVal = witness.Value.As<IRProxyVal>();
                                        SLANG_ASSERT(proxyVal);
                                        return proxyVal->inst.get();
                                    }
                                }
                            }
                        }
                    }
                }
                auto declRef = maybeCloneDeclRef(newDeclRef);
                return builder->getDeclRefVal(declRef);
            }
            break;
        case kIROp_TypeType:
            {
                IRValue* od = (IRValue*)originalValue;
                int ioDiff = 0;
                auto newType = od->type->SubstituteImpl(subst, &ioDiff);
                return builder->getTypeVal(newType.As<Type>());
            }
            break;
        default:
            SLANG_UNEXPECTED("no value registered for IR value");
            UNREACHABLE_RETURN(nullptr);
        }
    }

    IRValue* cloneValue(
        IRSpecContextBase*  context,
        IRValue*        originalValue);

    RefPtr<Val> cloneSubstitutionArg(
        IRSpecContext*  context,
        Val*            val)
    {
        if (auto proxyVal = dynamic_cast<IRProxyVal*>(val))
        {
            auto newIRVal = cloneValue(context, proxyVal->inst.get());

            RefPtr<IRProxyVal> newProxyVal = new IRProxyVal();
            newProxyVal->inst.init(nullptr, newIRVal);
            return newProxyVal;
        }
        else if (auto type = dynamic_cast<Type*>(val))
        {
            return context->maybeCloneType(type);
        }
        else
        {
            return context->maybeCloneVal(val);
        }
    }

    RefPtr<GenericSubstitution> cloneGenericSubst(IRSpecContext*  context, GenericSubstitution* genSubst)
    {
        if (!genSubst)
            return nullptr;

        RefPtr<GenericSubstitution> newSubst = new GenericSubstitution();
        newSubst->outer = cloneGenericSubst(context, genSubst->outer);
        newSubst->genericDecl = genSubst->genericDecl;

        for (auto arg : genSubst->args)
        {
            auto newArg = cloneSubstitutionArg(context, arg);
            newSubst->args.Add(newArg);
        }
        return newSubst;
    }

    RefPtr<GlobalGenericParamSubstitution> cloneGlobalGenericSubst(IRSpecContext* context, GlobalGenericParamSubstitution* subst)
    {
        if (!subst)
            return nullptr;
        auto newSubst = new GlobalGenericParamSubstitution();
        newSubst->actualType = subst->actualType;
        newSubst->paramDecl = subst->paramDecl;
        newSubst->witnessTables = subst->witnessTables;
        newSubst->outer = cloneGlobalGenericSubst(context, subst->outer);
        return newSubst;
    }

    SubstitutionSet cloneSubstitutions(
        IRSpecContext*  context,
        SubstitutionSet  subst)
    {
        SubstitutionSet rs;
        if (!subst)
            return rs;
        rs.genericSubstitutions = cloneGenericSubst(context, subst.genericSubstitutions);
        rs.globalGenParamSubstitutions = cloneGlobalGenericSubst(context, subst.globalGenParamSubstitutions);
        if (auto thisSubst = subst.thisTypeSubstitution)
        {
            RefPtr<ThisTypeSubstitution> newSubst = new ThisTypeSubstitution();
            newSubst->sourceType = thisSubst->sourceType;
            rs.thisTypeSubstitution = newSubst;
        }
        return rs;
    }

    DeclRef<Decl> IRSpecContext::maybeCloneDeclRef(DeclRef<Decl> const& declRef)
    {
        // Un-specialized decl? Nothing to do.
        if (!declRef.substitutions)
            return declRef;

        DeclRef<Decl> newDeclRef = declRef;

        // Scan through substitutions and clone as needed.
        //
        // TODO: this is wasteful since we clone *everything*
        newDeclRef.substitutions = cloneSubstitutions(this, declRef.substitutions);

        return newDeclRef;
    }

    IRValue* cloneValue(
        IRSpecContextBase*  context,
        IRValue*        originalValue)
    {
        IRValue* clonedValue = nullptr;
        if (context->getClonedValues().TryGetValue(originalValue, clonedValue))
        {
            return clonedValue;
        }

        return context->maybeCloneValue(originalValue);
    }

    IRValue* maybeCloneValueWithMangledName(
        IRSpecContextBase*  context,
        IRGlobalValue*      originalValue)
    {
        for (auto gv = context->shared->module->firstGlobalValue; gv; gv = gv->nextGlobalValue)
        {
            if (gv->mangledName == originalValue->mangledName)
                return gv;
        }
        return cloneValue(context, originalValue);
    }
        
    void cloneInst(
        IRSpecContextBase*  context,
        IRBuilder*      builder,
        IRInst*         originalInst)
    {
        switch (originalInst->op)
        {
        // TODO: are there any instruction types that need to be handled
        // specially here? That would be anything that has more state
        // than is visible in its operand list...
        case 0: // nothing yet
        default:
            {
                // The common case is that we just need to construct a cloned
                // instruction with the right number of operands, intialize
                // it, and then add it to the sequence.
                UInt argCount = originalInst->getArgCount();
                IRInst* clonedInst = createInstWithTrailingArgs<IRInst>(
                    builder, originalInst->op,
                    context->maybeCloneType(originalInst->type),
                    0, nullptr,
                    argCount, nullptr);
                registerClonedValue(context, clonedInst, originalInst);
                auto oldBuilder = context->builder;
                context->builder = builder;
                for (UInt aa = 0; aa < argCount; ++aa)
                {
                    IRValue* originalArg = originalInst->getArg(aa);
                    IRValue* clonedArg;
                    if (originalArg->op == kIROp_witness_table)
                        clonedArg = cloneGlobalValueWithMangledName((IRSpecContext*)context, 
                        ((IRGlobalValue*)originalArg)->mangledName, (IRGlobalValue*)originalArg);
                    else
                        clonedArg = cloneValue(context, originalArg);
                    clonedInst->getArgs()[aa].init(clonedInst, clonedArg);
                }
                builder->addInst(clonedInst);
                context->builder = oldBuilder;
                cloneDecorations(context, clonedInst, originalInst);
            }

            break;
        }
    }

    void cloneGlobalValueWithCodeCommon(
        IRSpecContextBase*      context,
        IRGlobalValueWithCode*  clonedValue,
        IRGlobalValueWithCode*  originalValue);

    IRGlobalVar* cloneGlobalVarImpl(
        IRSpecContext*  context,
        IRGlobalVar*    originalVar,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedVar = context->builder->createGlobalVar(
            context->maybeCloneType(originalVar->getDataType()->getValueType()));

        if(auto rate = originalVar->getRate() )
        {
            clonedVar->type = context->builder->getSession()->getRateQualifiedType(
                rate, clonedVar->type);
        }

        registerClonedValue(context, clonedVar, originalValues);

        auto mangledName = originalVar->mangledName;
        clonedVar->mangledName = mangledName;

        cloneDecorations(context, clonedVar, originalVar);

        VarLayout* layout = nullptr;
        if (context->globalVarLayouts.TryGetValue(mangledName, layout))
        {
            context->builder->addLayoutDecoration(clonedVar, layout);
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
        IRSpecContext*      context,
        IRGlobalConstant*    originalVal,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedVal = context->builder->createGlobalConstant(context->maybeCloneType(originalVal->getFullType()));
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

    IRWitnessTable* cloneWitnessTableImpl(
        IRSpecContextBase*  context,
        IRWitnessTable* originalTable,
        IROriginalValuesForClone const& originalValues,
        IRWitnessTable* dstTable = nullptr,
        bool registerValue = true)
    {
        auto clonedTable = dstTable ? dstTable : context->builder->createWitnessTable();
        if (registerValue)
            registerClonedValue(context, clonedTable, originalValues);

        auto mangledName = originalTable->mangledName;
       
        clonedTable->mangledName = mangledName;
        clonedTable->genericDecl = originalTable->genericDecl;
        clonedTable->subTypeDeclRef = originalTable->subTypeDeclRef;
        clonedTable->supTypeDeclRef = originalTable->supTypeDeclRef;
        cloneDecorations(context, clonedTable, originalTable);

        // Clone the entries in the witness table as well
        for( auto originalEntry : originalTable->entries )
        {
            auto clonedKey = cloneValue(context, originalEntry->requirementKey.get());
            
            // if a global val with the mangled name already exists, don't clone again
            auto clonedVal = maybeCloneValueWithMangledName(context, (IRGlobalValue*)(originalEntry->satisfyingVal.get()));

            /*auto clonedEntry = */context->builder->createWitnessTableEntry(
                clonedTable,
                clonedKey,
                clonedVal);
        }

        return clonedTable;
    }

    IRWitnessTable* cloneWitnessTableWithoutRegistering(
        IRSpecContextBase*  context,
        IRWitnessTable* originalTable,
        IRWitnessTable* dstTable = nullptr)
    {
        return cloneWitnessTableImpl(context, originalTable, IROriginalValuesForClone(), dstTable, false);
    }

    void cloneGlobalValueWithCodeCommon(
        IRSpecContextBase*      context,
        IRGlobalValueWithCode*  clonedValue,
        IRGlobalValueWithCode*  originalValue)
    {
        // Next we are going to clone the actual code.
        IRBuilder builderStorage = *context->builder;
        IRBuilder* builder = &builderStorage;
        builder->curFunc = clonedValue;


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
        }

        // Okay, now we are in a good position to start cloning
        // the instructions inside the blocks.
        {
            IRBlock* ob = originalValue->getFirstBlock();
            IRBlock* cb = clonedValue->getFirstBlock();
            while (ob)
            {
                assert(cb);

                builder->curBlock = cb;
                for (auto oi = ob->getFirstInst(); oi; oi = oi->getNextInst())
                {
                    cloneInst(context, builder, oi);
                }

                ob = ob->getNextBlock();
                cb = cb->getNextBlock();
            }
        }

    }

    void cloneFunctionCommon(
        IRSpecContextBase*  context,
        IRFunc*         clonedFunc,
        IRFunc*         originalFunc)
    {
        // First clone all the simple properties.
        clonedFunc->mangledName = originalFunc->mangledName;
        clonedFunc->genericDecls = originalFunc->genericDecls;
        clonedFunc->specializedGenericLevel = originalFunc->specializedGenericLevel;
        clonedFunc->type = context->maybeCloneType(originalFunc->type);

        cloneDecorations(context, clonedFunc, originalFunc);

        cloneGlobalValueWithCodeCommon(
            context,
            clonedFunc,
            originalFunc);

        // Shuffle the function to the end of the list, because
        // it needs to follow its dependencies.
        //
        // TODO: This isn't really a good requirement to place on the IR...
        clonedFunc->removeFromParent();
        clonedFunc->insertAtEnd(context->getModule());
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
        cloneFunctionCommon(context, clonedFunc, originalFunc);
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
            if(decoration->targetName == targetName)
                return TargetSpecializationLevel::specializedForTarget;

            result = TargetSpecializationLevel::specializedForOtherTarget;
        }

        return result;
    }

    bool isDefinition(
        IRGlobalValue* val)
    {
        switch (val->op)
        {
        case kIROp_witness_table:
            return ((IRWitnessTable*)val)->entries.first != nullptr;

        case kIROp_global_var:
        case kIROp_global_constant:
        case kIROp_Func:
            return ((IRGlobalValueWithCode*)val)->firstBlock != nullptr;

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
        IRSpecContext*  context,
        IRFunc*         originalFunc,
        IROriginalValuesForClone const& originalValues)
    {
        auto clonedFunc = context->builder->createFunc();
        registerClonedValue(context, clonedFunc, originalValues);
        cloneFunctionCommon(context, clonedFunc, originalFunc);
        return clonedFunc;
    }

    // Directly clone a global value, based on a single definition/declaration, `originalVal`.
    // The symbol `sym` will thread together other declarations of the same value, and
    // we will register the new value as the cloned version of all of those.
    IRGlobalValue* cloneGlobalValueImpl(
        IRSpecContext*  context,
        IRGlobalValue*  originalVal,
        IRSpecSymbol*   sym)
    {
        if( !originalVal )
        {
            SLANG_UNEXPECTED("cloning a null value");
            UNREACHABLE_RETURN(nullptr);
        }

        switch( originalVal->op )
        {
        case kIROp_Func:
            return cloneFuncImpl(context, (IRFunc*) originalVal, sym);

        case kIROp_global_var:
            return cloneGlobalVarImpl(context, (IRGlobalVar*)originalVal, sym);

        case kIROp_global_constant:
            return cloneGlobalConstantImpl(context, (IRGlobalConstant*)originalVal, sym);

        case kIROp_witness_table:
            return cloneWitnessTableImpl(context, (IRWitnessTable*)originalVal, sym);

        default:
            SLANG_UNEXPECTED("unknown global value kind");
            UNREACHABLE_RETURN(nullptr);
        }

    }

    // Clone a global value, which has the given `mangledName`.
    // The `originalVal` is a known global IR value with that name, if one is available.
    // (It is okay for this parameter to be null).
    IRGlobalValue* cloneGlobalValueWithMangledName(
        IRSpecContext*  context,
        Name*           mangledName,
        IRGlobalValue*  originalVal)
    {
        // Check if we've already cloned this value, for the case where
        // an original value has already been established.
        IRValue* clonedVal = nullptr;
        if( originalVal && context->getClonedValues().TryGetValue(originalVal, clonedVal) )
        {
            return (IRGlobalValue*) clonedVal;
        }

        if(getText(mangledName).Length() == 0)
        {
            // If there is no mangled name, then we assume this is a local symbol,
            // and it can't possibly have multiple declarations.
            return cloneGlobalValueImpl(context, originalVal, nullptr);
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
            UNREACHABLE_RETURN(cloneGlobalValueImpl(context, originalVal, nullptr));
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
        if( !originalVal && context->getClonedValues().TryGetValue(bestVal, clonedVal) )
        {
            return (IRGlobalValue*) clonedVal;
        }

        return cloneGlobalValueImpl(context, bestVal, sym);
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

        for (auto gv = originalModule->firstGlobalValue; gv; gv = gv->nextGlobalValue)
        {
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

    RefPtr<GlobalGenericParamSubstitution> createGlobalGenericParamSubstitution(
        EntryPointRequest * entryPointRequest,
        ProgramLayout * programLayout,
        IRSpecContext*  context,
        IRModule* originalIRModule);

    struct IRSpecializationState
    {
        ProgramLayout*      programLayout;
        CodeGenTarget       target;
        TargetRequest*      targetReq;

        IRModule* irModule = nullptr;
        RefPtr<ProgramLayout> newProgramLayout;

        IRSharedSpecContext sharedContextStorage;
        IRSpecContext contextStorage;

        IRSharedSpecContext* getSharedContext() { return &sharedContextStorage; }
        IRSpecContext* getContext() { return &contextStorage; }
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
        // Create the GlobalGenericParamSubstitution for substituting global generic types
        // into user-provided type arguments
        auto globalParamSubst = createGlobalGenericParamSubstitution(entryPointRequest, programLayout, context, originalIRModule);

        context->subst.globalGenParamSubstitutions = globalParamSubst;
        
        // now specailize the program layout using the substitution
        RefPtr<ProgramLayout> newProgramLayout = specializeProgramLayout(targetReq, programLayout, context->subst);

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
            if (sym.Value->irGlobalValue->op == kIROp_witness_table)
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
        IRSharedSpecContext* getShared() { return shared; }

        // Override the "maybe clone" logic so that we always clone
        virtual IRValue* maybeCloneValue(IRValue* originalVal) override;

        virtual RefPtr<Type> maybeCloneType(Type* originalType) override;
        virtual RefPtr<Val> maybeCloneVal(Val* val) override;
    };

    // Convert a type-level value into an IR-level equivalent.
    IRValue* getIRValue(
        IRGenericSpecContext*   context,
        Val*                    val)
    {
        if( auto subtypeWitness = dynamic_cast<SubtypeWitness*>(val) )
        {
            auto mangledName = context->getModule()->session->getNameObj(getMangledNameForConformanceWitness(
                subtypeWitness->sub,
                subtypeWitness->sup));
            RefPtr<IRSpecSymbol> symbol;

            if (context->getSymbols().TryGetValue(mangledName, symbol))
            {
                return symbol->irGlobalValue;
            }
            else
            {
                // we don't have the required witness table yet, 
                // try to emit a specialize instruction to get one
                auto subDeclRef = subtypeWitness->sub->AsDeclRefType();
                auto subDeclRefGen = DeclRef<Decl>(subDeclRef->declRef.decl, 
                    createDefaultSubstitutions(context->builder->getSession(), subDeclRef->declRef.decl));

                auto genericName = context->getModule()->session->getNameObj(getMangledNameForConformanceWitness(
                    subDeclRefGen,
                    subtypeWitness->sup));
                if (context->getSymbols().TryGetValue(genericName, symbol))
                {
                    auto specInst = context->builder->emitSpecializeInst(subtypeWitness->sup, symbol->irGlobalValue, subDeclRef->declRef);
                    return specInst;
                }
                else
                {
                    SLANG_UNEXPECTED("witness table not exist");
                    UNREACHABLE_RETURN(nullptr);
                }
            }
        }
        else if (auto intVal = dynamic_cast<ConstantIntVal*>(val))
        {
            return context->builder->getIntValue(context->shared->originalModule->session->getBuiltinType(BaseType::Int), intVal->value);
        }
        else if (auto proxyVal = dynamic_cast<IRProxyVal*>(val))
        {
            // The type-level value actually references an IR-level value,
            // so we need to make sure to emit as if we were referencing
            // the pointed-to value and not the proxy type-level `Val`
            // instead.

            return context->maybeCloneValue(proxyVal->inst.get());
        }
        else
        {
            SLANG_UNEXPECTED("unimplemented");
            UNREACHABLE_RETURN(nullptr);
        }
    }

    IRValue* getSubstValue(
        IRGenericSpecContext*   context,
        DeclRef<Decl>           declRef)
    {
        auto subst = context->subst.genericSubstitutions;
        SLANG_ASSERT(subst);
        auto genericDecl = subst->genericDecl;

        UInt orinaryParamCount = 0;
        for( auto mm : genericDecl->Members )
        {
            if(mm.As<GenericTypeParamDecl>())
                orinaryParamCount++;
            else if(mm.As<GenericValueParamDecl>())
                orinaryParamCount++;
        }

        if( auto constraintDeclRef = declRef.As<GenericTypeConstraintDecl>() )
        {
            // We have a constraint, but we need to find its index in the
            // argument list of the substitutions.
            UInt constraintIndex = 0;
            bool found = false;
            for( auto cd : genericDecl->getMembersOfType<GenericTypeConstraintDecl>() )
            {
                if( cd.Ptr() == constraintDeclRef.getDecl() )
                {
                    found = true;
                    break;
                }

                constraintIndex++;
            }
            assert(found);

            UInt argIndex = orinaryParamCount + constraintIndex;
            assert(argIndex < subst->args.Count());

            return getIRValue(context, subst->args[argIndex]);
        }
        else if (auto valDeclRef = declRef.As<GenericValueParamDecl>())
        {
            // We have a constraint, but we need to find its index in the
            // argument list of the substitutions.
            UInt argIdx = 0;
            bool found = false;
            for (auto cd : genericDecl->Members)
            {
                if (cd.Ptr() == valDeclRef.getDecl())
                {
                    found = true;
                    break;
                }
                if (cd.As<GenericTypeParamDecl>())
                    argIdx++;
                else if (cd.As<GenericValueParamDecl>())
                    argIdx++;
            }
            assert(found);

            assert(argIdx < subst->args.Count());

            return getIRValue(context, subst->args[argIdx]);
        }
        else
        {
            SLANG_UNEXPECTED("unimplemented");
            UNREACHABLE_RETURN(nullptr);
        }
    }

    IRValue* IRGenericSpecContext::maybeCloneValue(IRValue* originalVal)
    {
        switch( originalVal->op )
        {
        case kIROp_decl_ref:
            {
                auto declRefVal = (IRDeclRef*) originalVal;
                auto declRef = declRefVal->declRef;
                auto genSubst = subst.genericSubstitutions;
                SLANG_ASSERT(genSubst);
                // We may have a direct reference to one of the parameters
                // of the generic we are specializing, and in that case
                // we nee to translate it over to the equiavalent of
                // the `Val` we have been given.
                if(declRef.getDecl()->ParentDecl == genSubst->genericDecl &&
                    (declRef.As<GenericTypeParamDecl>() || declRef.As<GenericValueParamDecl>()||
                        declRef.As<GenericTypeConstraintDecl>()))
                {
                    if (auto substVal = getSubstValue(this, declRef))
                        return substVal;
                }
                int diff = 0;
                auto substDeclRef = declRefVal->declRef.SubstituteImpl(subst, &diff);
                if(!diff)
                    return originalVal;

                return builder->getDeclRefVal(substDeclRef);
            }
            break;

        default:
            return originalVal;
        }
    }

    RefPtr<Type> IRGenericSpecContext::maybeCloneType(Type* originalType)
    {
        return originalType->Substitute(subst).As<Type>();
    }

    RefPtr<Val> IRGenericSpecContext::maybeCloneVal(Val * val)
    {
        return val->Substitute(subst);
    }

    // Given a list of substitutions, return the inner-most
    // generic substitution in the list, or NULL if there
    // are no generic substitutions.
    RefPtr<GenericSubstitution> getInnermostGenericSubst(
        SubstitutionSet  inSubst)
    {
        return inSubst.genericSubstitutions;
    }

    RefPtr<GenericDecl> getInnermostGenericDecl(
        Decl*   inDecl)
    {
        auto decl = inDecl;
        while( decl )
        {
            GenericDecl* genericDecl = dynamic_cast<GenericDecl*>(decl);
            if(genericDecl)
                return genericDecl;

            decl = decl->ParentDecl;
        }
        return nullptr;
    }

    // This function takes a list of substitutions that we'd
    // like to apply, but which might apply to a different
    // declaration in cases where we have got target-specific
    // overloads in the mix, and produces a new set of 
    // substitutiosn without this issue.
    RefPtr<GenericSubstitution> cloneSubstitutionsForSpecialization(
        IRSharedSpecContext* sharedContext,
        RefPtr<GenericSubstitution> oldSubst,
        Decl*                       newDecl)
    {
        // We will "peel back" layers of substitutions until
        // we find our first generic subsitution.
        auto oldGenericSubst = oldSubst;
        if(!oldGenericSubst)
            return nullptr;

        auto innerGenericName = oldGenericSubst->genericDecl->inner->getName();

        // We will also peel back layers of declarations until
        // we find our first generic decl.
        GenericDecl* newGenericDecl = nullptr;

        for (Decl* d = newDecl; d; d = d->ParentDecl)
        {
            if (auto gd = dynamic_cast<GenericDecl*>(d))
            {
                if (gd->inner->getName() == innerGenericName)
                {
                    newGenericDecl = gd;
                    break;
                }
            }
        }

        if( !newGenericDecl )
        {
            if(auto gd = dynamic_cast<GenericDecl*>(newDecl))
            {
                if( auto ed = gd->inner.As<ExtensionDecl>() )
                {
                    // TODO: we should confirm that it is an extension for the correct type...

                    newGenericDecl = gd;
                }
            }
        }

        SLANG_ASSERT(newGenericDecl);

        RefPtr<GenericSubstitution> newSubst = new GenericSubstitution();
        newSubst->genericDecl = newGenericDecl;
        newSubst->args = oldGenericSubst->args;

        newSubst->outer = cloneSubstitutionsForSpecialization(
            sharedContext,
            oldGenericSubst->outer,
            newGenericDecl->ParentDecl);

        return newSubst;
    }

    IRFunc* getSpecializedFunc(
        IRSharedSpecContext* sharedContext,
        IRFunc*                     genericFunc,
        DeclRef<Decl>               specDeclRef);

    IRWitnessTable* specializeWitnessTable(IRSharedSpecContext * sharedContext, IRWitnessTable* originalTable, DeclRef<Decl> specDeclRef, IRWitnessTable* dstTable)
    {
        // First, we want to see if an existing specialization
        // has already been made. To do that we will need to
        // compute the mangled name of the specialized function,
        // so that we can look for existing declarations.
        String specializedMangledName = getMangledNameForConformanceWitness(specDeclRef.Substitute(originalTable->subTypeDeclRef),
            specDeclRef.Substitute(originalTable->supTypeDeclRef));

        if (dstTable && getText(dstTable->mangledName).Length())
            specializedMangledName = getText(dstTable->mangledName);

        // TODO: This is a terrible linear search, and we should
        // avoid it by building a dictionary ahead of time,
        // as is being done for the `IRSpecContext` used above.
        // We can probalby use the same basic context, actually.
        if (!dstTable)
        {
            auto module = sharedContext->module;
            for (auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue())
            {
                if (getText(gv->mangledName) == specializedMangledName)
                    return (IRWitnessTable*)gv;
            }
        }
        RefPtr<GenericSubstitution> newSubst = cloneSubstitutionsForSpecialization(
            sharedContext,
            specDeclRef.substitutions.genericSubstitutions,
            originalTable->genericDecl);

        IRGenericSpecContext context;
        context.shared = sharedContext;
        context.builder = &sharedContext->builderStorage;
        context.subst = specDeclRef.substitutions;
        context.subst.genericSubstitutions = newSubst;
        // TODO: other initialization is needed here...

        auto specTable = cloneWitnessTableWithoutRegistering(&context, originalTable, dstTable);

        // Set up the clone to recognize that it is no longer generic
        specTable->mangledName = context.getModule()->session->getNameObj(specializedMangledName);
        specTable->genericDecl = nullptr;
        
        // Specialization of witness tables should trigger cascading specializations 
        // of involved functions.
        for (auto entry : specTable->entries)
        {
            if (entry->satisfyingVal.get()->op == kIROp_Func)
            {
                IRFunc* func = (IRFunc*)entry->satisfyingVal.get();
                auto specFunc = getSpecializedFunc(sharedContext, func, specDeclRef);
                entry->satisfyingVal.set(specFunc);
                insertGlobalValueSymbol(sharedContext, specFunc);
            }
            
        }
        // We also need to make sure that we register this specialized
        // function under its mangled name, so that later lookup
        // steps will find it.
        insertGlobalValueSymbol(sharedContext, specTable);

        return specTable;
    }
    
    IRFunc* getSpecializedFunc(
        IRSharedSpecContext* sharedContext,
        IRFunc*                     genericFunc,
        DeclRef<Decl>               specDeclRef)
    {
        // First, we want to see if an existing specialization
        // has already been made. To do that we will need to
        // compute the mangled name of the specialized function,
        // so that we can look for existing declarations.
        String specMangledName;
        if (genericFunc->getGenericDecl() == specDeclRef.decl)
            specMangledName = getMangledName(specDeclRef);
        else
            specMangledName = mangleSpecializedFuncName(getText(genericFunc->mangledName), specDeclRef.substitutions);
        auto specMangledNameObj = sharedContext->module->session->getNameObj(specMangledName);
        RefPtr<IRSpecSymbol> symb;
        if (sharedContext->symbols.TryGetValue(specMangledNameObj, symb))
        {
            return (IRFunc*)(symb->irGlobalValue);
        }
        // TODO: This is a terrible linear search, and we should
        // avoid it by building a dictionary ahead of time,
        // as is being done for the `IRSpecContext` used above.
        // We can probalby use the same basic context, actually.
        for (auto gv = sharedContext->module->getFirstGlobalValue(); gv; gv = gv->getNextValue())
        {
            if (gv->mangledName == specMangledNameObj)
                return (IRFunc*) gv;
        }

        // If we get to this point, then we need to construct a
        // new `IRFunc` to represent the result of specialization.

        // The substitutions we are applying might have been created
        // using a different overload of a target-specific function,
        // so we need to create a dummy substitution here, to make
        // sure it used the correct generic.
        RefPtr<GenericSubstitution> newSubst = cloneSubstitutionsForSpecialization(
            sharedContext,
            specDeclRef.substitutions.genericSubstitutions,
            genericFunc->getGenericDecl());

        if (!newSubst)
            return genericFunc;

        IRGenericSpecContext context;
        context.shared = sharedContext;
        context.builder = &sharedContext->builderStorage;
        context.subst = specDeclRef.substitutions;
        context.subst.genericSubstitutions = newSubst;

        // TODO: other initialization is needed here...

        auto specFunc = cloneSimpleFuncWithoutRegistering(&context, genericFunc);

        specFunc->mangledName = context.getModule()->session->getNameObj(specMangledName);
        
        // reduce specialized generic level by 1
        if (specFunc->specializedGenericLevel >= 0)
            specFunc->specializedGenericLevel--;

        // Put the function into the global sequence right after
        // the function it specializes.
        //
        // TODO: This shouldn't be needed, if we introduce a sorting
        // step before we emit code.
        //specFunc->removeFromParent();
        //specFunc->insertAfter(genericFunc);

        // At this point we've created a new non-generic function,
        // which means we should add it to our work list for
        // subsequent processing.
        if (specFunc->specializedGenericLevel == -1)
            sharedContext->workList.Add(specFunc);

        // We also need to make sure that we register this specialized
        // function under its mangled name, so that later lookup
        // steps will find it.
        insertGlobalValueSymbol(sharedContext, specFunc);

        return specFunc;
    }

    // Find the value in the given witness table that
    // satisfies the given requirement (or return
    // null if not found).
    IRValue* findWitnessVal(
        IRWitnessTable*         witnessTable,
        DeclRef<Decl> const&    requirementDeclRef)
    {
        // For now we will do a dumb linear search
        for( auto entry : witnessTable->entries )
        {
            // We expect the key on the entry to be a decl-ref,
            // but lets go ahead and check, just to be sure.
            auto requirementKey = entry->requirementKey.get();
            if(requirementKey->op != kIROp_decl_ref)
                continue;
            auto keyDeclRef = ((IRDeclRef*) requirementKey)->declRef;

            // If the keys don't match, continue with the next entry.
            if (!keyDeclRef.Equals(requirementDeclRef))
            {
                // requirementDeclRef may be pointing to the inner decl of a generic decl
                // in this case we compare keyDeclRef against the parent decl of requiredDeclRef
                if (auto genRequiredDeclRef = requirementDeclRef.GetParent().As<GenericDecl>())
                {
                    if (!keyDeclRef.Equals(genRequiredDeclRef))
                    {
                        continue;
                    }
                }
                else
                    continue;
            }

            // If the keys matched, then we use the value from
            // this entry.
            auto satisfyingVal = entry->satisfyingVal.get();
            return satisfyingVal;
        }

        // No matching entry found.
        return nullptr;
    }

    // Go through the code in the module and try to identify
    // calls to generic functions where the generic arguments
    // are known, and specialize the callee based on those
    // known values.
    void specializeGenerics(
        IRModule*   module,
        CodeGenTarget target)
    {
        IRSharedSpecContext sharedContextStorage;
        auto sharedContext = &sharedContextStorage;

        initializeSharedSpecContext(
            sharedContext,
            module->session,
            module,
            module,
            target);

        // Our goal here is to find `specialize` instructions that
        // can be replaced with references to a suitably sepcialized
        // funciton. As a simplification, we will only consider `specialize`
        // calls that are inside of non-generic functions, since we assume
        // that these will allow us to fully specialize the referenced
        // function.
        //
        // We start by building up a work list of non-generic functions.
        for( auto gv = module->getFirstGlobalValue();
            gv;
            gv = gv->getNextValue() )
        {
            // Is it a function? If not, skip.
            if(gv->op != kIROp_Func)
                continue;
            auto func = (IRFunc*) gv;

            // Is it generic? If so, skip.
            if(func->getGenericDecl())
                continue;

            sharedContext->workList.Add(func);
        }

        // Build dictionary for witness tables
        Dictionary<Name*, IRWitnessTable*> witnessTables;
        for (auto gv = module->getFirstGlobalValue();
            gv;
            gv = gv->getNextValue())
        {
            if (gv->op == kIROp_witness_table)
                witnessTables.AddIfNotExists(gv->mangledName, (IRWitnessTable*)gv);
        }

        // Now that we have our work list, we are going to
        // process it until it goes empty. Along the way
        // we may specialize a function and thus create
        // a new non-generic function, and in that case
        // we will add the new function to the work list.
        auto& workList = sharedContext->workList;
        while( auto count = workList.Count() )
        {
            // We will process the last entry in the
            // work list, which amounts to treating
            // it like a stack when we have recursive
            // specialization to perform.
            auto func = workList[count-1];
            workList.RemoveAt(count-1);
            
            // We are going to go ahead and walk through
            // all the instructions in this function,
            // and look for `specialize` operations.
            for( auto bb = func->getFirstBlock(); bb; bb = bb->getNextBlock() )
            {
                // We need to be careful when iterating over the instructions,
                // because we might end up removing the "current" instruction,
                // so that accessing `ii->next` would crash.
                IRInst* nextInst = nullptr;
                for( auto ii = bb->getFirstInst(); ii; ii = nextInst )
                {
                    nextInst = ii->getNextInst();

                    // We want to handle both `specialize` instructions,
                    // which trigger specialization, and also `lookup_interface_method`
                    // instructions, which may allow us to "de-virtualize"
                    // calls.

                    switch( ii->op )
                    {
                    default:
                        // Most instructions are ones we don't care about here.
                        continue;

                    case kIROp_specialize:
                        {
                            // We have a `specialize` instruction, so lets see
                            // whether we have an opportunity to perform the
                            // specialization here and now.
                            IRSpecialize* specInst = (IRSpecialize*) ii;

                            // Now we extract the specialized decl-ref that will
                            // tell us how to specialize things.
                            auto specDeclRefVal = (IRDeclRef*)specInst->specDeclRefVal.get();
                            auto specDeclRef = specDeclRefVal->declRef;

                            // We need to specialize functions and witness tables
                            auto genericVal = specInst->genericVal.get();
                            if (genericVal->op == kIROp_Func)
                            {
                                auto genericFunc = (IRFunc*)genericVal;
                                if (!genericFunc->getGenericDecl())
                                    continue;

                                // Okay, we have a candidate for specialization here.
                                //
                                // We will first find or construct a specialized version
                                // of the callee funciton/
                                auto specFunc = getSpecializedFunc(sharedContext, genericFunc, specDeclRef);
                                //
                                // Then we will replace the use sites for the `specialize`
                                // instruction with uses of the specialized function.
                                //
                                specInst->replaceUsesWith(specFunc);

                                specInst->removeAndDeallocate();
                            }
                            else if (genericVal->op == kIROp_witness_table)
                            {
                                // specialize a witness table
                                auto originalTable = (IRWitnessTable*)genericVal;
                                auto specWitnessTable = specializeWitnessTable(sharedContext, originalTable, specDeclRef, nullptr); 
                                witnessTables.AddIfNotExists(specWitnessTable->mangledName, specWitnessTable);
                                specInst->replaceUsesWith(specWitnessTable);
                                specInst->removeAndDeallocate();
                            }
                        }
                        break;
                    case kIROp_lookup_witness_table:
                        {
                            // try find concrete witness table from global scope
                            IRLookupWitnessTable* lookupInst = (IRLookupWitnessTable*)ii;
                            IRWitnessTable* witnessTable = nullptr;
                            auto srcDeclRef = ((IRDeclRef*)lookupInst->sourceType.get())->declRef;
                            auto interfaceDeclRef = ((IRDeclRef*)lookupInst->interfaceType.get())->declRef;
                            auto mangledName = module->session->getNameObj(getMangledNameForConformanceWitness(srcDeclRef, interfaceDeclRef));
                            witnessTables.TryGetValue(mangledName, witnessTable);
                            
                            if (!witnessTable)
                            {
                                // try specialize the witness table
                                auto genDeclRef = srcDeclRef;
                                genDeclRef.substitutions = createDefaultSubstitutions(module->session, genDeclRef.decl);
                                auto genName = module->session->getNameObj(getMangledNameForConformanceWitness(genDeclRef, interfaceDeclRef));
                                IRWitnessTable* genTable = nullptr;
                                if (witnessTables.TryGetValue(genName, genTable))
                                {
                                    witnessTable = specializeWitnessTable(sharedContext, genTable, srcDeclRef, nullptr);
                                    witnessTables.AddIfNotExists(witnessTable->mangledName, witnessTable);
                                }
                            }
                            if (witnessTable)
                            {
                                lookupInst->replaceUsesWith(witnessTable);
                                lookupInst->removeAndDeallocate();
                            }
                        }
                        break;
                    case kIROp_lookup_interface_method:
                        {
                            // We have a `lookup_interface_method` instruction,
                            // so let's see whether it is a lookup in a known
                            // witness table.
                            IRLookupWitnessMethod* lookupInst = (IRLookupWitnessMethod*) ii;

                            // We only want to deal with the case where the witness-table
                            // argument points to a concrete global table.
                            auto witnessTableArg = lookupInst->witnessTable.get();
                            if(witnessTableArg->op != kIROp_witness_table)
                                continue;
                            IRWitnessTable* witnessTable = (IRWitnessTable*)witnessTableArg;

                            // We also need to be sure that the requirement we
                            // are trying to look up is identified via a decl-ref:
                            auto requirementArg = lookupInst->requirementDeclRef.get();
                            if(requirementArg->op != kIROp_decl_ref)
                                continue;
                            auto requirementDeclRef = ((IRDeclRef*) requirementArg)->declRef;

                            // Use the witness table to look up the value that
                            // satisfies the requirement.
                            auto satisfyingVal = findWitnessVal(witnessTable, requirementDeclRef);
                            // We expect to always find something, but lets just
                            // be careful here.
                            if(!satisfyingVal)
                                continue;

                            // If we get through all of the above checks, then we
                            // have a (more) concrete method that implements the interface,
                            // and so we should dispatch to that directly, rather than
                            // use the `lookup_interface_method` instruction.
                            lookupInst->replaceUsesWith(satisfyingVal);
                            lookupInst->removeAndDeallocate();
                        }
                        break;
                    }


                    // We only care about `specialize` instructions.
                    if(ii->op != kIROp_specialize)
                        continue;

                }
            }
        }

        // Once the work list has gone dry, we should have the invariant
        // that there are no `specialize` instructions inside of non-generic
        // functions that in turn reference a generic function.
    }

    RefPtr<GlobalGenericParamSubstitution> createGlobalGenericParamSubstitution(
        EntryPointRequest * entryPointRequest,
        ProgramLayout * programLayout,
        IRSpecContext*  context,
        IRModule* originalIRModule)
    {
        RefPtr<GlobalGenericParamSubstitution> globalParamSubst;
        GlobalGenericParamSubstitution * curTailSubst = nullptr;
        struct WitnessTableSpecializationWorkItem
        {
            IRWitnessTable* dstTable;
            IRWitnessTable* srcTable;
            DeclRef<Decl> specDeclRef;
        };
        List<WitnessTableSpecializationWorkItem> witnessTablesToSpecailize;
        for (auto param : programLayout->globalGenericParams)
        {
            auto paramSubst = new GlobalGenericParamSubstitution();
            if (!globalParamSubst)
                globalParamSubst = paramSubst;
            if (curTailSubst)
                curTailSubst->outer = paramSubst;
            curTailSubst = paramSubst;
            paramSubst->paramDecl = param->decl;
            SLANG_ASSERT((UInt)param->index < entryPointRequest->genericParameterTypes.Count());
            paramSubst->actualType = entryPointRequest->genericParameterTypes[param->index];
            // find witness tables
            for (auto witness : entryPointRequest->genericParameterWitnesses)
            {
                if (auto subtypeWitness = witness.As<SubtypeWitness>())
                {
                    if (subtypeWitness->sub->EqualsVal(paramSubst->actualType))
                    {
                        auto witnessTableName = getMangledNameForConformanceWitness(subtypeWitness->sub, subtypeWitness->sup);
                        auto findWitnessTableByName = [&](String name)
                        {
                            auto globalVar = originalIRModule->getFirstGlobalValue();
                            IRGlobalValue * rs = nullptr;
                            while (globalVar)
                            {
                                if (getText(globalVar->mangledName) == name)
                                {
                                    rs = globalVar;
                                    break;
                                }
                                globalVar = globalVar->getNextValue();
                            }
                            return rs;
                        };
                        auto table = findWitnessTableByName(witnessTableName);
                        if (!table)
                        {
                            if (auto subDeclRefType = subtypeWitness->sub.As<DeclRefType>())
                            {
                                auto defaultSubst = createDefaultSubstitutions(entryPointRequest->compileRequest->mSession, subDeclRefType->declRef.getDecl());
                                auto genericWitnessTableName = getMangledNameForConformanceWitness(DeclRef<Decl>(subDeclRefType->declRef.getDecl(), defaultSubst), subtypeWitness->sup);
                                table = findWitnessTableByName(genericWitnessTableName);
                                SLANG_ASSERT(table);
                                WitnessTableSpecializationWorkItem workItem;
                                workItem.srcTable = (IRWitnessTable*)cloneGlobalValue(context, (IRWitnessTable*)(table));
                                workItem.dstTable = context->builder->createWitnessTable();
                                workItem.dstTable->mangledName = context->getModule()->session->getNameObj(getMangledNameForConformanceWitness(subDeclRefType->declRef, subtypeWitness->sup));
                                workItem.specDeclRef = subDeclRefType->declRef;
                                witnessTablesToSpecailize.Add(workItem);
                                table = workItem.dstTable;
                            }
                        }
                        else
                            table = cloneGlobalValue(context, (IRWitnessTable*)(table));
                        SLANG_ASSERT(table);
                        IRProxyVal * tableVal = new IRProxyVal();
                        tableVal->inst.init(nullptr, table);
                        paramSubst->witnessTables.Add(KeyValuePair<RefPtr<Type>, RefPtr<Val>>(subtypeWitness->sup, tableVal));
                    }
                }
            }
        }
        for (auto workItem : witnessTablesToSpecailize)
        {
            int diff = 0;
            specializeWitnessTable(context->shared, workItem.srcTable,
                workItem.specDeclRef.SubstituteImpl(SubstitutionSet(nullptr, nullptr, globalParamSubst), &diff), workItem.dstTable);
        }
        return globalParamSubst;
    }

    
    void markConstExpr(
        Session* session,
        IRValue* irValue)
    {
        // We will take an IR value with type `T`,
        // and turn it into one with type `@ConstExpr T`.

        // TODO: need to be careful if the value already has a rate
        // qualifier set.

        irValue->type = session->getConstExprType(irValue->getDataType());
    }
}
