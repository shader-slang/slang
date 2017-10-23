// ir.cpp
#include "ir.h"
#include "ir-insts.h"

#include "../core/basic.h"
#include "mangle.h"

namespace Slang
{

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

        if (0) {}

#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    else if(strcmp(name, #MNEMONIC) == 0) return kIROp_##ID;

#define PSEUDO_INST(ID)  \
    else if(strcmp(name, #ID) == 0) return kIRPseudoOp_##ID;

#include "ir-inst-defs.h"

        return IROp(kIROp_Invalid);
    }

    IROpInfo getIROpInfo(IROp op)
    {
        return kIROpInfos[op];
    }

    //

    void IRUse::init(IRInst* u, IRValue* v)
    {
        user = u;
        usedValue = v;

        if(v)
        {
            nextUse = v->firstUse;
            prevLink = &v->firstUse;

            v->firstUse = this;
        }
    }

    //

    IRUse* IRInst::getArgs()
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

    void IRFunc::addBlock(IRBlock* block)
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
        case kIROp_break:
        case kIROp_continue:
        case kIROp_loop:
        case kIROp_if:
        case kIROp_ifElse:
        case kIROp_loopTest:
            return true;
        }
    }

    bool isTerminatorInst(IRInst* inst)
    {
        if (!inst) return false;
        return isTerminatorInst(inst->op);
    }


    //

    // Add an instruction to a specific parent
    void IRBuilder::addInst(IRBlock* block, IRInst* inst)
    {
        inst->parentBlock = block;

        if (!block->firstInst)
        {
            inst->prevInst = nullptr;
            inst->nextInst = nullptr;

            block->firstInst = inst;
            block->lastInst = inst;
        }
        else
        {
            auto prev = block->lastInst;

            inst->prevInst = prev;
            inst->nextInst = nullptr;

            prev->nextInst = inst;
            block->lastInst = inst;
        }
    }

    // Add an instruction into the current scope
    void IRBuilder::addInst(
        IRInst*     inst)
    {
        auto insertBefore = insertBeforeInst;
        if(insertBeforeInst)
        {
            inst->insertBefore(insertBeforeInst);
            return;
        }

        auto parent = block;
        if (!parent)
            return;

        addInst(parent, inst);
    }

    static IRValue* createValueImpl(
        IRBuilder*  builder,
        UInt        size,
        IROp        op,
        IRType*     type)
    {
        IRValue* value = (IRValue*) malloc(size);
        memset(value, 0, size);
        value->op = op;
        value->type = type;
        return value;
    }

    template<typename T>
    static T* createValue(
        IRBuilder*  builder,
        IROp        op,
        IRType*     type)
    {
        return (T*) createValueImpl(builder, sizeof(T), op, type);
    }


    // Create an IR instruction/value and initialize it.
    //
    // In this case `argCount` and `args` represnt the
    // arguments *after* the type (which is a mandatory
    // argument for all instructions).
    static IRInst* createInstImpl(
        IRBuilder*      builder,
        UInt            size,
        IROp            op,
        IRType*         type,
        UInt            fixedArgCount,
        IRValue* const* fixedArgs,
        UInt            varArgCount = 0,
        IRValue* const* varArgs = nullptr)
    {
        IRInst* inst = (IRInst*) malloc(size);
        memset(inst, 0, size);

        auto module = builder->getModule();
        inst->argCount = fixedArgCount + varArgCount;

        inst->op = op;

        inst->type = type;

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
        return (T*)createInstImpl(
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
        return (T*)createInstImpl(
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
        return (T*)createInstImpl(
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
        return (T*)createInstImpl(
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
        return (T*)createInstImpl(
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
        return (T*)createInstImpl(
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

        return (T*)createInstImpl(
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
        if(left.inst->parentBlock != right.inst->parentBlock) return false;
        if(left.inst->argCount != right.inst->argCount) return false;

        auto argCount = left.inst->argCount;
        auto leftArgs = left.inst->getArgs();
        auto rightArgs = right.inst->getArgs();
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            if(leftArgs[aa].usedValue != rightArgs[aa].usedValue)
                return false;
        }

        return true;
    }

    int IRInstKey::GetHashCode()
    {
        auto code = Slang::GetHashCode(inst->op);
        code = combineHash(code, Slang::GetHashCode(inst->parentBlock));
        code = combineHash(code, Slang::GetHashCode(inst->argCount));

        auto argCount = inst->argCount;
        auto args = inst->getArgs();
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            code = combineHash(code, Slang::GetHashCode(args[aa].usedValue));
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
        if( builder->shared->constantMap.TryGetValue(key, irValue) )
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

        irValue = createInst<IRConstant>(builder, op, type);
        memcpy(&irValue->u, value, valueSize);

        key.inst = irValue;
        builder->shared->constantMap.Add(key, irValue);

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

    IRValue* IRBuilder::getDeclRefVal(
        DeclRefBase const&  declRef)
    {
        // TODO: we should cache these...
        auto irValue = createInst<IRDeclRef>(
            this,
            kIROp_decl_ref,
            nullptr);
        irValue->declRef = DeclRef<Decl>(declRef.decl, declRef.substitutions);
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

    IRInst* IRBuilder::emitCallInst(
        IRType*         type,
        IRValue*        func,
        UInt            argCount,
        IRValue* const* args)
    {
        auto inst = createInstWithTrailingArgs<IRCall>(
            this,
            kIROp_Call,
            type,
            1,
            &func,
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
        IRFunc* func = createValue<IRFunc>(
            this,
            kIROp_Func,
            nullptr);
        addGlobalValue(getModule(), func);
        return func;
    }

    IRGlobalVar* IRBuilder::createGlobalVar(
        IRType* valueType)
    {
        auto ptrType = getSession()->getPtrType(valueType);
        IRGlobalVar* globalVar = createValue<IRGlobalVar>(
            this,
            kIROp_global_var,
            ptrType);
        addGlobalValue(getModule(), globalVar);
        return globalVar;
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

        auto f = this->func;
        if (f)
        {
            f->addBlock(bb);
            this->block = bb;
        }
        return bb;
    }

    IRParam* IRBuilder::emitParam(
        IRType* type)
    {
        auto param = createValue<IRParam>(
            this,
            kIROp_Param,
            type);

        if (auto bb = block)
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
        auto ptrType = ptr->getType()->As<PtrType>();
        if( !ptrType )
        {
            // Bad!
            return nullptr;
        }

        auto valueType = ptrType->getValueType();

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

    IRInst* IRBuilder::emitBranch(
        IRBlock*    block)
    {
        auto inst = createInst<IRUnconditionalBranch>(
            this,
            kIROp_unconditionalBranch,
            nullptr,
            block);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitBreak(
        IRBlock*    target)
    {
        auto inst = createInst<IRBreak>(
            this,
            kIROp_break,
            nullptr,
            target);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitContinue(
        IRBlock*    target)
    {
        auto inst = createInst<IRContinue>(
            this,
            kIROp_continue,
            nullptr,
            target);
        addInst(inst);
        return inst;
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

    IRInst* IRBuilder::emitIf(
        IRValue*    val,
        IRBlock*    trueBlock,
        IRBlock*    afterBlock)
    {
        IRValue* args[] = { val, trueBlock, afterBlock };
        UInt argCount = sizeof(args) / sizeof(args[0]);

        auto inst = createInst<IRIf>(
            this,
            kIROp_if,
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

    IRInst* IRBuilder::emitLoopTest(
        IRValue*    val,
        IRBlock*    bodyBlock,
        IRBlock*    breakBlock)
    {
        IRValue* args[] = { val, bodyBlock, breakBlock };
        UInt argCount = sizeof(args) / sizeof(args[0]);

        auto inst = createInst<IRLoopTest>(
            this,
            kIROp_loopTest,
            nullptr,
            argCount,
            args);
        addInst(inst);
        return inst;
    }

    IRDecoration* IRBuilder::addDecorationImpl(
        IRValue*        inst,
        UInt            decorationSize,
        IRDecorationOp  op)
    {
        auto decoration = (IRDecoration*) malloc(decorationSize);
        memset(decoration, 0, decorationSize);

        decoration->op = op;

        decoration->next = inst->firstDecoration;
        inst->firstDecoration = decoration;

        return decoration;
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
        int     indent;

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
            {
                auto irFunc = (IRFunc*) inst;
                dump(context, "@");
                dump(context, irFunc->mangledName.Buffer());
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


    static void dumpDeclRef(
        IRDumpContext*          context,
        DeclRef<Decl> const&    declRef);

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

        if(genericParentDeclRef)
        {
            auto subst = declRef.substitutions;
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
        for (auto ii = block->firstInst; ii; ii = ii->nextInst)
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
                dumpInstTypeClause(context, pp->getType());
            }
            context->indent -= 2;
            dump(context, ")");
        }
        dump(context, ":\n");
        context->indent++;

        dumpChildrenRaw(context, block);
    }

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
        auto type = inst->getType();

        if (!type)
        {
            // No result, okay...
        }
        else
        {
            auto basicType = type->As<BasicExpressionType>();
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

            auto argVal = inst->getArgs()[ii].usedValue;

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
            if (!first) dump(context, ", ");

            if( auto typeParamDecl = mm.As<GenericTypeParamDecl>() )
            {
                dumpDeclRef(context, makeDeclRef(typeParamDecl.Ptr()));
                first = false;
            }
            else if( auto valueParamDecl = mm.As<GenericTypeParamDecl>() )
            {
                dumpDeclRef(context, makeDeclRef(valueParamDecl.Ptr()));
                first = false;
            }
        }
        first = true;
        for (auto mm : genericDecl->Members)
        {
            if (!first) dump(context, ", ");
            else dump(context, " where ");

            if( auto constraintDecl = mm.As<GenericTypeConstraintDecl>() )
            {
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

        if (func->genericDecl)
        {
            dump(context, " ");
            dumpGenericSignature(context, func->genericDecl);
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

    void dumpIRGlobalVar(
        IRDumpContext*  context,
        IRGlobalVar*    var)
    {
        dump(context, "\n");
        dumpIndent(context);
        dump(context, "ir_global_var ");
        dumpID(context, var);
        dumpInstTypeClause(context, var->getType());

        // TODO: deal with the case where a global
        // might have embedded initialization logic.

        dump(context, ";\n");
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

    void IRValue::replaceUsesWith(IRValue* other)
    {
        // We will walk through the list of uses for the current
        // instruction, and make them point to the other inst.
        IRUse* ff = firstUse;

        // No uses? Nothing to do.
        if(!ff)
            return;

        IRUse* uu = ff;
        for(;;)
        {
            // The uses had better all be uses of this
            // instruction, or invariants are broken.
            assert(uu->usedValue == this);

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
    }

    void IRValue::deallocate()
    {
        // Run destructor to be sure...
        this->~IRValue();

        // And then free the memory
        free((void*) this);
    }

    // Insert this instruction into the same basic block
    // as `other`, right before it.
    void IRInst::insertBefore(IRInst* other)
    {
        // Make sure this instruction has been removed from any previous parent
        this->removeFromParent();

        auto bb = other->parentBlock;
        assert(bb);

        auto pp = other->prevInst;
        if( pp )
        {
            pp->nextInst = this;
        }
        else
        {
            bb->firstInst = this;
        }

        this->prevInst = pp;
        this->nextInst = other;
        this->parentBlock = bb;

        other->prevInst = this;
    }

    // Remove this instruction from its parent block,
    // and then destroy it (it had better have no uses!)
    void IRInst::removeFromParent()
    {
        // If we don't currently have a parent, then
        // we are doing fine.
        if(!parentBlock)
            return;

        auto bb = parentBlock;
        auto pp = prevInst;
        auto nn = nextInst;

        if(pp)
        {
            SLANG_ASSERT(pp->parentBlock == bb);
            pp->nextInst = nn;
        }
        else
        {
            bb->firstInst = nn;
        }

        if(nn)
        {
            SLANG_ASSERT(nn->parentBlock == bb);
            nn->prevInst = pp;
        }
        else
        {
            bb->lastInst = pp;
        }

        prevInst = nullptr;
        nextInst = nullptr;
        parentBlock = nullptr;
    }

    void IRInst::removeArguments()
    {
        UInt argCount = this->argCount;
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            IRUse& use = getArgs()[aa];

            if(!use.usedValue)
                continue;

            // Need to unlink this use from the appropriate linked list.
            use.usedValue = nullptr;
            *use.prevLink = use.nextUse;
            use.prevLink = nullptr;
            use.nextUse = nullptr;
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
        builder.shared = &shared;

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

    struct GlobalVaryingDeclarator
    {
        enum class Flavor
        {
            array,
        };

        Flavor                      flavor;
        Val*                        elementCount;
        GlobalVaryingDeclarator*    next;
    };

    ScalarizedVal createSimpleGLSLGlobalVarying(
        IRBuilder*                  builder,
        Type*                       type,
        VarLayout*                  varLayout,
        TypeLayout*                 typeLayout,
        LayoutResourceKind          kind,
        GlobalVaryingDeclarator*    declarator)
    {
        // TODO: We might be creating an `in` or `out` variable based on
        // an `in out` function parameter. In this case we should
        // rewrite the `typeLayout` to only include the information
        // for the appropriate `kind`.
        //
        // TODO: actually, we should *always* be re-creating the layout,
        // because we need to apply any offsets from the parent...

        // TODO: If there are any `declarator`s, we need to unwrap
        // them here, and allow them to modify the type of the
        // variable that we declare.
        //
        // They should probably also affect how we return the
        // `ScalarizedVal`, since we need to reflect the AOS->SOA conversion.

        // TODO: detect when the layout represents a system input/output
        if( varLayout->systemValueSemantic.Length() != 0 )
        {
            // This variable represents a system input/output,
            // and we should probably handle that differently, right?
        }

        // Simple case: just create a global variable of the matching type,
        // and then use the value of the global as a replacement for the
        // value of the original parameter.
        //
        auto globalVariable = addGlobalVariable(builder->getModule(), type);
        moveValueBefore(globalVariable, builder->getFunc());
        builder->addLayoutDecoration(globalVariable, varLayout);
        return ScalarizedVal::address(globalVariable);
    }

    ScalarizedVal createGLSLGlobalVaryingsImpl(
        IRBuilder*                  builder,
        Type*                       type,
        VarLayout*                  varLayout,
        TypeLayout*                 typeLayout,
        LayoutResourceKind          kind,
        GlobalVaryingDeclarator*    declarator)
    {
        if( type->As<BasicExpressionType>() )
        {
            return createSimpleGLSLGlobalVarying(builder, type, varLayout, typeLayout, kind, declarator);
        }
        else if( type->As<VectorExpressionType>() )
        {
            return createSimpleGLSLGlobalVarying(builder, type, varLayout, typeLayout, kind, declarator);
        }
        else if( type->As<MatrixExpressionType>() )
        {
            // TODO: a matrix-type varying should probably be handled like an array of rows
            return createSimpleGLSLGlobalVarying(builder, type, varLayout, typeLayout, kind, declarator);
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
                builder,
                elementType,
                varLayout,
                elementTypeLayout,
                kind,
                &arrayDeclarator);
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
                    tupleValImpl->type = type;

                    // Okay, we want to walk through the fields here, and
                    // generate one variable for each.
                    for( auto ff : structTypeLayout->fields )
                    {
                        auto fieldVal = createGLSLGlobalVaryingsImpl(
                            builder,
                            ff->typeLayout->type,
                            ff,
                            ff->typeLayout,
                            kind,
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
        return createSimpleGLSLGlobalVarying(builder, type, varLayout, typeLayout, kind, declarator);
    }

    ScalarizedVal createGLSLGlobalVaryings(
        IRBuilder*          builder,
        Type*               type,
        VarLayout*          layout,
        LayoutResourceKind  kind)
    {
        return createGLSLGlobalVaryingsImpl(builder, type, layout, layout->typeLayout, kind, nullptr);
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
            return ScalarizedVal();
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

        default:
            SLANG_UNEXPECTED("unimplemented");
            break;
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
                UInt elementCount = tupleVal->elements.Count();

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
            break;

        default:
            SLANG_UNEXPECTED("unimplemented");
            break;
        }
    }

    void legalizeEntryPointForGLSL(
        Session*                session,
        IRFunc*                 func,
        EntryPointLayout*       entryPointLayout)
    {
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
        builder.shared = &shared;
        builder.func = func;

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
                &builder,
                resultType,
                entryPointLayout->resultLayout,
                LayoutResourceKind::FragmentOutput);

            for( auto bb = func->getFirstBlock(); bb; bb = bb->getNextBlock() )
            {
                // TODO: This is silly, because we are looking at every instruction,
                // when we know that a `returnVal` should only ever appear as a
                // terminator...
                for( auto ii = bb->getFirstInst(); ii; ii = ii->nextInst )
                {
                    if(ii->op != kIROp_ReturnVal)
                        continue;

                    IRReturnVal* returnInst = (IRReturnVal*) ii;
                    IRValue* returnValue = returnInst->getVal();

                    // Make sure we add these instructions to the right block
                    builder.block = bb;

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
                auto paramType = pp->getType();

                // Any initialization code we insert nees to be at the start
                // of the block:
                builder.block = firstBlock;
                builder.insertBeforeInst = firstBlock->getFirstInst();

                // TODO: We need to distinguish any true pointers in the
                // user's code from pointers that only exist for
                // parameter-passing. This `PtrType` here should actually
                // be `OutTypeBase`, but I'm not confident that all
                // the other code is handling that correctly...
                if(auto paramPtrType = paramType->As<PtrType>() )
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
                        auto globalInputVal = createGLSLGlobalVaryings(&builder, valueType, paramLayout, LayoutResourceKind::VertexInput);

                        assign(&builder, localVal, globalInputVal);
                    }

                    // Any places where the original parameter was used inside
                    // the function body should instead use the new local variable.
                    // Since the parameter was a pointer, we use the variable instruction
                    // itself (which is an `alloca`d pointer) directly:
                    pp->replaceUsesWith(localVariable);

                    // We also need one or more global variabels to write the output to
                    // when the function is done. We create them here.
                    auto globalOutputVal = createGLSLGlobalVaryings(&builder, valueType, paramLayout, LayoutResourceKind::FragmentOutput);

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

                        builder.block = bb;
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

                    auto globalValue = createGLSLGlobalVaryings(&builder, paramType, paramLayout, LayoutResourceKind::VertexInput);

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

        auto voidFuncType = new FuncType();
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
        // The specialized module we are building
        IRModule*   module;

        // The original, unspecialized module we are copying
        IRModule*   originalModule;

        // A map from mangled symbol names to zero or
        // more global IR values that have that name,
        // in the *original* module.
        typedef Dictionary<String, RefPtr<IRSpecSymbol>> SymbolDictionary;
        SymbolDictionary symbols;

        // A map from values in the original IR module
        // to their equivalent in the cloned module.
        typedef Dictionary<IRValue*, IRValue*> ClonedValueDictionary;
        ClonedValueDictionary clonedValues;

        SharedIRBuilder sharedBuilderStorage;
        IRBuilder builderStorage;
    };

    struct IRSpecContextBase
    {
        IRSharedSpecContext* shared;

        IRSharedSpecContext* getShared() { return shared; }

        IRModule* getModule() { return getShared()->module; }

        IRModule* getOriginalModule() { return getShared()->originalModule; }

        IRSharedSpecContext::SymbolDictionary& getSymbols() { return getShared()->symbols; }

        IRSharedSpecContext::ClonedValueDictionary& getClonedValues() { return getShared()->clonedValues; }

        // The IR builder to use for creating nodes
        IRBuilder*  builder;

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
    };

    void registerClonedValue(
        IRSpecContextBase*  context,
        IRValue*    clonedValue,
        IRValue*    originalValue)
    {
        context->getClonedValues().Add(originalValue, clonedValue);
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

            default:
                // Don't clone any decorations we don't understand.
                break;
            }
        }

        // TODO: implement this
    }

    struct IRSpecContext : IRSpecContextBase
    {
        // The code-generation target in use
        CodeGenTarget target;

        // A map from the mangled name of a global variable
        // to the layout to use for it.
        Dictionary<String, VarLayout*> globalVarLayouts;

        // Override the "maybe clone" logic so that we always clone
        virtual IRValue* maybeCloneValue(IRValue* originalVal) override;
    };


    IRGlobalVar* cloneGlobalVar(IRSpecContext* context, IRGlobalVar* originalVar);
    IRFunc* cloneFunc(IRSpecContext* context, IRFunc* originalFunc);

    IRValue* IRSpecContext::maybeCloneValue(IRValue* originalValue)
    {
        switch (originalValue->op)
        {
        case kIROp_global_var:
            return cloneGlobalVar(this, (IRGlobalVar*)originalValue);
            break;

        case kIROp_Func:
            return cloneFunc(this, (IRFunc*)originalValue);
            break;

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
                return builder->getDeclRefVal(od->declRef);
            }
            break;

        default:
            SLANG_UNEXPECTED("no value registered for IR value");
            return nullptr;
        }
    }

    IRValue* cloneValue(
        IRSpecContextBase*  context,
        IRValue*        originalValue)
    {
        IRValue* clonedValue = nullptr;
        if (context->getClonedValues().TryGetValue(originalValue, clonedValue))
            return clonedValue;

        return context->maybeCloneValue(originalValue);
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
                builder->addInst(clonedInst);
                registerClonedValue(context, clonedInst, originalInst);

                cloneDecorations(context, clonedInst, originalInst);

                for (UInt aa = 0; aa < argCount; ++aa)
                {
                    IRValue* originalArg = originalInst->getArg(aa);
                    IRValue* clonedArg = cloneValue(context, originalArg);

                    clonedInst->getArgs()[aa].init(clonedInst, clonedArg);
                }
            }

            break;
        }
    }

    IRGlobalVar* cloneGlobalVar(IRSpecContext* context, IRGlobalVar* originalVar)
    {
        auto clonedVar = context->builder->createGlobalVar(originalVar->getType()->getValueType());
        registerClonedValue(context, clonedVar, originalVar);

        auto mangledName = originalVar->mangledName;
        clonedVar->mangledName = mangledName;

        cloneDecorations(context, clonedVar, originalVar);

        VarLayout* layout = nullptr;
        if (context->globalVarLayouts.TryGetValue(mangledName, layout))
        {
            context->builder->addLayoutDecoration(clonedVar, layout);
        }

        // TODO: once we support initializers on global variables,
        // we'll need to handle cloning it here.

        return clonedVar;
    }

    void cloneFunctionCommon(
        IRSpecContextBase*  context,
        IRFunc*         clonedFunc,
        IRFunc*         originalFunc)
    {
        // First clone all the simple properties.
        clonedFunc->mangledName = originalFunc->mangledName;
        clonedFunc->genericDecl = originalFunc->genericDecl;
        clonedFunc->type = context->maybeCloneType(originalFunc->type);

        cloneDecorations(context, clonedFunc, originalFunc);

        // Next we are going to clone the actual code.
        IRBuilder builderStorage = *context->builder;
        IRBuilder* builder = &builderStorage;
        builder->func = clonedFunc;

        // We will walk through the blocks of the function, and clone each of them.
        //
        // We need to create the cloned blocks first, and then walk through them,
        // because blocks might be forward referenced (this is not possible
        // for other cases of instructions).
        for (auto originalBlock = originalFunc->getFirstBlock();
            originalBlock;
            originalBlock = originalBlock->getNextBlock())
        {
            IRBlock* clonedBlock = builder->createBlock();
            clonedFunc->addBlock(clonedBlock);
            registerClonedValue(context, clonedBlock, originalBlock);

            // We can go ahead and clone parameters here, while we are at it.
            builder->block = clonedBlock;
            for (auto originalParam = originalBlock->getFirstParam();
                originalParam;
                originalParam = originalParam->getNextParam())
            {
                IRParam* clonedParam = builder->emitParam(
                    context->maybeCloneType(
                        originalParam->getType()));
                registerClonedValue(context, clonedParam, originalParam);
            }
        }

        // Okay, now we are in a good position to start cloning
        // the instructions inside the blocks.
        {
            IRBlock* ob = originalFunc->getFirstBlock();
            IRBlock* cb = clonedFunc->getFirstBlock();
            while (ob)
            {
                assert(cb);

                builder->block = cb;
                for (auto oi = ob->getFirstInst(); oi; oi = oi->nextInst)
                {
                    cloneInst(context, builder, oi);
                }

                ob = ob->getNextBlock();
                cb = cb->getNextBlock();
            }
        }

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
        String mangledName = getMangledName(entryPointRequest->decl);
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

    IRFunc* cloneSimpleFunc(IRSpecContextBase* context, IRFunc* originalFunc)
    {
        auto clonedFunc = context->builder->createFunc();
        registerClonedValue(context, clonedFunc, originalFunc);
        cloneFunctionCommon(context, clonedFunc, originalFunc);
        return clonedFunc;
    }

    // Get a string form of the target so that we can
    // use it to match against target-specialization modifiers
    //
    // TODO: We shouldn't be using strings for this.
    String getTargetName(IRSpecContext* context)
    {
        switch( context->target )
        {
        case CodeGenTarget::HLSL:
            return "hlsl";

        case CodeGenTarget::GLSL:
            return "glsl";

        default:
            SLANG_UNEXPECTED("unhandled case");
            return "unknown";
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
        return UInt(newLevel) > UInt(oldLevel);
    }

    IRFunc* cloneFunc(IRSpecContext* context, IRFunc* originalFunc)
    {
        // We are being asked to clone a particular function, but in
        // the IR that comes out of the front-end there could still
        // be multiple, target-specific, declarations of any given
        // function, all of which share the same mangled name.
        auto mangledName = originalFunc->mangledName;

        if(mangledName.Length() == 0)
        {
            return cloneSimpleFunc(context, originalFunc);
        }

        //
        // We will scan through all of the available function declarations
        // with the same mangled name as `originalFunc` and try
        // to pick the "best" one for our target.

        RefPtr<IRSpecSymbol> sym;
        if( !context->getSymbols().TryGetValue(originalFunc->mangledName, sym) )
        {
            // This shouldn't happen!
            SLANG_UNEXPECTED("no matching function registered");
            return cloneSimpleFunc(context, originalFunc);
        }

        // We will try to track the "best" definition we can find.
        IRFunc* bestFunc = (IRFunc*) sym->irGlobalValue;

        for( auto ss = sym->nextWithSameName; ss; ss = ss->nextWithSameName )
        {
            IRFunc* newFunc = (IRFunc*) ss->irGlobalValue;
            if(isBetterForTarget(context, newFunc, bestFunc))
                bestFunc = newFunc;
        }

        // All right, we are now in a position to clone the "best"
        // definition that was found.
        auto clonedFunc = context->builder->createFunc();

        // The resulting function will be used as the cloned version
        // of every declaration/definition in the original IR.
        for( auto ss = sym; ss; ss = ss->nextWithSameName )
        {
            registerClonedValue(context, clonedFunc, ss->irGlobalValue);
        }

        // Clone the "best" definition into our context
        cloneFunctionCommon(context, clonedFunc, bestFunc);

        return clonedFunc;
    }

    StructTypeLayout* getGlobalStructLayout(
        ProgramLayout*  programLayout);

    void insertGlobalValueSymbol(
        IRSharedSpecContext*    sharedContext,
        IRGlobalValue*          gv)
    {
        String mangledName = gv->mangledName;

        // Don't try to register a symbol for global values
        // with no mangled name, since these represent symbols
        // that shouldn't get "linkage"
        if (mangledName == "")
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

    void initializeSharedSpecContext(
        IRSharedSpecContext*    sharedContext,
        Session*                session,
        IRModule*               module,
        IRModule*               originalModule)
    {

        SharedIRBuilder* sharedBuilder = &sharedContext->sharedBuilderStorage;
        sharedBuilder->module = nullptr;
        sharedBuilder->session = session;

        IRBuilder* builder = &sharedContext->builderStorage;
        builder->shared = sharedBuilder;

        if( !module )
        {
            module = builder->createModule();
            sharedBuilder->module = module;
        }

        sharedContext->module = module;
        sharedContext->originalModule = originalModule;

        // First, we will populate a map with all of the IR values
        // that use the same mangled name, to make lookup easier
        // in other steps.
        for (auto gv = originalModule->firstGlobalValue; gv; gv = gv->nextGlobalValue)
        {
            insertGlobalValueSymbol(sharedContext, gv);
        }
    }

    IRModule* specializeIRForEntryPoint(
        EntryPointRequest*  entryPointRequest,
        ProgramLayout*      programLayout,
        CodeGenTarget       target)
    {
        auto compileRequest = entryPointRequest->compileRequest;
        auto session = compileRequest->mSession;
        auto translationUnit = entryPointRequest->getTranslationUnit();
        auto originalIRModule = translationUnit->irModule;
        if (!originalIRModule)
        {
            // We should already have emitted IR for the original
            // translation unit, and it we don't have it, then
            // we are now in trouble.
            return nullptr;
        }

        auto entryPointLayout = findEntryPointLayout(programLayout, entryPointRequest);

        // We now need to start cloning IR symbols from `originalIRModule`
        // into a fresh IR module for this entry point. Along the way we need to:
        //
        // 1. Attach layout information from `programLayout` and/or `entryPointLayout`
        //    onto the cloned IR symbols, to drive later code generation.
        //
        // 2. In cases where a function might have multiple target-specific definitions,
        //    we need to pick the "best" one for the chosen code generation target.
        //

        IRSharedSpecContext sharedContextStorage;

        initializeSharedSpecContext(
            &sharedContextStorage,
            compileRequest->mSession,
            nullptr,
            originalIRModule);

        IRSpecContext contextStorage;
        IRSpecContext*  context = &contextStorage;
        context->shared = &sharedContextStorage;
        context->builder = &sharedContextStorage.builderStorage;
        context->target = target;


        // Next, we want to optimize lookup for layout infromation
        // associated with global declarations, so that we can 
        // look things up based on the IR values (using mangled names)
        auto globalStructLayout = getGlobalStructLayout(programLayout);
        for (auto globalVarLayout : globalStructLayout->fields)
        {
            String mangledName = getMangledName(globalVarLayout->varDecl);
            context->globalVarLayouts.AddIfNotExists(mangledName, globalVarLayout);
        }

        // Next, we make sure to clone the global value for
        // the entry point function itself, and rely on
        // this step to recursively copy over anything else
        // it might reference.
        auto irEntryPoint = specializeIRForEntryPoint(context, entryPointRequest, entryPointLayout);

        // TODO: *technically* we should consider the case where
        // we have global variables with initializers, since
        // these should get run whether or not the entry point
        // references them.

        // Depending on the downstream target, we may need to apply some
        // guaranteed transformations to legalize things. We will go
        // ahead and apply there here for now.
        switch (target)
        {
        case CodeGenTarget::GLSL:
            {
                legalizeEntryPointForGLSL(session, irEntryPoint, entryPointLayout);
            }
            break;

        default:
            break;
        }

        return sharedContextStorage.module;
    }

    //

    struct IRSharedGenericSpecContext : IRSharedSpecContext
    {
        // Non-generic functions to be processed
        List<IRFunc*> workList;
    };

    struct IRGenericSpecContext : IRSpecContextBase
    {
        IRSharedGenericSpecContext* getShared() { return (IRSharedGenericSpecContext*) shared; }

        // The substutions to apply
        RefPtr<Substitutions>   subst;

        // Override the "maybe clone" logic so that we always clone
        virtual IRValue* maybeCloneValue(IRValue* originalVal) override;

        virtual RefPtr<Type> maybeCloneType(Type* originalType) override;
    };

    IRValue* IRGenericSpecContext::maybeCloneValue(IRValue* originalVal)
    {
        switch( originalVal->op )
        {
        case kIROp_decl_ref:
            {
                auto declRefVal = (IRDeclRef*) originalVal;
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


    IRFunc* getSpecializedFunc(
        IRSharedGenericSpecContext* sharedContext,
        IRFunc*                     genericFunc,
        DeclRef<Decl>               specDeclRef)
    {
        // First, we want to see if an existing specialization
        // has already been made. To do that we will need to
        // compute the mangled name of the specialized function,
        // so that we can look for existing declarations.

        String specMangledName = getMangledName(specDeclRef);

        // TODO: This is a terrible linear search, and we should
        // avoid it by building a dictionary ahead of time,
        // as is being done for the `IRSpecContext` used above.
        // We can probalby use the same basic context, actually.
        auto module = genericFunc->parentModule;
        for(auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue())
        {
            if(gv->mangledName == specMangledName)
                return (IRFunc*) gv;
        }

        // If we get to this point, then we need to construct a
        // new `IRFunc` to represent the result of specialization.

        // The substitutions we are applying might have been created
        // using a different overload of a target-specific function,
        // so we need to create a dummy substitution here, to make
        // sure it used the correct generic.
        RefPtr<Substitutions> newSubst = new Substitutions();
        newSubst->genericDecl = genericFunc->genericDecl;
        newSubst->args = specDeclRef.substitutions->args;

        IRGenericSpecContext context;
        context.shared = sharedContext;
        context.builder = &sharedContext->builderStorage;
        context.subst = newSubst;

        // TODO: other initialization is needed here...

        auto specFunc = cloneSimpleFunc(&context, genericFunc);

        // Set up the clone to recognize that it is no longer generic
        specFunc->mangledName = specMangledName;
        specFunc->genericDecl = nullptr;

        // Put the function into the global sequence right after
        // the function it specializes.
        //
        // TODO: This shouldn't be needed, if we introduce a sorting
        // step before we emit code.
        specFunc->removeFromParent();
        specFunc->insertAfter(genericFunc);

        // At this point we've created a new non-generic function,
        // which means we should add it to our work list for
        // subsequent processing.
        sharedContext->workList.Add(specFunc);

        // We also need to make sure that we register this specialized
        // function under its mangled name, so that later lookup
        // steps will find it.
        insertGlobalValueSymbol(sharedContext, specFunc);

        return specFunc;
    }

    void specializeGenerics(
        IRModule*   module)
    {
        IRSharedGenericSpecContext sharedContextStorage;
        auto sharedContext = &sharedContextStorage;

        initializeSharedSpecContext(
            sharedContext,
            module->session,
            module,
            module);

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
            if(func->genericDecl)
                continue;

            sharedContext->workList.Add(func);
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
                    nextInst = ii->nextInst;

                    // We only care about `specialize` instructions.
                    if(ii->op != kIROp_specialize)
                        continue;

                    IRSpecialize* specInst = (IRSpecialize*) ii;

                    // We need to check that the value being specialized is
                    // a generic function.
                    auto genericVal = specInst->genericVal.usedValue;
                    if(genericVal->op != kIROp_Func)
                        continue;
                    auto genericFunc = (IRFunc*) genericVal;
                    if(!genericFunc->genericDecl)
                        continue;

                    // Now we extract the specialized decl-ref that will
                    // tell us how to specialize things.
                    auto specDeclRefVal = (IRDeclRef*) specInst->specDeclRefVal.usedValue;
                    auto specDeclRef = specDeclRefVal->declRef;

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
            }
        }

        // Once the work list has gone dry, we should have the invariant
        // that there are no `specialize` instructions inside of non-generic
        // functions that in turn reference a generic function.
    }

    //

}
