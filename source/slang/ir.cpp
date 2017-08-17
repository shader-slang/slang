// ir.cpp
#include "ir.h"

#include "../core/basic.h"

namespace Slang
{

#define OP(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    static const IROpInfo kIROpInfo_##ID {  \
        #MNEMONIC, ARG_COUNT, FLAGS, }

#define PARENT kIROpFlag_Parent

    OP(TypeType,    type.type,      0, 0);
    OP(VoidType,    type.void,      0, 0);
    OP(BlockType,   type.block,     0, 0);
    OP(VectorType,  type.vector,    2, 0);
    OP(BoolType,    type.bool,      0, 0);
    OP(Float32Type, type.f32,       0, 0);
    OP(Int32Type,   type.i32,       0, 0);
    OP(UInt32Type,  type.u32,       0, 0);
    OP(StructType,  type.struct,    0, 0);

    OP(IntLit,      integer_constant,   0, 0);
    OP(FloatLit,    float_constant,     0, 0);

    OP(Construct,   construct,         0, 0);

    OP(Module,      module, 0, PARENT);
    OP(Func,        func,   0, PARENT);
    OP(Block,       block,  0, PARENT);

    OP(Param,           param,  0, 0);

    OP(FieldExtract,    get_field,      1, 0);
    OP(ReturnVal,       return_val,     1, 0);
    OP(ReturnVoid, return_void, 1, 0);

#define INTRINSIC(NAME)                     \
    static const IROpInfo kIROpInfo_Intrinsic_##NAME {  \
        "intrinsic." #NAME, 0, 0, };
#include "intrinsic-defs.h"

#undef PARENT
#undef OP


    static IROpInfo const* const kIRIntrinsicOpInfos[] =
    {
        nullptr,

#define INTRINSIC(NAME) &kIROpInfo_Intrinsic_##NAME,
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
        IROpInfo const* op,
        IRType*         type,
        UInt            argCount,
        IRValue* const* args)
    {
        IRValue* inst = (IRInst*) malloc(size);
        memset(inst, 0, size);

        IRUse* instArgs = inst->getArgs();

        auto module = builder->getModule();
        if (!module || (type && type->op == &kIROpInfo_VoidType))
        {
            // Can't or shouldn't assign an ID to this op
        }
        else
        {
            inst->id = ++module->idCounter;
        }
        inst->argCount = argCount + 1;

        inst->op = op;

        inst->type.init(inst, type);

        for( UInt aa = 0; aa < argCount; ++aa )
        {
            instArgs[aa+1].init(inst, args[aa]);
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
        IROpInfo const* op,
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
        IROpInfo const* op,
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
        IROpInfo const* op,
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
        IROpInfo const* op,
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
        IROpInfo const* op,
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
        IROpInfo const* op,
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
        IROpInfo const* op,
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

        if( parent->op == &kIROpInfo_Func )
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
        IROpInfo const* op,
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
        IROpInfo const* op,
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
        IROpInfo const* op,
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
        IROpInfo const* op,
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
        IRBuilder*              builder,
        IROpInfo const*         op,
        IRType*                 type,
        UInt                    valueSize,
        void const*             value)
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

    static IRType* getBaseTypeImpl(IRBuilder* builder, IROpInfo const* op)
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
        case BaseType::Bool:    return getBaseTypeImpl(this, &kIROpInfo_BoolType);
        case BaseType::Float:   return getBaseTypeImpl(this, &kIROpInfo_Float32Type);
        case BaseType::Int:     return getBaseTypeImpl(this, &kIROpInfo_Int32Type);
        case BaseType::UInt:     return getBaseTypeImpl(this, &kIROpInfo_UInt32Type);

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
            &kIROpInfo_VectorType,
            getTypeType(),
            elementType,
            elementCount);
    }

    IRType* IRBuilder::getTypeType()
    {
        return findOrEmitInst<IRType>(
            this,
            &kIROpInfo_TypeType,
            nullptr);
    }

    IRType* IRBuilder::getVoidType()
    {
        return findOrEmitInst<IRType>(
            this,
            &kIROpInfo_VoidType,
            getTypeType());
    }

    IRType* IRBuilder::getBlockType()
    {
        return findOrEmitInst<IRType>(
            this,
            &kIROpInfo_BlockType,
            getTypeType());
    }

    IRType* IRBuilder::getStructType(
        UInt            fieldCount,
        IRType* const*  fieldTypes)
    {
        auto inst = createInstWithTrailingArgs<IRStructType>(
            this,
            &kIROpInfo_StructType,
            getTypeType(),
            fieldCount,
            (IRValue* const*)fieldTypes);
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
            &kIROpInfo_IntLit,
            type,
            sizeof(value),
            &value);
    }

    IRValue* IRBuilder::getFloatValue(IRType* type, IRFloatingPointValue value)
    {
        return findOrEmitConstant(
            this,
            &kIROpInfo_FloatLit,
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
            kIRIntrinsicOpInfos[(int)intrinsicOp],
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
            &kIROpInfo_Construct,
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
            &kIROpInfo_Module,
            nullptr);
    }


    IRFunc* IRBuilder::createFunc()
    {
        return createInst<IRFunc>(
            this,
            &kIROpInfo_Func,
            nullptr);
    }

    IRBlock* IRBuilder::createBlock()
    {
        return createInst<IRBlock>(
            this,
            &kIROpInfo_Block,
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
            &kIROpInfo_Param,
            type);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitFieldExtract(
        IRType*     type,
        IRValue*    base,
        UInt        fieldIndex)
    {
        auto inst = createInst<IRFieldExtract>(
            this,
            &kIROpInfo_FieldExtract,
            type,
            base);

        inst->fieldIndex = fieldIndex;

        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitReturn(
        IRValue*    val)
    {
        auto inst = createInst<IRReturnVal>(
            this,
            &kIROpInfo_ReturnVal,
            getVoidType(),
            val);
        addInst(inst);
        return inst;
    }

    IRInst* IRBuilder::emitReturn()
    {
        auto inst = createInst<IRReturnVoid>(
            this,
            &kIROpInfo_ReturnVoid,
            getVoidType());
        addInst(inst);
        return inst;
    }

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

        if (inst->id)
        {
            dumpID(context, inst);
            dump(context, " = ");
        }

        dump(context, op->name);

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

        if (op->flags & kIROpFlag_Parent)
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
