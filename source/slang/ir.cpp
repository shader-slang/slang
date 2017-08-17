// ir.cpp
#include "ir.h"

#include "../core/basic.h"

namespace Slang
{

    static const IROpInfo kIROpInfos[] =
    {
#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    { #MNEMONIC, ARG_COUNT, FLAGS, },
#include "ir-inst-defs.h"
    };


    static const IROp kIRIntrinsicOps[] =
    {
        (IROp) 0,

#define INTRINSIC(NAME) kIROp_Intrinsic_##NAME,
#include "intrinsic-defs.h"

    };

    //

    void IRUse::init(IRValue* u, IRValue* v)
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
        return &type;
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

    //

    IRParam* IRFunc::getFirstParam()
    {
        auto entryBlock = getFirstBlock();
        if(!entryBlock) return nullptr;

        auto firstInst = entryBlock->firstChild;
        if(!firstInst) return nullptr;

        if(firstInst->op != kIROp_Param)
            return nullptr;

        return (IRParam*) firstInst;
    }

    IRParam* IRParam::getNextParam()
    {
        auto next = nextInst;
        if(!next) return nullptr;

        if(next->op != kIROp_Param)
            return nullptr;

        return (IRParam*) next;
    }

    //

    // Add an instruction to a specific parent
    void IRBuilder::addInst(IRParentInst* parent, IRInst* inst)
    {
        inst->parent = parent;

        if (!parent->firstChild)
        {
            inst->prevInst = nullptr;
            inst->nextInst = nullptr;

            parent->firstChild = inst;
            parent->lastChild = inst;
        }
        else
        {
            auto prev = parent->lastChild;

            inst->prevInst = prev;
            inst->nextInst = nullptr;

            prev->nextInst = inst;
            parent->lastChild = inst;
        }
    }

    // Add an instruction into the current scope
    void IRBuilder::addInst(
        IRInst*     inst)
    {
        auto parent = parentInst;
        if (!parent)
            return;

        addInst(parent, inst);
    }

    // Create an IR instruction/value and initialize it.
    //
    // In this case `argCount` and `args` represnt the
    // arguments *after* the type (which is a mandatory
    // argument for all instructions).
    static IRValue* createInstImpl(
        IRBuilder*      builder,
        UInt            size,
        IROp            op,
        IRType*         type,
        UInt            fixedArgCount,
        IRValue* const* fixedArgs,
        UInt            varArgCount = 0,
        IRValue* const* varArgs = nullptr)
    {
        IRValue* inst = (IRInst*) malloc(size);
        memset(inst, 0, size);

        auto module = builder->getModule();
        if (!module || (type && type->op == kIROp_VoidType))
        {
            // Can't or shouldn't assign an ID to this op
        }
        else
        {
            inst->id = ++module->idCounter;
        }
        inst->argCount = fixedArgCount + varArgCount + 1;

        inst->op = op;

        auto operand = inst->getArgs();

        operand->init(inst, type);
        operand++;

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
            if(leftArgs[aa].usedValue != rightArgs[aa].usedValue)
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
            code = combineHash(code, Slang::GetHashCode(args[aa].usedValue));
        }
        return code;
    }

    static IRParentInst* joinParentInstsForInsertion(
        IRParentInst*   left,
        IRParentInst*   right)
    {
        // Are they the same? Easy.
        if(left == right) return left;

        // Have we already failed to find a location? Then bail.
        if(!left) return nullptr;
        if(!right) return nullptr;

        // Is one inst a parent of the other? Pick the child.
        for( auto ll = left; ll; ll = ll->parent )
        {
            // Did we find the right node in the parent list of the left?
            if(ll == right) return left;
        }
        for( auto rr = right; rr; rr = rr->parent )
        {
            // Did we find the left node in the parent list of the right?
            if(rr == left) return right;
        }

        // Seems like they are unrelated, so we should play it safe
        return nullptr;
    }


    static IRInst* findOrEmitInstImpl(
        IRBuilder*      builder,
        UInt            size,
        IROp            op,
        IRType*         type,
        UInt            argCount,
        IRValue* const* args)
    {
        // First, we need to pick a good insertion point
        // for the instruction, which we do by looking
        // at its operands.
        //

        IRParentInst* parent = builder->shared->module;
        if( type )
        {
            parent = joinParentInstsForInsertion(parent, type->parent);
        }
        for( UInt aa = 0; aa < argCount; ++aa )
        {
            auto arg = args[aa];
            parent = joinParentInstsForInsertion(parent, arg->parent);
        }

        // If we failed to find a good insertion point, then insert locally.
        if( !parent )
        {
            parent = builder->parentInst;
        }

        if( parent->op == kIROp_Func )
        {
            // We are trying to insert into a function, and we should really
            // be inserting into its entry block.
            assert(parent->firstChild);
            parent = (IRBlock*) ((IRFunc*) parent)->firstChild;
        }

        // We now know where we want to insert, but there might
        // already be an equivalent instruction in that block.
        //
        // We will check for such an instruction in a slightly hacky
        // way: we will construct a temporary instruction and
        // then use it to look up in a cache of instructions.

        IRInst* keyInst = createInstImpl(builder, size, op, type, argCount ,args);
        keyInst->parent = parent;

        IRInstKey key;
        key.inst = keyInst;

        IRInst* inst = nullptr;
        if( builder->shared->globalValueNumberingMap.TryGetValue(key, inst) )
        {
            // We found a match, so just use that.

            free(keyInst);
            return inst;
        }

        // No match, so use our "key" instruction for real.
        inst = keyInst;

        builder->shared->globalValueNumberingMap.Add(key, inst);

        keyInst->parent = nullptr;
        builder->addInst(parent, inst);

        return inst;
    }

    template<typename T>
    static T* findOrEmitInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        UInt            argCount,
        IRValue* const* args)
    {
        return (T*) findOrEmitInstImpl(
            builder,
            sizeof(T),
            op,
            type,
            argCount,
            args);
    }

    template<typename T>
    static T* findOrEmitInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type)
    {
        return (T*) findOrEmitInstImpl(
            builder,
            sizeof(T),
            op,
            type,
            0,
            nullptr);
    }

    template<typename T>
    static T* findOrEmitInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        IRInst*         arg)
    {
        return (T*) findOrEmitInstImpl(
            builder,
            sizeof(T),
            op,
            type,
            1,
            &arg);
    }

    template<typename T>
    static T* findOrEmitInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        IRInst*         arg1,
        IRInst*         arg2)
    {
        IRInst* args[] = { arg1, arg2 };
        return (T*) findOrEmitInstImpl(
            builder,
            sizeof(T),
            op,
            type,
            2,
            &args[0]);
    }

    //

    bool operator==(IRConstantKey const& left, IRConstantKey const& right)
    {
        if(left.inst->op             != right.inst->op)             return false;
        if(left.inst->type.usedValue != right.inst->type.usedValue) return false;
        if(left.inst->u.ptrData[0]   != right.inst->u.ptrData[0])   return false;
        if(left.inst->u.ptrData[1]   != right.inst->u.ptrData[1])   return false;
        return true;
    }

    int IRConstantKey::GetHashCode()
    {
        auto code = Slang::GetHashCode(inst->op);
        code = combineHash(code, Slang::GetHashCode(inst->type.usedValue));
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

        IRParentInst* parent = builder->shared->module;

        IRConstant keyInst;
        keyInst.op = op;
        keyInst.type.usedValue = type;
        memcpy(&keyInst.u, value, valueSize);

        IRConstantKey key;
        key.inst = &keyInst;

        IRConstant* inst = nullptr;
        if( builder->shared->constantMap.TryGetValue(key, inst) )
        {
            // We found a match, so just use that.
            return inst;
        }

        // We now know where we want to insert, but there might
        // already be an equivalent instruction in that block.
        //
        // We will check for such an instruction in a slightly hacky
        // way: we will construct a temporary instruction and
        // then use it to look up in a cache of instructions.

        inst = createInst<IRConstant>(builder, op, type);
        memcpy(&inst->u, value, valueSize);

        key.inst = inst;
        builder->shared->constantMap.Add(key, inst);

        builder->addInst(parent, inst);

        return inst;
    }


    //

    static IRType* getBaseTypeImpl(IRBuilder* builder, IROp op)
    {
        auto inst = findOrEmitInst<IRType>(
            builder,
            op,
            builder->getTypeType());
        return inst;
    }

    IRType* IRBuilder::getBaseType(BaseType flavor)
    {
        switch( flavor )
        {
        case BaseType::Bool:    return getBaseTypeImpl(this, kIROp_BoolType);
        case BaseType::Float:   return getBaseTypeImpl(this, kIROp_Float32Type);
        case BaseType::Int:     return getBaseTypeImpl(this, kIROp_Int32Type);
        case BaseType::UInt:     return getBaseTypeImpl(this, kIROp_UInt32Type);

        default:
            SLANG_UNEXPECTED("unhandled base type");
            return nullptr;
        }
    }

    IRType* IRBuilder::getBoolType()
    {
        return getBaseType(BaseType::Bool);
    }

    IRType* IRBuilder::getVectorType(IRType* elementType, IRValue* elementCount)
    {
        return findOrEmitInst<IRVectorType>(
            this,
            kIROp_VectorType,
            getTypeType(),
            elementType,
            elementCount);
    }

    IRType* IRBuilder::getTypeType()
    {
        return findOrEmitInst<IRType>(
            this,
            kIROp_TypeType,
            nullptr);
    }

    IRType* IRBuilder::getVoidType()
    {
        return findOrEmitInst<IRType>(
            this,
            kIROp_VoidType,
            getTypeType());
    }

    IRType* IRBuilder::getBlockType()
    {
        return findOrEmitInst<IRType>(
            this,
            kIROp_BlockType,
            getTypeType());
    }

    IRStructDecl* IRBuilder::createStructType()
    {
        return createInst<IRStructDecl>(
            this,
            kIROp_StructType,
            getTypeType());
    }

    IRStructField* IRBuilder::createStructField(IRType* fieldType)
    {
        return createInst<IRStructField>(
            this,
            kIROp_StructField,
            fieldType);
    }


    IRType* IRBuilder::getFuncType(
        UInt            paramCount,
        IRType* const*  paramTypes,
        IRType*         resultType)
    {
        auto inst = createInstWithTrailingArgs<IRFuncType>(
            this,
            kIROp_FuncType,
            getTypeType(),
            1,
            (IRValue* const*) &resultType,
            paramCount,
            (IRValue* const*) paramTypes);
        addInst(inst);
        return inst;
    }

    IRValue* IRBuilder::getBoolValue(bool value)
    {
        SLANG_UNIMPLEMENTED_X("IR");
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

    IRInst* IRBuilder::emitIntrinsicInst(
        IRType*         type,
        IntrinsicOp     intrinsicOp,
        UInt            argCount,
        IRValue* const* args)
    {
        auto inst = createInstWithTrailingArgs<IRInst>(
            this,
            kIRIntrinsicOps[(int)intrinsicOp],
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
        return createInst<IRModule>(
            this,
            kIROp_Module,
            nullptr);
    }


    IRFunc* IRBuilder::createFunc()
    {
        return createInst<IRFunc>(
            this,
            kIROp_Func,
            nullptr);
    }

    IRBlock* IRBuilder::createBlock()
    {
        return createInst<IRBlock>(
            this,
            kIROp_Block,
            getBlockType());
    }

    IRBlock* IRBuilder::emitBlock()
    {
        auto inst = createBlock();
        addInst(inst);
        return inst;
    }

    IRParam* IRBuilder::emitParam(
        IRType* type)
    {
        auto inst = createInst<IRParam>(
            this,
            kIROp_Param,
            type);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitFieldExtract(
        IRType*         type,
        IRValue*        base,
        IRStructField*  field)
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

    IRInst* IRBuilder::emitReturn(
        IRValue*    val)
    {
        auto inst = createInst<IRReturnVal>(
            this,
            kIROp_ReturnVal,
            getVoidType(),
            val);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitReturn()
    {
        auto inst = createInst<IRReturnVoid>(
            this,
            kIROp_ReturnVoid,
            getVoidType());
        addInst(inst);
        return inst;
    }

    IRDecoration* IRBuilder::addDecorationImpl(
        IRInst*         inst,
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

    IRHighLevelDeclDecoration* IRBuilder::addHighLevelDeclDecoration(IRInst* inst, Decl* decl)
    {
        auto decoration = addDecoration<IRHighLevelDeclDecoration>(inst, kIRDecorationOp_HighLevelDecl);
        decoration->decl = decl;
        return decoration;
    }

    //


    struct IRDumpContext
    {
        FILE*   file;
        int     indent;
    };

    static void dump(
        IRDumpContext*  context,
        char const*     text)
    {
        fprintf(context->file, "%s", text);
    }

    static void dump(
        IRDumpContext*  context,
        UInt            val)
    {
        fprintf(context->file, "%llu", (unsigned long long)val);
    }

    static void dumpIndent(
        IRDumpContext*  context)
    {
        for (int ii = 0; ii < context->indent; ++ii)
        {
            dump(context, "  ");
        }
    }

    static void dumpID(
        IRDumpContext* context,
        IRInst*         inst)
    {
        if (!inst)
        {
            dump(context, "<null>");
        }
        else
        {
            dump(context, "%");
            dump(context, inst->id);
        }
    }

    static void dumpInst(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        dumpIndent(context);
        if (!inst)
        {
            dump(context, "<null>");
        }

        // TODO: need to display a name for the result...

        auto op = inst->op;
        auto opInfo = &kIROpInfos[op];

        if (inst->id)
        {
            dumpID(context, inst);
            dump(context, " = ");
        }

        dump(context, opInfo->name);

        // TODO: dump operands
        uint32_t argCount = inst->argCount;
        for (uint32_t ii = 0; ii < argCount; ++ii)
        {
            if (ii != 0)
                dump(context, ", ");
            else
            {
                dump(context, " ");
            }

            auto argVal = inst->getArgs()[ii].usedValue;

            // TODO: actually print the damn operand...

            dumpID(context, argVal);
        }

        dump(context, "\n");

        if (opInfo->flags & kIROpFlag_Parent)
        {
            dumpIndent(context);
            dump(context, "{\n");
            context->indent++;
            auto parent = (IRParentInst*)inst;
            for (auto ii = parent->firstChild; ii; ii = ii->nextInst)
            {
                dumpInst(context, ii);
            }
            context->indent--;
            dumpIndent(context);
            dump(context, "}\n");
        }
    }

    void dumpIR(IRModule* module)
    {
        IRDumpContext context;
        context.file = stderr;
        context.indent = 0;

        dumpInst(&context, module);
    }

}
