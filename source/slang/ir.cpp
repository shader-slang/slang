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

    // IRFunc

    IRType* IRFunc::getResultType() { return getType()->getResultType(); }
    UInt IRFunc::getParamCount() { return getType()->getParamCount(); }
    IRType* IRFunc::getParamType(UInt index) { return getType()->getParamType(index); }


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

    // IRParam

    IRParam* IRParam::getNextParam()
    {
        // TODO: this is written as a search because we don't
        // currently do the careful thing and emit parameters
        // before any other members of a block.
        //
        // This should change on the emit side, instead.

        auto next = nextInst;

        for (;;)
        {
            if (!next) return nullptr;

            if(next->op == kIROp_Param)
                return (IRParam*) next;

            next = next->nextInst;
        }
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
        UInt            fixedArgCount,
        IRValue* const* fixedArgs,
        UInt            varArgCount = 0,
        IRValue* const* varArgs     = nullptr)
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
        for( UInt aa = 0; aa < fixedArgCount; ++aa )
        {
            auto arg = fixedArgs[aa];
            parent = joinParentInstsForInsertion(parent, arg->parent);
        }
        for( UInt aa = 0; aa < varArgCount; ++aa )
        {
            auto arg = varArgs[aa];
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

        IRInst* keyInst = createInstImpl(builder, size, op, type, fixedArgCount, fixedArgs, varArgCount, varArgs);
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
        IRType*         type,
        UInt            fixedArgCount,
        IRValue* const* fixedArgs,
        UInt            varArgCount,
        IRValue* const* varArgs)
    {
        return (T*) findOrEmitInstImpl(
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

    template<typename T>
    static T* findOrEmitInst(
        IRBuilder*      builder,
        IROp            op,
        IRType*         type,
        IRInst*         arg1,
        IRInst*         arg2,
        IRInst*         arg3)
    {
        IRInst* args[] = { arg1, arg2, arg3 };
        return (T*) findOrEmitInstImpl(
            builder,
            sizeof(T),
            op,
            type,
            3,
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
        memset(&keyInst, 0, sizeof(keyInst));
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
        case BaseType::Void:    return getVoidType();

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

    IRType* IRBuilder::getMatrixType(
        IRType* elementType,
        IRValue* rowCount,
        IRValue* columnCount)
    {
        return findOrEmitInst<IRMatrixType>(
            this,
            kIROp_MatrixType,
            getTypeType(),
            elementType,
            rowCount,
            columnCount);
    }

    IRType* IRBuilder::getArrayType(IRType* elementType, IRValue* elementCount)
    {
        // The client requests an unsized array by passing `nullptr` for
        // the `elementCount`.
        //
        // We currently encode an unsized array as an ordinary array with
        // zero elements. TODO: carefully consider this choice.
        if (!elementCount)
        {
            elementCount = getIntValue(
                getBaseType(BaseType::Int),
                0);
        }

        return findOrEmitInst<IRArrayType>(
            this,
            kIROp_arrayType,
            getTypeType(),
            elementType,
            elementCount);
    }

    IRType* IRBuilder::getArrayType(IRType* elementType)
    {
        return getArrayType(elementType, nullptr);
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

    IRType* IRBuilder::getIntrinsicType(
        IROp op,
        UInt argCount,
        IRValue* const* args)
    {
        return findOrEmitInst<IRType>(
            this,
            op,
            getTypeType(),
            0,
            nullptr,
            argCount,
            args);
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
        // TODO: need to unique things here!
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

    IRType* IRBuilder::getPtrType(
        IRType*         valueType,
        IRAddressSpace  addressSpace)
    {
        auto uintType = getBaseType(BaseType::UInt);
        auto irAddressSpace = getIntValue(uintType, addressSpace);

        auto inst = findOrEmitInst<IRPtrType>(
            this,
            kIROp_PtrType,
            getTypeType(),
            valueType,
            irAddressSpace);
        return inst;
    }


    IRType* IRBuilder::getPtrType(
        IRType* valueType)
    {
        return getPtrType(valueType, kIRAddressSpace_Default);
    }


    IRValue* IRBuilder::getBoolValue(bool inValue)
    {
        IRIntegerValue value = inValue;
        return findOrEmitConstant(
            this,
            kIROp_boolConst,
            getBoolType(),
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

    IRVar* IRBuilder::emitVar(
        IRType*         type,
        IRAddressSpace  addressSpace)
    {
        auto allocatedType = getPtrType(type, addressSpace);
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
        auto ptrType = ptr->getType();
        if( ptrType->op != kIROp_PtrType )
        {
            // Bad!
            return nullptr;
        }

        auto valueType = ((IRPtrType*) ptrType)->getValueType();

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
        auto type = getVoidType();
        auto inst = createInst<IRStore>(
            this,
            kIROp_Store,
            type,
            dstPtr,
            srcVal);

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

    IRInst* IRBuilder::emitFieldAddress(
        IRType*         type,
        IRValue*        base,
        IRStructField*  field)
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
        auto intType = getBaseType(BaseType::Int);

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
        auto intType = getBaseType(BaseType::Int);

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

    IRInst* IRBuilder::emitBranch(
        IRBlock*    block)
    {
        auto inst = createInst<IRUnconditionalBranch>(
            this,
            kIROp_unconditionalBranch,
            getVoidType(),
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
            getVoidType(),
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
            getVoidType(),
            target);
        addInst(inst);
        return inst;
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
            getVoidType(),
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
        IRInst* args[] = { val, trueBlock, falseBlock };
        UInt argCount = sizeof(args) / sizeof(args[0]);

        auto inst = createInst<IRConditionalBranch>(
            this,
            kIROp_conditionalBranch,
            getVoidType(),
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
        IRInst* args[] = { val, trueBlock, afterBlock };
        UInt argCount = sizeof(args) / sizeof(args[0]);

        auto inst = createInst<IRIf>(
            this,
            kIROp_if,
            getVoidType(),
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
        IRInst* args[] = { val, trueBlock, falseBlock, afterBlock };
        UInt argCount = sizeof(args) / sizeof(args[0]);

        auto inst = createInst<IRIfElse>(
            this,
            kIROp_ifElse,
            getVoidType(),
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
        IRInst* args[] = { val, bodyBlock, breakBlock };
        UInt argCount = sizeof(args) / sizeof(args[0]);

        auto inst = createInst<IRLoopTest>(
            this,
            kIROp_loopTest,
            getVoidType(),
            argCount,
            args);
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

    IRLayoutDecoration* IRBuilder::addLayoutDecoration(IRInst* inst, Layout* layout)
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

    static void dumpID(
        IRDumpContext* context,
        IRInst*         inst)
    {
        if (!inst)
        {
            dump(context, "<null>");
        }
        else if(inst->id)
        {
            dump(context, "%");
            dump(context, (UInt) inst->id);
        }
        else
        {
            dump(context, "_");
        }
    }

    static void dumpType(
        IRDumpContext*  context,
        IRType*         type);

    static void dumpOperand(
        IRDumpContext*  context,
        IRInst*         inst)
    {
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

        auto type = inst->getType();
        if (type)
        {
            switch (type->op)
            {
            case kIROp_TypeType:
                dumpType(context, (IRType*)inst);
                return;

            default:
                break;
            }
        }


        dumpID(context, inst);
    }

    static void dumpType(
        IRDumpContext*  context,
        IRType*         type)
    {
        if (!type)
        {
            dumpID(context, type);
            return;
        }

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
        IRParentInst*   parent)
    {
        for (auto ii = parent->firstChild; ii; ii = ii->nextInst)
        {
            dumpInst(context, ii);
        }
    }

    static void dumpChildren(
        IRDumpContext*  context,
        IRInst*         inst)
    {
        auto op = inst->op;
        auto opInfo = &kIROpInfos[op];
        if (opInfo->flags & kIROpFlag_Parent)
        {
            dumpIndent(context);
            dump(context, "{\n");
            context->indent++;
            auto parent = (IRParentInst*)inst;
            dumpChildrenRaw(context, parent);
            context->indent--;
            dumpIndent(context);
            dump(context, "}\n");
        }
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
                dump(context, "(\n");
                context->indent++;
                for (auto pp = func->getFirstParam(); pp; pp = pp->getNextParam())
                {
                    if (pp != func->getFirstParam())
                        dump(context, ",\n");

                    dumpIndent(context);
                    dump(context, "param ");
                    dumpID(context, pp);
                    dumpInstTypeClause(context, pp->getType());
                }
                context->indent--;
                dump(context, ")\n");

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
                dump(context, ":\n");
                context->indent++;

                dumpChildrenRaw(context, block);
            }
            return;

        default:
            break;
        }

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


        // Okay, we have a seemingly "ordinary" op now
        dumpIndent(context);

        auto opInfo = &kIROpInfos[op];

        if (type && type->op == kIROp_TypeType)
        {
            dump(context, "type ");
            dumpID(context, inst);
            dump(context, "\t= ");
        }
        else if (type && type->op == kIROp_VoidType)
        {
        }
        else
        {
            dump(context, "let  ");
            dumpID(context, inst);
            dumpInstTypeClause(context, type);
            dump(context, "\t= ");
        }


        dump(context, opInfo->name);

        uint32_t argCount = inst->argCount;
        dump(context, "(");
        for (uint32_t ii = 1; ii < argCount; ++ii)
        {
            if (ii != 1)
                dump(context, ", ");

            auto argVal = inst->getArgs()[ii].usedValue;

            dumpOperand(context, argVal);
        }
        dump(context, ")");

        dump(context, "\n");

        // The instruction might have children,
        // so we need to handle those here
        dumpChildren(context, inst);
    }

    void printSlangIRAssembly(StringBuilder& builder, IRModule* module)
    {
        IRDumpContext context;
        context.builder = &builder;
        context.indent = 0;

        dumpChildrenRaw(&context, module);
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
