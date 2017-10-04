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
        return new IRModule();
    }


    IRFunc* IRBuilder::createFunc()
    {
        return createValue<IRFunc>(
            this,
            kIROp_Func,
            nullptr);
    }

    IRGlobalVar* IRBuilder::createGlobalVar(
        IRType* valueType)
    {
        auto ptrType = getSession()->getPtrType(valueType);
        return createValue<IRGlobalVar>(
            this,
            kIROp_global_var,
            ptrType);
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
        IRType*         type,
        IRAddressSpace  addressSpace)
    {
        auto allocatedType = getSession()->getPtrType(type);
        auto inst = createInst<IRVar>(
            this,
            kIROp_Var,
            allocatedType);
        addInst(inst);
        return inst;
    }


    IRVar* IRBuilder::emitVar(
        IRType* type)
    {
        return emitVar(type, kIRAddressSpace_Default);
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
        for (auto gv : module->globalValues)
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


}
