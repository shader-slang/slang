// ir.cpp
#include "ir.h"
#include "ir-insts.h"

#include "../core/basic.h"

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
            operand->init(inst, fixedArgs[aa]);
            operand++;
        }

        for( UInt aa = 0; aa < varArgCount; ++aa )
        {
            operand->init(inst, varArgs[aa]);
            operand++;
        }

        return inst;
    }

    // Create an IR instruction/value and initialize it.
    //
    // For this overload, the type of the instruction is
    // folded into the argument list (so `args[0]` needs
    // to be the type of the instruction)
    static IRValue* createInstImpl(
        IRBuilder*      builder,
        UInt            size,
        IROp            op,
        UInt            argCount,
        IRValue* const* args)
    {
        return createInstImpl(
            builder,
            size,
            op,
            (IRType*) args[0],
            argCount - 1,
            args + 1);
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
        irValue->declRef = declRef;
        return irValue;
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
        if(genericParentDeclRef)
        {
            parentDeclRef = genericParentDeclRef.GetParent();
        }

        if(parentDeclRef.As<ModuleDecl>())
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
        switch (op)
        {
#if 0
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
#endif

        default:
            break;
        }

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

    void dumpIRFunc(
        IRDumpContext*  context,
        IRFunc*         func)
    {
        dump(context, "\n");
        dumpIndent(context);
        dump(context, "ir_func ");
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

    // Remove this instruction from its parent block,
    // and then destroy it (it had better have no uses!)
    void IRInst::removeAndDeallocate()
    {
        removeFromParent();
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

    void legalizeEntryPointForGLSL(
        Session*                session,
        IRFunc*                 func,
        IREntryPointDecoration* entryPointInfo)
    {
        auto module = func->parentModule;

        auto entryPointLayout = entryPointInfo->layout;

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

            auto resultVariable = addGlobalVariable(module, resultType);
            moveValueBefore(resultVariable, func);

            // We need to transfer layout information from the entry point
            // down to the variable:
            builder.addLayoutDecoration(resultVariable, entryPointLayout->resultLayout);

            for( auto bb = func->getFirstBlock(); bb; bb = bb->getNextBlock() )
            {
                for( auto ii = bb->getFirstInst(); ii; ii = ii->nextInst )
                {
                    if(ii->op != kIROp_ReturnVal)
                        continue;

                    IRReturnVal* returnInst = (IRReturnVal*) ii;
                    IRValue* resultValue = returnInst->getVal();



                    // `store <resultVariable> <resultValue>`
                    IRStore* storeInst = createInst<IRStore>(
                        &builder,
                        kIROp_Store,
                        nullptr,
                        resultVariable,
                        resultValue);

                    // `returnVoid`
                    IRReturnVoid* returnVoid = createInst<IRReturnVoid>(
                        &builder,
                        kIROp_ReturnVoid,
                        nullptr);

                    // Put the two new instructions before the old one
                    storeInst->insertBefore(returnInst);
                    returnVoid->insertBefore(returnInst);

                    // and then remove the old one.
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
            IRInst* insertBeforeInst = firstBlock->getFirstInst();

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

                // TODO: We need to distinguish any true pointers in the
                // user's code from pointers that only exist for
                // parameter-passing. This `PtrType` here should actually
                // be `OutTypeBase`, but I'm not confident that all
                // the other code is handling that correctly...
                if(auto paramPtrType = paramType->As<PtrType>() )
                {
                    // Okay, we have the more interesting case here,
                    // where the parameter was being passed by reference.
                    // This actually makes our life pretty easy, though,
                    // since we can simply replace any uses of the existing
                    // pointer with the global variable (since it will
                    // be a pointer to storage).

                    // We start by creating the global variable, using
                    // the pointed-to type:
                    auto valueType = paramPtrType->getValueType();
                    auto paramVariable = addGlobalVariable(module, paramType);
                    moveValueBefore(paramVariable, func);

                    // TODO: We need to special-case `in out` variables here,
                    // because they actually need to be lowered to *two*
                    // global variables, not just one. We then need
                    // to emit logic to initialize the output variable
                    // based on the input at the start of the entry point,
                    // and then use the output variable thereafter.
                    //
                    // TODO: Actually, I need to double-check that it is
                    // legal in GLSL to use shader input/output parameters
                    // as temporaries in general; if not then we'd need
                    // to introduce a temporary no matter what.

                    // Next we attach the layout information from the
                    // original parameter to the new global variable,
                    // so that we can lay it out correctly when generating
                    // target code:
                    builder.addLayoutDecoration(paramVariable, paramLayout);

                    // And finally, we go ahead and replace all the
                    // uses of the parameter (which was a pointer) with
                    // uses of the new global variable's address.
                    pp->replaceUsesWith(paramVariable);
                }
                else
                {
                    // This is the "easy" case where the parameter wasn't
                    // being passed by reference. We start by just creating
                    // a variable of the appropriate type, and attaching
                    // the required layout information to it.
                    auto paramVariable = addGlobalVariable(module, paramType);
                    moveValueBefore(paramVariable, func);
                    builder.addLayoutDecoration(paramVariable, paramLayout);

                    // Next we need to replace uses of the parameter with
                    // references to the variable. We are going to do that
                    // somewhat naively, by simply loading the variable
                    // at the start.

                    IRInst* loadInst = builder.emitLoad(paramVariable);
                    loadInst->insertBefore(insertBeforeInst);

                    pp->replaceUsesWith(loadInst);
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
        voidFuncType->resultType = session->getVoidType();
        func->type = voidFuncType;

        // TODO: we should technically be constructing
        // a new `EntryPointLayout` here to reflect
        // the way that things have been moved around.
    }

    void legalizeEntryPointsForGLSL(
        Session*    session,
        IRModule*   module)
    {
        // We need to walk through all the global entry point
        // declarations, and transform them to comply with
        // GLSL rules.
        for( auto globalValue = module->getFirstGlobalValue(); globalValue; globalValue = globalValue->getNextValue())
        {
            // Is the global value a function?
            if(globalValue->op != kIROp_Func)
                continue;
            IRFunc* func = (IRFunc*) globalValue;

            // Is the function an entry point?
            IREntryPointDecoration* entryPointDecoration = func->findDecoration<IREntryPointDecoration>();
            if(!entryPointDecoration)
                continue;

            // Okay, we need to legalize this one.
            legalizeEntryPointForGLSL(session, func, entryPointDecoration);
        }
    }


}
