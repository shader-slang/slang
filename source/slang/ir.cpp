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

    // Add an instruction into the current scope
    static void addInst(
        IRBuilder*  builder,
        IRInst*     inst)
    {
        auto parent = builder->parentInst;
        if (!parent)
            return;

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

        auto module = builder->module;
        if (!module || (type && type->op == &kIROpInfo_VoidType))
        {
            // Can't or shouldn't assign an ID to this op
        }
        else
        {
            inst->id = ++module->idCounter;
        }

        inst->op = op;

        inst->type.init(inst, type);

        for( UInt aa = 0; aa < argCount; ++aa )
        {
            instArgs[aa+1].init(inst, args[aa]);
        }

        addInst(builder, inst);

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
    static T* createValueWithTrailingArgs(
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

    static IRType* getBaseTypeImpl(IRBuilder* builder, IROpInfo const* op)
    {
        return createInst<IRType>(
            builder,
            op,
            builder->getTypeType());
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
        // TODO: should unique things
         return createInst<IRVectorType>(
            this,
            &kIROpInfo_VectorType,
            getTypeType(),
            elementType,
            elementCount);
    }

    IRType* IRBuilder::getTypeType()
    {
        // TODO: should unique things
        IRType* type = createInst<IRType>(
            this,
            &kIROpInfo_TypeType,
            nullptr);

        // TODO: we need some way to stop this recursion,
        // but just saying that `Type isa Type` is unfounded.
        type->type.init(type, type);

        return type;
    }

    IRType* IRBuilder::getVoidType()
    {
        return createInst<IRType>(
            this,
            &kIROpInfo_VoidType,
            getTypeType());
    }

    IRType* IRBuilder::getBlockType()
    {
        return createInst<IRType>(
            this,
            &kIROpInfo_BlockType,
            getTypeType());
    }

    IRType* IRBuilder::getStructType(
        UInt            fieldCount,
        IRType* const*  fieldTypes)
    {
        return createValueWithTrailingArgs<IRStructType>(
            this,
            &kIROpInfo_StructType,
            getTypeType(),
            fieldCount,
            (IRValue* const*)fieldTypes);
    }

    IRValue* IRBuilder::getBoolValue(bool value)
    {
        SLANG_UNIMPLEMENTED_X("IR");
    }

    IRValue* IRBuilder::getIntValue(IRType* type, IRIntegerValue value)
    {
        IRIntLit* val = createInst<IRIntLit>(
            this,
            &kIROpInfo_IntLit,
            type);

        val->value = value;

        return val;
    }

    IRValue* IRBuilder::getFloatValue(IRType* type, IRFloatingPointValue value)
    {
        SLANG_UNIMPLEMENTED_X("IR");
    }

    IRInst* IRBuilder::emitIntrinsicInst(
        IRType*         type,
        IntrinsicOp     intrinsicOp,
        UInt            argCount,
        IRValue* const* args)
    {
        return createValueWithTrailingArgs<IRInst>(
            this,
            kIRIntrinsicOpInfos[(int)intrinsicOp],
            type,
            argCount,
            args);
    }

    IRInst* IRBuilder::emitConstructorInst(
        IRType*         type,
        UInt            argCount,
        IRValue* const* args)
    {
        return createValueWithTrailingArgs<IRInst>(
            this,
            &kIROpInfo_Construct,
            type,
            argCount,
            args);
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

    IRParam* IRBuilder::createParam(
        IRType* type)
    {
        return createInst<IRParam>(
            this,
            &kIROpInfo_Param,
            type);
    }

    IRInst* IRBuilder::createFieldExtract(
        IRType*     type,
        IRValue*    base,
        UInt        fieldIndex)
    {
        IRFieldExtract* irInst = createInst<IRFieldExtract>(
            this,
            &kIROpInfo_FieldExtract,
            type,
            base);

        irInst->fieldIndex = fieldIndex;

        return irInst;
    }

    IRInst* IRBuilder::createReturn(
        IRValue*    val)
    {
        return createInst<IRReturnVal>(
            this,
            &kIROpInfo_ReturnVal,
            getVoidType(),
            val);
    }

    IRInst* IRBuilder::createReturn()
    {
        return createInst<IRReturnVoid>(
            this,
            &kIROpInfo_ReturnVoid,
            getVoidType());
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
        unsigned argCount = op->fixedArgCount + 1;
        for (unsigned ii = 0; ii < argCount; ++ii)
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
